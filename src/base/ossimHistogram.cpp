//*******************************************************************
//
// License: MIT
// 
// Author: Ken Melero (kmelero@imagelinks.com)
//         Orginally developed by:
//                   Copyright (c) 1997 TargetJr Consortium
//               GE Corporate Research and Development (GE CRD)
//                             1 Research Circle
//                            Niskayuna, NY 12309
//         Adapted from:  IUE v4.1.2
// Description: 
//      A ossimHistogram contains an array of "buckets", which represent finite
// segments of some value axis, along with a corresponding array of
// frequency m_counts for each of these buckets.
//
//********************************************************************
// $Id$

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimHistogram.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimThinPlateSpline.h>
#include <ossim/base/ossimDpt.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;



// nonstandard versions that use operator>, so they behave differently
// than std:::min/max and ossim::min/max.  kept here for now for that
// reason.
#ifndef MAX
#  define MAX(x,y) ((x)>(y)?(x):(y))
#  define MIN(x,y) ((x)>(y)?(y):(x))
#endif


static const int MEAN_FLAG = 1, SD_FLAG = 2;
RTTI_DEF1(ossimHistogram, "ossimHistogram", ossimObject);
ossimHistogram::ossimHistogram()
   :
   m_statsConsistent(MEAN_FLAG | SD_FLAG),
   m_vals(new float [1]),
   m_counts(new float [1]),
   m_num(0),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0)
{
   m_vals[0] = 0.0;
   m_counts[0] = 0.0;
}

ossimHistogram::ossimHistogram(int xres, float val1, float val2)
   :
   m_statsConsistent(MEAN_FLAG | SD_FLAG),
   m_vals(new float [xres]),
   m_counts(new float [xres]),
   m_num(xres),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0)
{
   m_vmax = MAX(val1, val2);
   m_vmin = MIN(val1, val2);

   //---
   // Set the delta which is used to index the bins.
   // Note: that using "(m_vmax - m_vmin) / xres" was dropping the
   // last bin on integer data.
   //---
   if ( (m_vmax - m_vmin + 1) == xres )
   {
      m_delta = 1.0;
   }
   else
   {
      m_delta = (m_vmax - m_vmin) / xres;
   }

   m_mean = (float)((m_vmax + m_vmin)/2.0);
   m_standardDev = (float)((m_vmax - m_vmin)/(2.0*sqrt(3.0)));
   int i = 0;

   if (m_vals == NULL || m_counts == NULL)
   {
      fprintf(stderr, "Histogram : Ran out of memory for arrays.\n");
      m_vals = NULL;
      m_counts = NULL;
      m_num = 0;
      m_vmin = 0;
      m_vmax = 0;
      m_delta = 0.0;
   }
   else
   {
      //std::cout << std::setprecision(15) << m_vmin << ", " << m_vmax <<  ", " <<m_delta <<", "<< xres << std::endl; 
      for(i = 0; i < xres; i++)
      {
         m_vals[i] = m_vmin + m_delta * (float)(i + 0.5);
         //std::cout << m_vals[i] << std::endl;
         m_counts[i] = 0.0;
      }
   }
}

ossimHistogram::ossimHistogram(float* uvals, float* ucounts, int xres)
   :
   m_statsConsistent(MEAN_FLAG | SD_FLAG),
   m_vals(uvals),
   m_counts(ucounts),
   m_num(xres),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0)
{
   if ( ( xres >= 2 ) && uvals && ucounts )
   {
      m_delta = m_vals[1] - m_vals[0]; // Changed this from delta = 1.0
      //  m_vmax = GetMaxVal();
      //  m_vmin = GetMinVal(); JAF version
      m_vmin = uvals[0] - .5f*m_delta;
      m_vmax = uvals[m_num-1] + .5f*m_delta;
      m_mean = GetMean();
      m_standardDev = GetStandardDev();
   }   
}
ossimHistogram::ossimHistogram(const double* data, ossim_uint32 size, ossim_uint32 xres)
   :
   m_statsConsistent(0),
   m_vals(0),
   m_counts(0),
   m_num((int)xres),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0)
{
   if ((size == 0) || (xres == 0))
      return;

   // scan the dataset for min/max:
   m_vmin=(float)(data[0]);
   m_vmax=(float)(data[0]);
   for (ossim_uint32 i=1; i<size; ++i)
   {
      if ((float)(data[i]) < m_vmin)
         m_vmin = (float)(data[i]);
      else if ((float)(data[i]) > m_vmax)
         m_vmax = (float)(data[i]);
   }

   // Allocate histogram:
   m_delta = (m_vmax - m_vmin) / m_num;
   m_vals = new float [m_num];
   m_counts = new float [m_num];
   for (ossim_int32 i=0; i<m_num; ++i)
   {
      m_vals[i] = m_vmin + m_delta * (i + 0.5);
      m_counts[i] = 0.0;
   }

   // compute histogram:
   for (ossim_uint32 i=0; i<size; i++)
      UpCount((float)(data[i]));

   GetMean();
   GetStandardDev();
}

//-----------------------------------------------------------
// -- Copy constructor
ossimHistogram::ossimHistogram(const ossimHistogram& his)
:
m_statsConsistent(0),
m_vals(0),
m_counts(0),
m_num(0),
m_delta(0.0),
m_vmin(0),
m_vmax(0),
m_mean(0.0),
m_standardDev(0.0)
{

   int i = 0;
   m_num = his.GetRes();

   m_vals = new float[m_num];
   const float* his_vals = his.GetVals();

   m_counts = new float[m_num];
   const float* his_counts = his.GetCounts();

   if (m_vals == NULL || m_counts == NULL)
   {
      fprintf(stderr, "Histogram : Ran out of memory for arrays.\n");
      m_vals = NULL;
      m_counts = NULL;
      m_num = 0;
      m_vmin = 0;
      m_vmax = 0;
      m_delta = 0.0;
      m_statsConsistent = 0;
      return;
   }

   m_mean = his.GetMean();
   m_standardDev = his.GetStandardDev();

   for(i=0; i<m_num; i++)
   {
      m_vals[i] = his_vals[i];
      m_counts[i] = his_counts[i];
   }
   m_vmax = his.GetMaxVal();
   m_vmin = his.GetMinVal();
   m_delta = his.GetBucketSize();

   m_statsConsistent = 0;
   m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
}


