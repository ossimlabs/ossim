//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/base/ossimKMeansClustering.h>
#include <iostream>

using namespace std;

ossimKMeansClustering::ossimKMeansClustering()
:  m_numEntries(0),
   m_samples(0),
   m_populations(0),
   m_clustersValid(false),
   m_verbose(false)
{

}

ossimKMeansClustering::~ossimKMeansClustering()
{
   delete m_samples;
   delete m_populations;
   m_clusters.clear();
}

void ossimKMeansClustering::setNumClusters(ossim_uint32 K)
{
   m_clusters.clear();
   m_clusters.resize(K);
   m_clustersValid = false;
}

bool ossimKMeansClustering::computeKmeans()
{
   if ((m_numEntries == 0) || (m_samples == 0))
         return false;

   // If populations aren't provided, assume only 1 of each sample:
   if (m_populations == 0)
   {
      m_populations = new double [m_numEntries];
      for (ossim_uint32 i=0; i<m_numEntries; ++i)
         m_populations[i] = 1.0;
   }

   // Scan for min and max:
   double overall_min = OSSIM_DEFAULT_MAX_PIX_DOUBLE;
   double overall_max = OSSIM_DEFAULT_MIN_PIX_DOUBLE;
   for (ossim_uint32 i=0; i<m_numEntries; i++)
   {
      if (m_populations[i] == 0)
         continue;

      if (m_samples[i] < overall_min)
         overall_min = m_samples[i];
      else if (m_samples[i] > overall_max)
         overall_max = m_samples[i];
   }
   double max_delta = overall_max - overall_min;
   double convergenceThreshold = 0.1*(max_delta)/m_numEntries;
   ossim_uint32 numClusters = m_clusters.size();
   double* variances = new double [numClusters];
   ossim_uint32* priorCounts = new ossim_uint32 [numClusters];
  double interm_samples = (overall_max - overall_min) / numClusters;
   double mean_i = overall_min + interm_samples / 2.0;  // initial mean for cluster 0;
   double bound = overall_min;
   for (ossim_uint32 gid=0; gid<numClusters; ++gid)
   {
      //Initialize cluster with even spread and initial mean:
      m_clusters[gid].min = bound;
      bound += interm_samples;
      m_clusters[gid].max = bound;
      m_clusters[gid].mean = mean_i;
      m_clusters[gid].new_mean = 0;
      mean_i += interm_samples;
      m_clusters[gid].n = 0;
      m_clusters[gid].sigma = 1.0;
      variances[gid] = 0.0;
      priorCounts[gid] = 0;
   }
   double delta, min_delta;
   ossim_uint32 best_gid, np, iters = 0, max_iters = 20;
   bool converged = false;

   // Loop until converged on best solution:
   while (!converged && (iters < max_iters))
   {
      converged = true; // prove otherwise
      ++iters;

      for (ossim_uint32 gid=0; gid<numClusters; ++gid)
      {
         m_clusters[gid].min = overall_max;
         m_clusters[gid].max = overall_min;
      }

      for (ossim_uint32 i=0; i<m_numEntries; i++)
      {
         if ( m_populations[i] == 0)
            continue;

         // Find the current cluster:
         best_gid = 0;
         min_delta = OSSIM_DEFAULT_MAX_PIX_DOUBLE;
         for (ossim_uint32 gid=0; gid<numClusters; ++gid)
         {
            delta = fabs((m_samples[i] - m_clusters[gid].mean));
            if (delta < min_delta)
            {
               min_delta = delta;
               best_gid = gid;
            }
         }

         // Possible update of min/max for current cluster:
         if (m_samples[i] < m_clusters[best_gid].min)
            m_clusters[best_gid].min = m_samples[i];
         else if (m_samples[i] > m_clusters[best_gid].max)
            m_clusters[best_gid].max = m_samples[i];

         // Accumulate sample for the new mean for this cluster:
         m_clusters[best_gid].new_mean += m_populations[i]*m_samples[i];
         m_clusters[best_gid].n += m_populations[i];

         // Add this bin's contribution to the cluster stats. Use the sigma member as accumulator,
         // normalize later:
         delta = m_clusters[best_gid].mean - m_samples[i];
         variances[best_gid] += m_populations[i]*delta*delta;

      } // End loop over bins

      // Finished processing all input pixels for this iteration. Update the means:
      for (ossim_uint32 gid=0; gid<numClusters; ++gid)
      {
         // Compute new mean from accumulation:
         if (m_clusters[gid].n)
         {
            m_clusters[gid].new_mean /= m_clusters[gid].n;
            m_clusters[gid].sigma = sqrt(variances[gid] / m_clusters[gid].n);
            variances[gid] = 0.0;
         }

         //if (fabs(m_clusters[gid].mean - m_clusters[gid].new_mean) > convergenceThreshold )
         if (m_clusters[gid].n != priorCounts[gid])
            converged = false;

         if (m_verbose)
         {
            cout<<"iteration: "<<iters<<endl;
            cout<<"cluster["<<gid<<"].mean     = "<<m_clusters[gid].mean<<endl;
            cout<<"cluster["<<gid<<"].new_mean = "<<m_clusters[gid].new_mean<<endl;
            cout<<"cluster["<<gid<<"].sigma    = "<<m_clusters[gid].sigma<<endl;
            cout<<"cluster["<<gid<<"].n        = "<<m_clusters[gid].n<<"  prior: "<<priorCounts[gid]<<endl;
            cout<<"cluster["<<gid<<"].min      = "<<m_clusters[gid].min<<endl;
            cout<<"cluster["<<gid<<"].max      = "<<m_clusters[gid].max<<endl;
            cout << endl;
         }
         if (m_clusters[gid].n)
            m_clusters[gid].mean = m_clusters[gid].new_mean;

         if (!converged)
         {
            priorCounts[gid] = m_clusters[gid].n;
            m_clusters[gid].n = 0;
            m_clusters[gid].new_mean = 0;
         }
      }
   } // End overall loop for convergence:

   if (m_verbose)
   {
      for (ossim_uint32 gid=0; gid<numClusters; gid++)
      {
         cout<<"cluster["<<gid<<"] n        = "<<m_clusters[gid].n<<endl;
         cout<<"           mean     = "<<m_clusters[gid].mean<<endl;
         cout<<"           sigma    = "<<m_clusters[gid].sigma<<endl;
         cout<<"           min      = "<<m_clusters[gid].min<<endl;
         cout<<"           max      = "<<m_clusters[gid].max<<endl;
         cout << endl;
      }
   }

   delete variances;
   delete priorCounts;
   m_clustersValid = true;
   return true;

}

double ossimKMeansClustering::getMean(ossim_uint32 clusterId) const
{
   if (clusterId >= m_clusters.size())
      return ossim::nan();

   return m_clusters[clusterId].mean;
}

double ossimKMeansClustering::getSigma(ossim_uint32 clusterId) const
{
   if (clusterId >= m_clusters.size())
      return ossim::nan();

   return m_clusters[clusterId].sigma;
}

double ossimKMeansClustering::getMinValue(ossim_uint32 clusterId) const
{
   if (clusterId >= m_clusters.size())
      return ossim::nan();

   return m_clusters[clusterId].min;
}

double ossimKMeansClustering::getMaxValue(ossim_uint32 clusterId) const
{
   if (clusterId >= m_clusters.size())
      return ossim::nan();

   return m_clusters[clusterId].max;
}

const ossimKMeansClustering::Cluster*
ossimKMeansClustering::getCluster(ossim_uint32 i) const
{
   if (i >= m_clusters.size())
      return 0;
   return &(m_clusters[i]);
}


