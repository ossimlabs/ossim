//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef ossimKmeansClassifier_HEADER
#define ossimKmeansClassifier_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimReferenced.h>
#include <vector>

/***************************************************************************************************
 *
 * This class provides a sample set clustering capability using the traditional K-means scheme for
 * partitioning datasets into K distinct groups according to sample value.
 *
 * The samples can be provided as a simple array of samples, or as a histogram when the
 * populations of the corresponding samples are provided as well. The latter scheme is used by the
 * ossimKMeansFilter for clustering pixel values given the image histogram.
 *
 **************************************************************************************************/
class OSSIM_DLL ossimKMeansClustering : public ossimReferenced
{
public:
   class Cluster
   {
   public:
      Cluster() :
         min(0), max(0), mean(0), sigma(0), new_mean(0), n(0) {}

      double min;      // samples in group are => this, used for detecting convergence
      double max;      // samples in group are < this, used for detecting convergence
      double mean;
      double sigma;
      double new_mean;
      double n;        // number of samples collected for running mean
   };

   ossimKMeansClustering();
   ~ossimKMeansClustering();

   void setNumClusters(ossim_uint32 K);
   template<class T> void setSamples(T* samples, ossim_uint32 num_entries);
   template<class T> void setPopulations(T* populations, ossim_uint32 num_entries);
   bool computeKmeans();

   ossim_uint32 getNumClusters() const { return m_clusters.size(); }
   double getMean(ossim_uint32 groupId) const;
   double getSigma(ossim_uint32 groupId) const;
   double getMinValue(ossim_uint32 groupId) const;
   double getMaxValue(ossim_uint32 groupId) const;

   const ossimKMeansClustering::Cluster* getCluster(ossim_uint32 i) const;

   void setVerbose(bool v=true) const  { m_verbose = v; }

private:
   ossim_uint32 m_numEntries;
   double* m_samples;
   double* m_populations; // use double to handle arbitrarily large datasets
   std::vector<Cluster> m_clusters;
   bool m_clustersValid;
   mutable bool m_verbose;
};

template<class T> void ossimKMeansClustering::setSamples(T* samples, ossim_uint32 num_entries)
{
   if ((num_entries == 0) || (samples == 0))
      return;

   m_clustersValid = false;
   m_numEntries = num_entries;
   m_samples = new double[num_entries];
   for (ossim_uint32 i=0; i<num_entries; i++)
      m_samples[i] = (double) samples[i];
}

template<class T> void ossimKMeansClustering::setPopulations(T* populations,
                                                             ossim_uint32 num_entries)
{
   if ((num_entries == 0) || (populations == 0))
      return;

   m_clustersValid = false;
   m_populations = new double[num_entries];
   for (ossim_uint32 i=0; i<num_entries; i++)
      m_populations[i] = (double) populations[i];
}



#endif /* ossimKMeansClassifier_HEADER */