//---------------------------------------
// -- Resample a histogram

ossimHistogram::ossimHistogram(const ossimHistogram* his, float width)
:
m_statsConsistent(0),
m_vals(0),
m_counts(0),
m_num(0),
m_delta(0.0),
m_vmin(0),
m_vmax(0),
m_mean(0.0),
m_standardDev(0.0)
{

   m_statsConsistent =0;

// Attributes of original histogram

   float del = his->GetBucketSize();
   int max_index = his->GetRes() - 1;
   float minvalue = his->GetVals()[0] - del*.5f;
   float maxvalue = his->GetVals()[max_index] + del*.5f;


// Intialize a new histogram
   if(width == del) m_num = his->GetRes();
   else if(!(width == 0.0))
      m_num = (int)ceil((maxvalue - minvalue)/width);
   else
      m_num = 1; // This shouldn't happen anyway.

   m_vals = new float [m_num];
   m_counts = new float [m_num];
   m_delta = width;
   float mean_val = (maxvalue + minvalue)/2.0f;
   float half_range = (m_num * m_delta)/2.0f;
   m_vmax =  mean_val + half_range;
   m_vmin =  mean_val - half_range;
   int i = 0;

   if (m_vals == NULL || m_counts == NULL)
   {
      fprintf(stderr,
              "Histogram : Ran out of memory for arrays.\n");
      m_vals = NULL;
      m_counts = NULL;
      m_num = 0;
      m_vmin = 0;
      m_vmax = 0;
      m_delta = 0.0;
      m_mean = 0.0;
      m_standardDev = 0.0;
      m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
      return;
       
   }
   
   else
   {
      for(i = 0; i < m_num; i++)
      {
         m_vals[i] = m_vmin + m_delta * (i + 0.5f);
         m_counts[i] = 0.0;
      }
   }


// Cases:


   if(width == del)    // Then just copy his
   {
      const float* his_counts = his->GetCounts();
      for(i=0; i<m_num; i++)
         m_counts[i] = his_counts[i];
      m_mean = GetMean();
      m_standardDev = GetStandardDev();
      m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
      return;
   }


   if(del > width)     // Then interpolate his m_counts.
   {

// Boundary conditions:
//    Start
      float his_start = minvalue + .5f*del;
      float start = m_vmin + .5f*m_delta;
      float c0 = his->GetCount(his_start);
      float c1 = his->GetCount(his_start + del);
      float s0 = (c1 - c0)/del;

      for(float x = start; x <= (his_start + del + m_delta);)
      {
         float interp = s0 * (x - his_start) + c0;
         if(interp < 0) interp = 0; //Can be negative
         SetCount(x,interp);
         x += width;
      }
//    End
      float his_end = maxvalue - .5f*del;
      float end = m_vmax - .5f*m_delta;
      float cn = his->GetCount(his_end);
      float cn_1 = his->GetCount(his_end - del);
      float sn = (cn_1 - cn)/del;

      for(float y = end; y >= (his_end - del + m_delta);)
      {
         float interp = sn * (his_end - y) + cn;
         if(interp < 0) interp = 0; //Can be negative
         SetCount(y, interp);
         y -= m_delta;
      }
// Interior Loop

      for(float z = his_start + del; z <= (his_end - del);)
      {
         float ci = his->GetCount(z);
         float ci_1 = his->GetCount(z-del);
         float cip1 = his->GetCount(z+del);
         float deriv = (cip1 - ci_1)/(2.0f*del);
         float second_drv =
            ((cip1 + ci_1)/2.0f - ci)/(del*del);
         int fine_x_index = GetIndex(z);
         if (fine_x_index < 0)
         {
            if (z<m_vmin) fine_x_index = 0;
            else fine_x_index = m_num-1;
         }
         float fine_x = m_vals[fine_x_index];
         for(float xfine = fine_x; xfine < z + del;)
         {
            float interp = ci + deriv*(xfine -z) +
               second_drv*(xfine - z)*(xfine - z);

            if(interp < 0) interp = 0; //Can be negative
            SetCount(xfine, interp);
            xfine += width;
         }
         z += del;
      }
   }


   if(del < width)    //Just accumulate samples from his into larger bins
   {
      if( del != 0.0){
         float his_start = minvalue + .5f*del;
         float his_end = maxvalue - .5f*del;
         for(float x = his_start; x <= his_end;)
         {
            SetCount(x, (GetCount(x) + his->GetCount(x)));
            x += del;
         }
      }
   }
   m_mean = GetMean();
   m_standardDev = GetStandardDev();
   m_statsConsistent =0;
   m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
}

void ossimHistogram::create(int xres, float val1, float val2)
{
   // clear all the data
   deleteAll();

   // now set it up and initialize;
   xres = xres >0? xres:1;
   
   m_vals   = new float [xres];
   m_counts = new float [xres];
   m_num = xres;
   m_vmax = MAX(val1, val2);
   m_vmin = MIN(val1, val2);

   m_delta = (m_vmax - m_vmin) / xres;
   m_mean = (float)((m_vmax + m_vmin)/2.0);
   m_standardDev = (float)((m_vmax - m_vmin)/(2.0*sqrt(3.0)));
   m_statsConsistent = 0;
   m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
   int i = 0;
   if (m_vals == NULL || m_counts == NULL)
   {
      ossimNotify(ossimNotifyLevel_FATAL) << "Histogram : Ran out of memory for arrays.\n";
      m_vals = NULL;
      m_counts = NULL;
      m_num = 0;
      m_vmin = 0;
      m_vmax = 0;
      m_delta = 0.0;
   }
   else
   {
      for(i = 0; i < xres; i++)
      {
         m_vals[i] = m_vmin + m_delta * (float)(i + 0.5);
         m_counts[i] = 0.0;
      }
   }   
}
ossimHistogram* ossimHistogram::fillInteriorEmptyBins(int type)const
{
   if(m_num < 1) return 0;
   ossimHistogram* result = new ossimHistogram(*this);
   switch(type)
   {
      case HISTOGRAM_FILL_THIN_PLATE:
      case HISTOGRAM_FILL_DEFAULT:
      {
         ossimThinPlateSpline spline(1);
         double pvars[1];
         float* new_counts = result->GetCounts();
         ossim_int32 idxLeft = 0;
         ossim_int32 idxRight = m_num-1;
         while((idxLeft < m_num) && (new_counts[idxLeft]  < 1))++idxLeft;
         while((idxRight > -1) && (new_counts[idxRight] < 1))--idxRight;
         if(idxLeft < idxRight)
         {
            ossim_int32 idx = idxLeft;
            while(idx <= idxRight)
            {
               if(new_counts[idx]>0)
               {
                  pvars[0] = new_counts[idx];
                  spline.addPoint(idx, 0, pvars);
               }
               ++idx;
            }
            if(spline.solve())
            {
               idx = idxLeft;
               while(idx <= idxRight)
               {
                  if(spline.getPoint(idx, 0, pvars))
                  {
                     new_counts[idx] = pvars[0];
                  }
                  ++idx;
               }
               m_statsConsistent = 0;
            }
            else
            {
            }
         }
         
         break;
      }
   }
   
   return result;
}
//--------------------------------------------------
// -- Transform the value axis of a histogram by a
//    translation, transl, and a scale factor, scale.
//    The new histogram has the same resolution as his.

ossimHistogram* ossimHistogram::Scale(float scale_factor)
{

// Extract attributes of self

//    float lowvalue = m_vals[0];
   float highvalue = m_vals[m_num-1];

// Construct a new histogram

   ossimHistogram* scaled_his = new ossimHistogram(this, m_delta);
   float* new_counts = scaled_his->GetCounts();
   int i = 0;
   for(i=0; i < m_num; i++)  // Initialize
      new_counts[i] = 0.0;

// Compute scaled values
// We assume that the new histogram is to be scaled down from his

   float scale = scale_factor;
   if(scale_factor > 1.0) scale = 1.0;

   for(float x = highvalue; x > m_vmin;)
   {
      float trans_x = (x-m_vmin)*scale + m_vmin; // Scaled x.
      int index = GetIndex(trans_x);
      if (index < 0)
      {
         if (trans_x<m_vmin) index = 0;
         else index = m_num-1;
      }
      float fraction = (trans_x - m_vals[index])/m_delta;
      float abs_fraction = (float)fabs(fraction);
      int x_index = GetIndex(x);
      if (x_index < 0)
      {
         if (x<m_vmin) x_index = 0;
         else x_index = m_num-1;
      }

// Distribute the m_counts in proportion

      new_counts[index] += (1.0f - abs_fraction)*m_counts[x_index];
      if(fraction > 0)
         if(index < (m_num-1))
            new_counts[index + 1] +=
               abs_fraction*m_counts[x_index];
         else
            new_counts[index] +=
               abs_fraction*m_counts[x_index];
      else
         if(index > 0)
            new_counts[index - 1] +=
               abs_fraction*m_counts[x_index];
         else
            new_counts[index] +=
               abs_fraction*m_counts[x_index];
      x -= m_delta;
   }

// Compute new Histogram attributes

   m_mean = scaled_his->GetMean();
   m_standardDev = scaled_his->GetStandardDev();
   return scaled_his;
}

//---------------------------------------------------------------------
// -- Assuming that "this" is a histogram of population density,
//    construct a new histogram which is the cumulative distribution.
//    Each bin, xi, in his is assumed to represent a density, i.e.,
//            {x | (xi - .5*m_delta) < x <= (xi + .5*m_delta)}
//    Each bin, xi, in the result represents a cumulative distribution, i.e.,
//            {x | x <= (xi + .5*m_delta)}
ossimHistogram* ossimHistogram::CumulativeGreaterThanEqual()const
{
   ossimHistogram* cum_his = new ossimHistogram(*this);
   const float* density_counts = this->GetCounts();
   int res = this->GetRes();

   // Intitialize cumulative m_counts
   float* cum_counts = cum_his->GetCounts();
   int i = 0;
   for(i=0; i < res; i++)
      cum_counts[i] = 0;
  
   cum_counts[res-1] = density_counts[res-1];
   for(i = res-2; i>=0; --i)
   {
      cum_counts[i] += (density_counts[i] + cum_counts[i+1]);
   }

   return cum_his;
}

ossimHistogram* ossimHistogram::CumulativeLessThanEqual()const
{
   ossimHistogram* cum_his = new ossimHistogram(*this);
   const float* density_counts = this->GetCounts();
   int res = this->GetRes();

   // Intitialize cumulative m_counts
   float* cum_counts = cum_his->GetCounts();
   int i = 0;
   for(i=0; i < res; i++)
      cum_counts[i] = 0;

   cum_counts[0] = density_counts[0];
   for(i = 1; i < res; i++)
   {
      cum_counts[i] += (density_counts[i] + cum_counts[i-1]);
   }
  
   return cum_his;
}

//Provides the correct values for histogram m_counts when the bin index 
//extends outside the valid range of the m_counts array.  This function
//permits easy array access logic for the NonMaximumSuppression algorithm.
//The cyclic flag indicates that the m_counts array index is circular, i.e,
//cnts[0] equivalent to cnts[n_bins-1]
inline float GetExtendedCount(int bin, int n_bins, float* cnts, bool cyclic)
{
   int nbm = n_bins-1;
   if(!cyclic)
   {
      if(bin < 0)
         return cnts[0];
      if(bin >= n_bins)
         return cnts[nbm];
   }
   else
   {
      if(bin<0)
         return cnts[nbm+bin];
      if(bin >= n_bins)
         return cnts[bin-n_bins];
   }
   return cnts[bin];
}
//Prune any sequences of more than one maxium value
//That is, it is possible to have a "flat" top peak with an arbitarily
//long sequence of equal, but maximum values. The cyclic flag indictates
//that the sequence wraps around, i.e. cnts[0] equivalent to cnts[nbins-1]
inline void RemoveFlatPeaks(int nbins, float* cnts, bool cyclic)
{
   int nbm = nbins-1;

   //Here we define a small state machine - parsing for runs of peaks
   //init is the state corresponding to an initial run (starting at i ==0)
   bool init=(GetExtendedCount(0, nbins, cnts, cyclic) !=0 ) ? true : false;
   int init_end =0;

   //start is the state corresponding to any other run of peaks
   bool start=false;  
   int start_index=0; 
   int i = 0;

   //The scan of the state machine
   for(i = 0; i < nbins; i++)
   {
      float v = GetExtendedCount(i, nbins, cnts, cyclic);

      //State init: a string of non-zeroes at the begining.
      if(init&&v!=0)
         continue;

      if(init&&v==0)
      {
         init_end = i;
         init = false;
         continue;
      }

      //State !init&&!start: a string of "0s"
      if(!start&&v==0)
         continue;

      //State !init&&start: the first non-zero value
      if(!start&&v!=0)
      {
         start_index = i;
         start = true;
         continue;
      }
      //State ending flat peak: encountered a subsequent zero after starting
      if(start&&v==0)
      {
         int peak_location = (start_index+i-1)/2;//The middle of the run
         int k = 0;
         for(k = start_index; k<=(i-1); k++)
	    if(k!=peak_location)
               cnts[k] = 0;
         start = false;
      }
   }
   //Now handle the boundary conditions
   //The non-cyclic case
   if(!cyclic)
   {
      if(init_end!=0)  //Was there an initial run of peaks?
      {
         int init_location = (init_end-1)/2;
         int k = 0;
         for(k = 0; k<init_end; k++)
	    if(k!=init_location)
               cnts[k] = 0;
      }
      if(start)       // Did we reach the end of the array in a run of pks?
      {
         int end_location = (start_index + nbm)/2;
         int k = 0;
         for(k = start_index; k<nbins; k++)
	    if(k!=end_location)
               cnts[k] = 0;
      }
   }
   else  //The cyclic case
   {
      if(init_end!=0)  //Is there a run which crosses the cyclic cut?
      {
         if(start)    
         { //Yes, so define the peak location accordingly
	    int peak_location = (start_index + init_end - nbm -1)/2;
	    int k;
	    if(peak_location < 0) //Is the peak to the left of the cut?
            {// Yes, to the left
               peak_location += nbm; 
               for( k = 0; k< init_end; k++)
		  cnts[k]=0;
               for( k= start_index; k <nbins; k++)
		  if(k!=peak_location)
                     cnts[k] = 0;
            }
	    else   
            {//No, on the right.
               for( k = start_index; k< nbins; k++)
		  cnts[k]=0;
               for( k= 0; k < init_end; k++)
		  if(k!=peak_location)
                     cnts[k] = 0;
            }
         }
         else  
         {//There wasn't a final run so just clean up the initial run
	    int init_location = (init_end-1)/2;
	    int k = 0;
	    for(k = start_index; k<init_end; k++)
               if(k!=init_location)
                  cnts[k] = 0;
         }
      }
   }
}

//----------------------------------------------------------
// -- Suppress values in the Histogram which are not locally 
//    a maxium. The neighborhood for computing the local maximum
//    is [radius X radius], e.g. for radius =1 the neighborhood
//    is [-X-], for radius = 2, the neighborhood is [--X--], etc.
//    If the cyclic flag is true then the index space is assumed to
//    be equivalent to a circle. That is, elements "0" and (n_buckets-1)
//    are in correspondence.
ossimHistogram* ossimHistogram::NonMaximumSupress(int radius, bool cyclic)
{
   if((2*radius +1)> m_num/2)
   {
      ossimNotify(ossimNotifyLevel_WARN)<<"ossimHistogram::NonMaximumSupress the radius is too large \n";
      return NULL;
   }
   //Get the m_counts array of "this"
   ossimHistogram* h_new = new ossimHistogram(*this);
   int n_buckets = h_new->GetRes();
   float* counts_old = this->GetCounts();

   //Make a new Histogram for the suppressed version
   float* counts_new = h_new->GetCounts();
   int i;
   for( i =0; i < n_buckets; i++)
      counts_new[i] = 0;
  
   //Find local maxima
   for( i = 0; i<  n_buckets; i++)
   {
      //find the maxium value in the current kernel
      float max_count = counts_old[i];
      int k = 0;
      for(k = -radius; k <= radius ;k++)
      {
         int index = i+k;
         float c = GetExtendedCount(index, n_buckets, counts_old, cyclic);
         if( c > max_count)
	    max_count = c;
      }
      //Is position i a local maxium?
      if(max_count == counts_old[i])
         counts_new[i] = max_count;//Yes. So set the m_counts to the max value
   }
   RemoveFlatPeaks(n_buckets, counts_new, cyclic);
   return h_new;
}
//----------------------------------------------------------
// -- Compute the m_mean of the histogram population
float ossimHistogram::GetMean()const
{
   float xsum = 0.0;

   if(MEAN_FLAG&m_statsConsistent)
      return m_mean;
   else
   {
      if( this->GetBucketSize() > 0.0){
         for(float x=this->GetMinVal(); x<= this->GetMaxVal(); x +=this->GetBucketSize())
            xsum += x*GetCount(x);
      }

      float area = ComputeArea(m_vmin, m_vmax);
      if(area <= 0.0)
      {
         //	      fprintf(stderr, "Histogram : Area <= 0.0\n");
         return 0.0;
      }
      else
      {
         m_statsConsistent |=1;
         m_mean = xsum/area;
         return m_mean;
      }
   }
}



float ossimHistogram::GetStandardDev()const
{
   float sum = 0.0;

   if(SD_FLAG&m_statsConsistent)
      return m_standardDev;
   else
   {
      float xm = this -> GetMean(); // Force an Update of m_mean

      if( this->GetBucketSize() > 0.0){
         for(float x=this->GetMinVal();
             x<= this->GetMaxVal();
             x +=this->GetBucketSize())

            sum += (x-xm)*(x-xm)*GetCount(x);
      }

      float area = ComputeArea(m_vmin, m_vmax);
      if(area <= 0.0)
      {
         //	      fprintf(stderr, "Histogram : Area <= 0.0\n");
         return 0.0;
      }
      else
      {
         m_statsConsistent |= 2;
         m_standardDev = (float)sqrt(sum/area);
         return m_standardDev;
      }
   }
}

int ossimHistogram::GetIndex(float pixelval)const
{
#if 1
   if ((pixelval > m_vmax) || (pixelval < m_vmin) || (m_num==0) )
   {
      return -1;
   }
//   ossim_float32 d = m_vmax-m_vmin;
   int bandIdx = (ossim_int32)((pixelval-m_vmin)/m_delta);
   return bandIdx<GetRes()?bandIdx:-1;
//    if(bandIdx == m_num)
//    {
//       return m_num-1;
//    }
//    else if(bandIdx < m_num)
//    {
//       return bandIdx;
//    }
//    return -1;
#else
   if ((pixelval > m_vmax) || (pixelval < m_vmin))
      return -1;

   int idx = 0;
   int i = 0;

   for(i = 0; i < m_num; i++)
   {
      //std::cout << std::setprecision(15) << m_vals[i] << std::endl;
      // RWMC: This is very dangerous - might get an intermediate
      // value which is between m_vals[i]+0.5*m_delta and
      // m_vals[i+1]-0.5*m_delta, which would then return index of 0.
      // Changed to check range one-sided, which is safe because of
      // previous check on range.
      //       if ((pixelval > (m_vals[i] - 0.5 * m_delta)) &&
      //           (pixelval <= (m_vals[i] + 0.5 * m_delta)))
      if (pixelval <= (m_vals[i] + 0.5 * m_delta))
      {
         idx = i;
         break;
      }
   }
//std::cout << idx << std::endl;
   return idx;
#endif
}
float ossimHistogram::GetMinValFromIndex(ossim_uint32 idx)const
{
   float result = 0.0;
   
   if((int)idx < m_num)
   {
      result = (m_vals[idx]-(0.5 * m_delta));
      if(result < m_vmin) result = m_vmin;
   }

   return result;
}

float ossimHistogram::GetMaxValFromIndex(ossim_uint32 idx)const
{
   float result = 0.0;
   
   if((int)idx < m_num)
   {
      result = (m_vals[idx]+(0.5 * m_delta));
      if(result > m_vmax) result = m_vmax;
   }

   return result;  
}

float ossimHistogram::GetValFromIndex(ossim_uint32 idx)const
{
   float result = 0.0;
   if((int)idx < m_num)
   {
      result = m_vals[idx];
   }

   return result;
}

int ossimHistogram::GetValIndex(float pixelval)const
{
   if ((pixelval > m_vmax) || (pixelval < m_vmin))
      return -1;

   int idx = 0;
   int i = 0;

   for(i = 0; i < m_num; i++)
   {
      if ((pixelval > (m_vals[i] - 0.5 * m_delta)) &&
          (pixelval <= (m_vals[i] + 0.5 * m_delta)))
      {
         idx = i;
         break;
      }
   }

   return idx;
}



float ossimHistogram::GetCount(float pixelval)const
{
   int index = GetIndex(pixelval);

   if (index < 0)
      return -1;
   else
      return m_counts[index];
}



float ossimHistogram::GetMinVal()const
{
   int i=0;

   while (i<m_num-1 && !m_counts[i])
      i++;

   return m_vals[i];
}




float ossimHistogram::GetMaxVal()const
{
   int i=m_num-1;

   while (i>0 && !m_counts[i])
      i--;

   if (i < 0)
      return 0.0;

   return m_vals[i];
}


float ossimHistogram::GetMaxCount()const
{
   int i=0;
   float max;
   max = 0.0;
   for (i=0; i < m_num; i++)
      if (m_counts[i] > max)
         max = m_counts[i];
   return max;
}




float ossimHistogram::SetCount(float pixelval, float count)
{
   m_statsConsistent = 0;

   int index = GetIndex(pixelval);

   if (index < 0)
      return -1;
   else
   {
      m_counts[index] = count;
      return count;
   }
}


void ossimHistogram::UpCount(float pixelval, float occurences)
{

   m_statsConsistent = 0;
   int idx = GetIndex(pixelval);
   if (idx >= 0)  // Originally (index > 0)
   {
      m_counts[idx] += occurences;
   }
}

float ossimHistogram::ComputeArea(float low, float high)const
{
   float sum = 0.0;
   float maxval = GetMaxVal();
   float minval = GetMinVal();

   if (low < minval) low = minval;
   if (high > maxval) high = maxval;

   if (low <= high)
   {
      int indexlow, indexhigh;
      indexlow = (int) GetIndex(low);
      if (indexlow < 0)
      {
         if (low<m_vmin) indexlow = 0;
         else indexlow = m_num-1;
      }
      indexhigh = (int) GetIndex(high);
      if (indexhigh < 0)
      {
         if (high<m_vmin) indexhigh = 0;
         else indexhigh = m_num-1;
      }
      int i=indexlow;

      while (i<=indexhigh)
      {
         sum+= m_counts[i];
         i++;
      }
   }

   return sum;
}
//----------------------------------------------------------------------
// --Compute the total area under the histogram
//
float ossimHistogram::ComputeArea()const
{
   float m_vmin = this->GetMinVal();
   float m_vmax = this->GetMaxVal();
   if(m_vmin>m_vmax)
   {
      float temp = m_vmin;
      m_vmin = m_vmax;
      m_vmax = temp;
   }
   return this->ComputeArea(m_vmin, m_vmax);
}

float ossimHistogram::getLowFractionFromValue(float val) const
{
   // std::cout << "ossimHistogram::getLowFractionFromValue(float val)\n";
//   float minValue = floor(GetMinVal());
//   float maxValue = ceil(GetMaxVal());
   float minValue = GetMinVal();
   float maxValue = GetMaxVal();
   if (val < minValue || val > maxValue)
   {
      return ossim::nan();
   }
// std::cout << "VAL: " << val << "\n"
//           << "MIN: " << minValue << "\n"
//           << "MAX: " << maxValue << "\n"; 
   int total_buckets = GetRes();
   int cutoff_bucket = GetValIndex(val);
   float partial_sum = 0.0;
   float total_sum   = 0.0;
   // std::cout << "CUTOFF BUCKET ===" << cutoff_bucket << "\n";
   for(int i = 0; i < total_buckets; ++i)
   {
      total_sum += m_counts[i];
      if (i <= cutoff_bucket)
      {
         partial_sum += m_counts[i];
      }
   }
   // std::cout << "FRACTION ==== " << (partial_sum/total_sum) << "\n";
   return (partial_sum/total_sum);
}

float ossimHistogram::getHighFractionFromValue(float val) const
{
//   float min = floor(GetMinVal());
//   float max = ceil(GetMaxVal());
   float minValue = GetMinVal();
   float maxValue = GetMaxVal();
   if (val < minValue || val > maxValue)
   {
      return ossim::nan();
   }

   int total_buckets = GetRes();
   int cutoff_bucket = GetValIndex(val);
   float partial_sum = 0.0;
   float total_sum   = 0.0;
   
   for(int i = (total_buckets-1); i >= 0; --i)
   {
      total_sum += m_counts[i];
      if (i >= cutoff_bucket)
      {
         partial_sum += m_counts[i];
      }
   }

   return (partial_sum/total_sum);
}

//----------------------------------------------------------------------
//  -- Finds the lower bound value which elminates a given fraction of
//     histogram area.
//
float ossimHistogram::LowClipVal(float clip_fraction)const
{
   if(clip_fraction<0) clip_fraction=0.0;
   if(clip_fraction>1.0) clip_fraction=1.0;
   float area = this->ComputeArea();
   if(area==0.0) return this->GetMinVal();
   if(clip_fraction==0.0) return this->GetMinVal();
   if(clip_fraction==1.0) return this->GetMaxVal();
   float clip_area = area*clip_fraction;
   const float* m_counts = this->GetCounts();
   const float* m_vals = this->GetVals();
   int res = this->GetRes();
   float sum = 0;
   int i=0;

   for(; i<res; i++)
   {
      sum+=m_counts[i];
      if(sum>=clip_area)
         break;
   }

   return m_vals[i];
}

//----------------------------------------------------------------------
//  -- Finds the lower bound value which elminates a given fraction of
//     histogram area.
//
float ossimHistogram::HighClipVal(float clip_fraction)const
{
   if(clip_fraction<0) clip_fraction=0.0;
   if(clip_fraction>1.0) clip_fraction=1.0;
   float area = this->ComputeArea();
   if(area==0.0) return this->GetMaxVal();
   if(clip_fraction==0.0) return this->GetMaxVal();
   if(clip_fraction==1.0) return this->GetMinVal();
   float clip_area = area*clip_fraction;
   const float* m_counts = this->GetCounts();
   const float* m_vals = this->GetVals();
   int res = this->GetRes();
   float sum = 0;
   int i = (res-1);
   for(; i>=0; i--)
   {
      sum+=m_counts[i];
      if(sum>=clip_area)
         break;
   }
   return m_vals[i];
}

//--------------------------------------------------------------------------
// -- Prints histogram m_counts onto cout
void ossimHistogram::Print()const
{
   ostream& out = ossimNotify(ossimNotifyLevel_INFO);
   const float* m_vals = this->GetVals();
   const float* m_counts = this->GetCounts();
   int res = this->GetRes();
   int width = 0;
   int i = 0;
   for(i =0; i < res; i++)
   {
      if(width++ > 5)
      {
         width = 0;
         out << "\n";
      }
      out << m_vals[i] << " " << m_counts[i] << " | " ;
   }
   out << "\n MaxVal " << this->GetMaxVal() << "\n";
   out << " MinVal " << this->GetMinVal() << "\n";
   out << " BucketSize " << this->GetBucketSize() << "\n";
   out << " Resolution " << this->GetRes() << "\n";
   out << " Area "
       << this->ComputeArea(this->GetMinVal(),this->GetMaxVal()) << "\n";
   out << "------------------------------------------------\n\n";
}

//---------------------------------------------------------------------------
// --- dumps histogram  values  to file.

void ossimHistogram::Dump(char *dumpfile)const
{
   FILE *dumpfp = fopen(dumpfile, "w");

   if (!dumpfp)
   {
      fprintf(stderr, "Error opening histogram data file.\n");
      return;
   }
   int i = 0;

   for(i = 0; i < m_num; i++)
      fprintf(dumpfp, "%f %f\n", m_vals[i], m_counts[i]);
  
   fclose(dumpfp);
   return;
}

//---------------------------------------------------------------------------
// -- Writes histogram in format suitable for plotting tools like Gnuplot.

int ossimHistogram::WritePlot(const char *fname)const
{
   FILE *fp = fopen(fname, "w");

   if (!fp)
   {
      fprintf(stderr, "Error opening histogram plot file.\n");
      return 0;
   }

   for(int j = 0; j < m_num; j++)
      fprintf(fp, "%f %f\n", m_vals[j], m_counts[j]);

   fclose(fp);
   return 1;
}

void ossimHistogram::deleteAll()
{
   if (m_vals)
   {
      delete []m_vals;
      m_vals = NULL;
   }
   if (m_counts)
   {
      delete []m_counts;
      m_counts = NULL;
   }  
}

ossimHistogram::~ossimHistogram()
{
   deleteAll();
}


bool ossimHistogram::importHistogram(istream& in)
{
   ossimProprietaryHeaderInformation header;
   bool binsCreated = false;
   
   if(header.parseStream(in))
   {
      long numberOfBins = header.getNumberOfBins();
      
      if(numberOfBins)
      {
         create(numberOfBins, 0, numberOfBins - 1);
         binsCreated = true;

         if(binsCreated)
         {
            ossimString buffer;
            ossimString binNumber;
            ossimString count;

            while(in.good() &&
                  !in.eof() &&
                  *binNumber.c_str() != '.')
            {
               
               getline(in, buffer);

               istringstream s(buffer);

               s >> binNumber >> count;
               if(*binNumber.c_str() != (char)'.')
               {
                  SetCount((float)binNumber.toDouble(),
                           (float)count.toDouble());
               }
            }
         }
      }
      else
      {
         return false;
      }
   }  
   return true;
}

bool ossimHistogram::importHistogram(const ossimFilename& inputFile)
{
   if(inputFile.exists())
   {
      ifstream input(inputFile.c_str());

      return importHistogram(input);
   }

   return false;  
}


bool ossimHistogram::ossimProprietaryHeaderInformation::parseStream(istream& in)
{
   ossimString inputLine;
   
   getline(in, inputLine);  
   if(inputLine.find("File Type") != string::npos)
   {
      std::string::size_type index = inputLine.find(":");
      if(index != std::string::npos)
      {
         m_fileType = inputLine.substr(index+1);
         m_fileType = m_fileType.trim();
      }
      else
      {
         return false;
      }

   }
   else
   {
      return false;
   }

   getline(in, inputLine);  
   if(inputLine.find("Version") != string::npos)
   {
      std::string::size_type index = inputLine.find(":");
      if(index != std::string::npos)
      {
         m_version = inputLine.substr(index+1);
         m_version = m_version.trim();
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }

   getline(in, inputLine);  
   if(inputLine.find("Mapper Type") != string::npos)
   {
      std::string::size_type index = inputLine.find(":");
      if(index != std::string::npos)
      {
         m_mapperType = inputLine.substr(index+1);
         m_mapperType = m_mapperType.trim();
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }

   getline(in, inputLine);  
   if(inputLine.find("Number of Bins") != string::npos)
   {
      std::string::size_type index = inputLine.find(":");
      if(index != std::string::npos)
      {
         m_numberOfBins = inputLine.substr(index+1);
         m_numberOfBins = m_numberOfBins.trim();
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
   
   return true;   
}

bool ossimHistogram::saveState(ossimKeywordlist& kwl,
                               const char* prefix)const
{
   kwl.add(prefix,
           "type",
           "ossimHistogram",
           true);
   kwl.add(prefix,
           "number_of_bins",
           m_num,
           true);
   kwl.add(prefix,
           "min_value",
           m_vmin,
           true);
   kwl.add(prefix,
           "max_value",
           m_vmax,
           true);
   


   ossimString binArrayList = "(";
   bool firstValue = true;

   for(ossim_int32 index = 0; index < m_num; ++index)
   {
      if(fabs(m_counts[index]) > FLT_EPSILON)
      {

         if(!firstValue)
         {
            binArrayList += ",";
         }
         else
         {
            firstValue = false;
         }
         binArrayList += "("+ossimString::toString(index)+","+ossimString::toString(m_counts[index])+")";
     }
   }

   binArrayList += ")";

   kwl.add(prefix, "bins", binArrayList, true);
#if 0
   ossimString binValue = "";
   for(ossim_int32 index = 0; index < m_num; ++index)
   {
      if(fabs(m_counts[index]) > FLT_EPSILON)
      {
         //     binValue = prefix;
         binValue = "bin";
         binValue += ossimString::toString(index);
         
         kwl.add(prefix,
                 binValue.c_str(),
                 m_counts[index],
                 true);
      }
   }
#endif   
   return true;
}

bool ossimHistogram::loadState(const ossimKeywordlist& kwl,
                               const char* prefix)
{
//   std::cout << "ossimHistogram::loadState!!!!\n";
   const char* number_of_bins = kwl.find(prefix, "number_of_bins");

//   std::cout << "NBINS = " << number_of_bins << std::endl;
   if(number_of_bins)
   {
      ossim_uint32 bins = ossimString(number_of_bins).toUInt32();

//      std::cout << "BINS ======== " << bins << std::endl;
      if(bins > 0)
      {
         // setup some defaults
         float minValue = 0;
         float maxValue = bins - 1;

         // see if there is a range set for the data
         const char* min_value = kwl.find(prefix, "min_value");
         const char* max_value = kwl.find(prefix, "max_value");

         if(min_value)
         {
            minValue = (ossim_float32)ossimString(min_value).toDouble();
         }
         if(max_value)
         {
            maxValue = (ossim_float32)ossimString(max_value).toDouble();
         }

         create((int)bins, minValue, maxValue);
         float* countsPtr = GetCounts();
         memset(countsPtr, '\0', bins*sizeof(float));
         // this is new style histogram creation
         //
         ossimString binsString = kwl.find(prefix, "bins");
         if(!binsString.empty())
         {
            std::vector<ossimDpt> result;
            ossim::toVector(result, binsString);
            if(!result.empty())
            {
               ossim_uint32 idx = 0;
               for(idx = 0; idx < result.size();++idx)
               {
                  ossim_uint32 binIdx = static_cast<ossim_uint32>(result[idx].x);
                  if(binIdx < bins)
                  {
                     countsPtr[binIdx] = result[idx].y;
                  }
               }
            }
         }
         else
         {
            ossimKeywordlist binsKwl;
            ossim_uint32 offset = (ossim_uint32)(ossimString(prefix)+"bin").size();
            ossimString regExpression =  ossimString("^(") + ossimString(prefix) + "bin[0-9]+)";
            kwl.extractKeysThatMatch(binsKwl,regExpression);
            const ossimKeywordlist::KeywordMap& kwlMap = binsKwl.getMap();
            ossimKeywordlist::KeywordMap::const_iterator iter = kwlMap.begin();
            while(iter != kwlMap.end())
            {
               ossimString numberStr(iter->first.begin() + offset,
                                     iter->first.end());
               countsPtr[numberStr.toUInt32()] = ossimString(iter->second).toDouble();
               ++iter;
            }
         }

//         ossimKeywordlist kwl;
//         this->saveState(kwl);
//         std::cout << kwl << std::endl;
         
         return true;
#if 0
         // create the bins
         ossimString binNumber = "";
         ossimString regExpression =  ossimString("^(") + ossimString(prefix) + "bin[0-9]+)";
         vector<ossimString> keys = kwl.getSubstringKeyList( regExpression );
         ossim_uint32 numberOfBins = (ossim_uint32)keys.size();
         ossim_uint32 offset = (ossim_uint32)(ossimString(prefix)+"bin").size();

         std::vector<ossim_uint32> theNumberList(numberOfBins);
         ossim_uint32 idx = 0;
         for(idx = 0; idx < theNumberList.size();++idx)
         {
            ossimString numberStr(keys[idx].begin() + offset,
                                  keys[idx].end());
            theNumberList[idx] = numberStr.toUInt32();

         }
         
         float* countsPtr = GetCounts();
         memset(countsPtr, '\0', bins*sizeof(float));
         for(idx = 0; idx < numberOfBins;++idx)
         {
            const char* binCount = kwl.find(prefix, ossimString("bin") + ossimString::toString(theNumberList[idx]));
            countsPtr[theNumberList[idx]] = (float)ossimString(binCount).toDouble();
         }
#endif
      }
   }
   return true;
}

bool ossimHistogram::loadState(const ossimRefPtr<ossimXmlNode> xmlNode)
{
   ossimRefPtr<ossimXmlNode> binValues =  xmlNode->findFirstNode("binValues");
   ossimRefPtr<ossimXmlNode> minValueNode  =  xmlNode->findFirstNode("minValue");
   ossimRefPtr<ossimXmlNode> maxValueNode  =  xmlNode->findFirstNode("maxValue");

   if(binValues.valid())
   {
      ossim_uint32 count = 0;
      float minValue = 0.0;
      float maxValue = 0.0;
      std::vector<float> floatValues;
      std::istringstream in(binValues->getText());
      ossimString vString;
      while(!in.fail())
      {
         in>>vString;
         if(!in.fail())
         {
            floatValues.push_back(vString.toFloat32());
         }
      }
      count = (ossim_uint32)floatValues.size();
     
      if(count)
      {
         minValue = 0;
         maxValue = count - 1;

         if(minValueNode.valid())
         {
            minValue = minValueNode->getText().toFloat32();
         }
         if(maxValueNode.valid())
         {
            maxValue = maxValueNode->getText().toFloat32();
         }

         create(count, minValue, maxValue);
         float* countsPtr = GetCounts();
         ossim_uint32 idx = 0;
         for(idx = 0; idx < count; ++idx)
         {
            countsPtr[idx] = floatValues[idx];
         }
         return true;
      }
   }

   return false;
}

bool ossimHistogram::saveState(ossimRefPtr<ossimXmlNode> xmlNode)const
{
   ossimRefPtr<ossimXmlNode> binValues = new ossimXmlNode;
   xmlNode->setTag("ossimHistogram");
   xmlNode->addChildNode("minValue", ossimString::toString(m_vmin));
   xmlNode->addChildNode("maxValue", ossimString::toString(m_vmax));
   xmlNode->addChildNode("standardDeviation", ossimString::toString(m_standardDev));
   xmlNode->addChildNode("m_mean", ossimString::toString(m_mean));
   binValues->setTag("binValues");
   std::ostringstream out;

   ossim_int32 idx = 0;
   if(m_num > 0)
   {
      for(idx = 0; idx < m_num;++idx)
      {
         out << ossimString::toString(m_counts[idx], 8) << " ";
      }
      binValues->setText(out.str());
   }
   xmlNode->addChildNode(binValues.get());
   
   return true;
}
