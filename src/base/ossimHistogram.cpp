//---
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
//---
// $Id$

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimHistogram.h>
#include <ossim/base/ossimNotifyContext.h>
#include <ossim/base/ossimScalarTypeLut.h>
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
   m_vals(new double [1]),
   m_counts(new ossim_int64 [1]),
   m_num(0),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0),
   m_nullValue(ossim::nan()),
   m_nullCount(0),
   m_scalarType(OSSIM_SCALAR_UNKNOWN)
{
   m_vals[0] = 0.0;
   m_counts[0] = 0.0;
}

ossimHistogram::ossimHistogram(int xres, double val1, double val2, double nullValue, ossimScalarType scalar)
   :
   m_statsConsistent(MEAN_FLAG | SD_FLAG),
   m_vals(new double [xres]),
   m_counts(new ossim_int64 [xres]),
   m_num(xres),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0),
   m_nullValue(nullValue),
   m_nullCount(0),
   m_scalarType(scalar)
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
      if ( ossim::isInteger(scalar) )
      {
         m_delta = (m_vmax - m_vmin + 1) / xres;
      }
      else
      {
         m_delta = (m_vmax - m_vmin) / xres;
      }
   }

   m_mean = (double)((m_vmax + m_vmin)/2.0);
   m_standardDev = (double)((m_vmax - m_vmin)/(2.0*sqrt(3.0)));
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
      if ( ossim::isInteger( m_scalarType ) )
      {
         for(i = 0; i < xres; i++)
         {
            m_vals[i] = m_vmin + m_delta * (double)i;
            m_counts[i] = 0;
         }
      }
      else
      {
         for(i = 0; i < xres; i++)
         {
            m_vals[i] = m_vmin + m_delta * (double)(i + 0.5);
            m_counts[i] = 0;
         }
      }
   }
}

#if 0
ossimHistogram::ossimHistogram(double* uvals, double* ucounts, int xres)
   :
   m_statsConsistent(MEAN_FLAG | SD_FLAG),
   m_vals(uvals),
   m_counts(ucounts),
   m_num(xres),
   m_delta(0.0),
   m_vmin(0),
   m_vmax(0),
   m_mean(0.0),
   m_standardDev(0.0),
   m_nullValue(ossim::nan()),
   m_nullCount(0),
   m_scalarType(OSSIM_SCALAR_UNKNOWN)
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
#endif

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
   m_standardDev(0.0),
   m_nullValue(ossim::nan()),
   m_nullCount(0),
   m_scalarType(OSSIM_SCALAR_UNKNOWN)
{
   if ((size == 0) || (xres == 0))
      return;

   // scan the dataset for min/max:
   m_vmin=(double)(data[0]);
   m_vmax=(double)(data[0]);
   for (ossim_uint32 i=1; i<size; ++i)
   {
      if ((double)(data[i]) < m_vmin)
         m_vmin = (double)(data[i]);
      else if ((double)(data[i]) > m_vmax)
         m_vmax = (double)(data[i]);
   }

   // Allocate histogram:

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

   m_vals = new double [m_num];
   m_counts = new ossim_int64 [m_num];
   for (ossim_int32 i=0; i<m_num; ++i)
   {
      m_vals[i] = m_vmin + m_delta * (i + 0.5);
      m_counts[i] = 0;
   }

   // compute histogram:
   for (ossim_uint32 i=0; i<size; i++)
      UpCount((double)(data[i]));

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
m_standardDev(0.0),
m_nullValue(his.m_nullValue),
m_nullCount(his.m_nullCount),
m_scalarType(his.m_scalarType)
{
   int i = 0;
   m_num = his.GetRes();

   m_vals = new double[m_num];
   const double* his_vals = his.GetVals();

   m_counts = new ossim_int64[m_num];
   const ossim_int64* his_counts = his.GetCounts();

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
ossimHistogram::ossimHistogram(const ossimHistogram* his, double width)
:
m_statsConsistent(0),
m_vals(0),
m_counts(0),
m_num(0),
m_delta(0.0),
m_vmin(0),
m_vmax(0),
m_mean(0.0),
m_standardDev(0.0),
m_nullValue(ossim::nan()),
m_nullCount(0),
m_scalarType(OSSIM_SCALAR_UNKNOWN)
{
   if ( his )
   {
      m_statsConsistent =0;

      // Attributes of original histogram
      double del = his->GetBucketSize();
      int max_index = his->GetRes() - 1;
      double minvalue = his->GetVals()[0] - del*.5f;
      double maxvalue = his->GetVals()[max_index] + del*.5f;

      // Intialize a new histogram
      m_nullValue = his->getNullValue();
      m_nullCount = his->getNullCount();
      m_scalarType = his->getScalarType();

      if(width == del) m_num = his->GetRes();
      else if(!(width == 0.0))
      {
         m_num = (int)ceil((maxvalue - minvalue)/width);
         if ( m_nullCount )
         {
            m_nullCount /= width; // ??? drb
         }
      }
      else
         m_num = 1; // This shouldn't happen anyway.

      m_vals = new double [m_num];
      m_counts = new ossim_int64 [m_num];
      m_delta = width;
      double mean_val = (maxvalue + minvalue)/2.0f;
      double half_range = (m_num * m_delta)/2.0f;
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
            m_counts[i] = 0;
         }
      }


// Cases:


      if(width == del)    // Then just copy his
      {
         const ossim_int64* his_counts = his->GetCounts();
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
         double his_start = minvalue + .5f*del;
         double start = m_vmin + .5f*m_delta;
         double c0 = his->GetCount(his_start);
         double c1 = his->GetCount(his_start + del);
         double s0 = (c1 - c0)/del;

         for(double x = start; x <= (his_start + del + m_delta);)
         {
            double interp = s0 * (x - his_start) + c0;
            if(interp < 0) interp = 0; //Can be negative
            SetCount(x,interp);
            x += width;
         }
//    End
         double his_end = maxvalue - .5f*del;
         double end = m_vmax - .5f*m_delta;
         double cn = his->GetCount(his_end);
         double cn_1 = his->GetCount(his_end - del);
         double sn = (cn_1 - cn)/del;

         for(double y = end; y >= (his_end - del + m_delta);)
         {
            double interp = sn * (his_end - y) + cn;
            if(interp < 0) interp = 0; //Can be negative
            SetCount(y, interp);
            y -= m_delta;
         }
// Interior Loop

         for(double z = his_start + del; z <= (his_end - del);)
         {
            double ci = his->GetCount(z);
            double ci_1 = his->GetCount(z-del);
            double cip1 = his->GetCount(z+del);
            double deriv = (cip1 - ci_1)/(2.0f*del);
            double second_drv =
               ((cip1 + ci_1)/2.0f - ci)/(del*del);
            int fine_x_index = GetIndex(z);
            if (fine_x_index < 0)
            {
               if (z<m_vmin) fine_x_index = 0;
               else fine_x_index = m_num-1;
            }
            double fine_x = m_vals[fine_x_index];
            for(double xfine = fine_x; xfine < z + del;)
            {
               double interp = ci + deriv*(xfine -z) +
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
            double his_start = minvalue + .5f*del;
            double his_end = maxvalue - .5f*del;
            for(double x = his_start; x <= his_end;)
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
}

#if 0
void ossimHistogram::create(int xres, double val1, double val2)
{
   // clear all the data
   deleteAll();

   // now set it up and initialize;
   xres = xres >0? xres:1;
   
   m_vals   = new double [xres];
   m_counts = new ossim_int64 [xres];
   m_num = xres;
   m_vmax = MAX(val1, val2);
   m_vmin = MIN(val1, val2);

   if ( (m_vmax - m_vmin + 1) == xres )
   {
      m_delta = 1.0;
   }
   else
   {
      m_delta = (m_vmax - m_vmin) / xres;
   }

   m_mean = (double)((m_vmax + m_vmin)/2.0);
   m_standardDev = (double)((m_vmax - m_vmin)/(2.0*sqrt(3.0)));
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
         m_vals[i] = m_vmin + m_delta * (double)(i + 0.5);
         m_counts[i] = 0;
      }
   }   
}
#endif

void ossimHistogram::create(
   int bins, double minValue, double maxValue, ossim_float64 nullValue, ossimScalarType scalar)
{
   // create( bins, minValue, maxValue);

   // clear all the data
   deleteAll();

   // Must set null value after create call.
   m_nullValue = nullValue;
   m_scalarType = scalar;

   // now set it up and initialize;
   bins = bins >0? bins:1;

   m_vals   = new double [bins];
   m_counts = new ossim_int64 [bins];
   m_num = bins;
   m_vmax = MAX(minValue, maxValue);
   m_vmin = MIN(minValue, maxValue);

   if ( (m_vmax - m_vmin + 1) == bins )
   {
      m_delta = 1.0;
   }
   else
   {
      m_delta = (m_vmax - m_vmin) / bins;
   }

   m_mean = (double)((m_vmax + m_vmin)/2.0);
   m_standardDev = (double)((m_vmax - m_vmin)/(2.0*sqrt(3.0)));
   m_statsConsistent = 0;
   m_statsConsistent |= (MEAN_FLAG | SD_FLAG);
   // int i = 0;
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
      if (ossim::isInteger( m_scalarType ) )
      {
         for(int i = 0; i < bins; ++i)
         {
            m_vals[i] = m_vmin + m_delta * i;
            m_counts[i] = 0;
         }
      }
      else
      {
         for(int i = 0; i < bins; ++i)
         {
            m_vals[i] = m_vmin + m_delta * (double)(i + 0.5);
            m_counts[i] = 0;
         }
      }
   }
}

ossimHistogram* ossimHistogram::fillEmptyBins(bool interiorOnly, int type) const
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
         ossim_int64* new_counts = result->GetCounts();
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

         if (interiorOnly == false )
         {
            for ( ossim_int32 idx = 0; idx < m_num; ++idx )
            {
               if ( new_counts[idx] == 0 ) new_counts[idx] = 1;
            }
            result->updateMinMax();
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

ossimHistogram* ossimHistogram::Scale(double scale_factor)
{

// Extract attributes of self

//    double lowvalue = m_vals[0];
   double highvalue = m_vals[m_num-1];

// Construct a new histogram

   ossimHistogram* scaled_his = new ossimHistogram(this, m_delta);
   ossim_int64* new_counts = scaled_his->GetCounts();
   int i = 0;
   for(i=0; i < m_num; i++)  // Initialize
      new_counts[i] = 0;

// Compute scaled values
// We assume that the new histogram is to be scaled down from his

   double scale = scale_factor;
   if(scale_factor > 1.0) scale = 1.0;

   for(double x = highvalue; x > m_vmin;)
   {
      double trans_x = (x-m_vmin)*scale + m_vmin; // Scaled x.
      int index = GetIndex(trans_x);
      if (index < 0)
      {
         if (trans_x<m_vmin) index = 0;
         else index = m_num-1;
      }
      double fraction = (trans_x - m_vals[index])/m_delta;
      double abs_fraction = (double)fabs(fraction);
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
   const ossim_int64* density_counts = this->GetCounts();
   int res = this->GetRes();

   // Intitialize cumulative m_counts
   ossim_int64* cum_counts = cum_his->GetCounts();
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
   const ossim_int64* density_counts = this->GetCounts();
   int res = this->GetRes();

   // Intitialize cumulative m_counts
   ossim_int64* cum_counts = cum_his->GetCounts();
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
inline double GetExtendedCount(int bin, int n_bins, ossim_int64* cnts, bool cyclic)
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
inline void RemoveFlatPeaks(int nbins, ossim_int64* cnts, bool cyclic)
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
      double v = GetExtendedCount(i, nbins, cnts, cyclic);

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
   ossim_int64* counts_old = this->GetCounts();

   //Make a new Histogram for the suppressed version
   ossim_int64* counts_new = h_new->GetCounts();
   int i;
   for( i =0; i < n_buckets; i++)
      counts_new[i] = 0;
  
   //Find local maxima
   for( i = 0; i<  n_buckets; i++)
   {
      //find the maxium value in the current kernel
      double max_count = counts_old[i];
      int k = 0;
      for(k = -radius; k <= radius ;k++)
      {
         int index = i+k;
         double c = GetExtendedCount(index, n_buckets, counts_old, cyclic);
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
double ossimHistogram::GetMean()const
{
   double xsum = 0.0;

   if(MEAN_FLAG&m_statsConsistent)
      return m_mean;
   else
   {
      if( this->GetBucketSize() > 0.0){
         for(double x=this->GetMinVal(); x<= this->GetMaxVal(); x +=this->GetBucketSize())
            xsum += x*GetCount(x);
      }

      double area = ComputeArea(m_vmin, m_vmax);
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



double ossimHistogram::GetStandardDev()const
{
   double sum = 0.0;

   if(SD_FLAG&m_statsConsistent)
      return m_standardDev;
   else
   {
      double xm = this -> GetMean(); // Force an Update of m_mean

      if( this->GetBucketSize() > 0.0){
         for(double x=this->GetMinVal();
             x<= this->GetMaxVal();
             x +=this->GetBucketSize())

            sum += (x-xm)*(x-xm)*GetCount(x);
      }

      double area = ComputeArea(m_vmin, m_vmax);
      if(area <= 0.0)
      {
         //	      fprintf(stderr, "Histogram : Area <= 0.0\n");
         return 0.0;
      }
      else
      {
         m_statsConsistent |= 2;
         m_standardDev = (double)sqrt(sum/area);
         return m_standardDev;
      }
   }
}

int ossimHistogram::GetIndex(double pixelval)const
{
#if 1
   if ((pixelval > m_vmax) || (pixelval < m_vmin) || (m_num==0) )
   {
      return -1;
   }
//   ossim_float64 d = m_vmax-m_vmin;
   int bandIdx = (ossim_int32)((pixelval-m_vmin)/m_delta);
   return bandIdx<m_num?bandIdx:-1;
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
double ossimHistogram::GetMinValFromIndex(ossim_uint32 idx)const
{
   double result = 0.0;
   
   if((int)idx < m_num)
   {
      result = (m_vals[idx]-(0.5 * m_delta));
      if(result < m_vmin) result = m_vmin;
   }

   return result;
}

double ossimHistogram::GetMaxValFromIndex(ossim_uint32 idx)const
{
   double result = 0.0;
   
   if((int)idx < m_num)
   {
      result = (m_vals[idx]+(0.5 * m_delta));
      if(result > m_vmax) result = m_vmax;
   }

   return result;  
}

double ossimHistogram::GetValFromIndex(ossim_uint32 idx)const
{
   double result = 0.0;
   if((int)idx < m_num)
   {
      result = m_vals[idx];
   }

   return result;
}

int ossimHistogram::GetValIndex(double pixelval)const
{
   int idx = -1;
   if ( (pixelval >= m_vmin) && (pixelval <= m_vmax) )
   {
      for(int i = 0; i < m_num; ++i)
      {
         if ((pixelval > (m_vals[i] - 0.5 * m_delta)) &&
             (pixelval <= (m_vals[i] + 0.5 * m_delta)))
         {
            idx = i;
            break;
         }
      }
   }
   return idx;
}

double ossimHistogram::GetCount(double pixelval)const
{
   int index = GetIndex(pixelval);

   if (index < 0)
      return -1;
   else
      return m_counts[index];
}

double ossimHistogram::GetMinVal()const
{
   int i=0;

   while (i<m_num-1 && !m_counts[i])
      i++;

   return m_vals[i];
}

double ossimHistogram::GetMaxVal()const
{
   int i=m_num-1;

   while (i>0 && !m_counts[i])
      i--;

   if (i < 0)
      return 0.0;

   return m_vals[i];
}


double ossimHistogram::GetMaxCount()const
{
   int i=0;
   double max;
   max = 0.0;
   for (i=0; i < m_num; i++)
      if (m_counts[i] > max)
         max = m_counts[i];
   return max;
}




double ossimHistogram::SetCount(double pixelval, double count)
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


void ossimHistogram::UpCount(double pixelval, double occurences)
{

   m_statsConsistent = 0;
   int idx = GetIndex(pixelval);
   if (idx >= 0)  // Originally (index > 0)
   {
      m_counts[idx] += (ossim_int64)occurences;
   }
}

double ossimHistogram::ComputeArea(double low, double high)const
{
   double sum = 0.0;
   double maxval = GetMaxVal();
   double minval = GetMinVal();

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
double ossimHistogram::ComputeArea()const
{
   double m_vmin = this->GetMinVal();
   double m_vmax = this->GetMaxVal();
   if(m_vmin>m_vmax)
   {
      double temp = m_vmin;
      m_vmin = m_vmax;
      m_vmax = temp;
   }
   return this->ComputeArea(m_vmin, m_vmax);
}

double ossimHistogram::getLowFractionFromValue(double val) const
{
   ossim_float64 result = ossim::nan();
   int cutoff_bucket = GetValIndex(val);
   if ( cutoff_bucket > -1 )
   {
      int total_buckets = GetRes();
      double partial_sum = 0.0;
      double total_sum   = 0.0;
      for(int i = 0; i < total_buckets; ++i)
      {
         total_sum += m_counts[i];
         if (i <= cutoff_bucket)
         {
            partial_sum += m_counts[i];
         }
      }
      result = partial_sum/total_sum;
   }

#if 0 /* Please leave for debug. */
   std::cout << "ossimHistogram::getLowFractionFromValue debug:\n"
             << "val: " << val
             << " cutoff_bucket" << cutoff_bucket
             << " result: " << result << "\n";
#endif

   return result;
}

double ossimHistogram::getHighFractionFromValue(double val) const
{
   ossim_float64 result = ossim::nan();
   int cutoff_bucket = GetValIndex(val);
   if ( cutoff_bucket > -1 )
   {
      int total_buckets = GetRes();
      double partial_sum = 0.0;
      double total_sum   = 0.0;
      for(int i = (total_buckets-1); i >= 0; --i)
      {
         total_sum += m_counts[i];
         if (i >= cutoff_bucket)
         {
            partial_sum += m_counts[i];
         }
      }
      result = partial_sum/total_sum;
   }

#if 0 /* Please leave for debug. */
   std::cout << "ossimHistogram::getHighFractionFromValue debug:\n"
             << "val: " << val
             << " cutoff_bucket" << cutoff_bucket
             << " result: " << result << "\n";
#endif
   
   return result;
}

//----------------------------------------------------------------------
//  -- Finds the lower bound value which elminates a given fraction of
//     histogram area.
//
double ossimHistogram::LowClipVal(double clip_fraction)const
{
   if(clip_fraction<0) clip_fraction=0.0;
   if(clip_fraction>1.0) clip_fraction=1.0;
   double area = this->ComputeArea();
   if(area==0.0) return this->GetMinVal();
   if(clip_fraction==0.0) return this->GetMinVal();
   if(clip_fraction==1.0) return this->GetMaxVal();
   double clip_area = area*clip_fraction;
   const ossim_int64* m_counts = this->GetCounts();
   const double* m_vals = this->GetVals();
   int res = this->GetRes();
   double sum = 0;
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
double ossimHistogram::HighClipVal(double clip_fraction)const
{
   if(clip_fraction<0) clip_fraction=0.0;
   if(clip_fraction>1.0) clip_fraction=1.0;
   double area = this->ComputeArea();
   if(area==0.0) return this->GetMaxVal();
   if(clip_fraction==0.0) return this->GetMaxVal();
   if(clip_fraction==1.0) return this->GetMinVal();
   double clip_area = area*clip_fraction;
   const ossim_int64* m_counts = this->GetCounts();
   const double* m_vals = this->GetVals();
   int res = this->GetRes();
   double sum = 0;
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
   const double* m_vals = this->GetVals();
   const ossim_int64* m_counts = this->GetCounts();
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
   out << "\n MaxVal " << this->GetMaxVal() << "\n"
       << " MinVal " << this->GetMinVal() << "\n"
       << " BucketSize " << this->GetBucketSize() << "\n"
       << " Resolution " << this->GetRes() << "\n"
       << " Area "
       << this->ComputeArea(this->GetMinVal(),this->GetMaxVal()) << "\n"
       << " Scalar type "
       << ossimScalarTypeLut::instance()->getEntryString( m_scalarType ).c_str()
       << "\n"
       << "------------------------------------------------\n\n";
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
      fprintf(dumpfp, "%f %lld\n", m_vals[i], m_counts[i]);
  
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
      fprintf(fp, "%f %lld\n", m_vals[j], m_counts[j]);

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
   m_nullValue = ossim::nan();
   m_nullCount = 0;
   m_scalarType = OSSIM_SCALAR_UNKNOWN;
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
         // create(numberOfBins, 0, numberOfBins - 1);
         create(numberOfBins, 0, numberOfBins - 1, m_nullValue, m_scalarType);
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
                  SetCount((double)binNumber.toDouble(),
                           (double)count.toDouble());
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

   //---
   // Counting nulls not implemented completely so test and only save if
   // initialized.
   //---
   if ( ossim::isnan(m_nullValue) == false )
   {
      kwl.add(prefix,
              "null_value",
              m_nullValue,
              true);
   }
   if ( m_nullCount > 0 )
   {
      kwl.add(prefix,
              "null_count",
              m_nullCount,
              true);
   }

   if ( m_scalarType != OSSIM_SCALAR_UNKNOWN )
   {
      kwl.add(prefix,
              "scalar_type",
              ossimScalarTypeLut::instance()->getEntryString( m_scalarType ).c_str(),
              true);
   }

   ossimString binArrayList = "(";
   bool firstValue = true;

   //---
   // If we know the scalar type, and it is an interger, use the form of
   // "(pixel_value,count)"; if not, then use "(bin_index,count)".
   // Note "bin_index" may or may not be the same as the pixel value.
   //---
   if ( ossim::isInteger( m_scalarType ) )
   {
      ossim_int32 pixelValue = 0;
      ossim_int64 count = 0;
      for(ossim_int32 index = 0; index < m_num; ++index)
      {
         if( m_counts[index] > 0 )
         {
            if(!firstValue)
            {
               binArrayList += ",";
            }
            else
            {
               firstValue = false;
            }
            pixelValue = GetValFromIndex( index );
            count = m_counts[index];
            binArrayList += "("+ossimString::toString(pixelValue)+","
               +ossimString::toString(count)+")";
         }
      }   
   }
   else
   {
      ossim_int64 count = 0;
      for(ossim_int32 index = 0; index < m_num; ++index)
      {
         if( m_counts[index] > 0 )
         {
            if(!firstValue)
            {
               binArrayList += ",";
            }
            else
            {
               firstValue = false;
            }
            count = m_counts[index];
            binArrayList += "("+ossimString::toString(index)+","
               +ossimString::toString(count)+")";
         }
      }
   }

   binArrayList += ")";

   kwl.add(prefix, "bins", binArrayList, true);

   return true;
}

bool ossimHistogram::loadState(const ossimKeywordlist& kwl,
                               const char* prefix)
{
   // std::cout << "ossimHistogram::loadState!!!!\n";
   const char* number_of_bins = kwl.find(prefix, "number_of_bins");

   // std::cout << "NBINS = " << number_of_bins << std::endl;

   if(number_of_bins)
   {
      ossim_uint32 bins = ossimString(number_of_bins).toUInt32();

//      std::cout << "BINS ======== " << bins << std::endl;
      if(bins > 0)
      {
         // setup some defaults
         double minValue = 0;
         double maxValue = bins - 1;

         // see if there is a range set for the data
         const char* min_value = kwl.find(prefix, "min_value");
         const char* max_value = kwl.find(prefix, "max_value");
         if(min_value)
         {
            minValue = (ossim_float64)ossimString(min_value).toDouble();
         }
         if(max_value)
         {
            maxValue = (ossim_float64)ossimString(max_value).toDouble();
         }

         // Must do null stuff after create.
         const char* null_value = kwl.find(prefix, "null_value");
         const char* null_count = kwl.find(prefix, "null_count");
         if(null_value)
         {
            m_nullValue = ossimString(null_value).toDouble();
         }
         if(null_count)
         {
            m_nullCount = ossimString(null_count).toUInt64();
         }

         //---
         // If "scalar_type" is set(in histogram file) and and of integer type,
         // then the histogram should be in the form of "(pixel_value,count)";
         // if not, it should be "(bin_index,count)" where "bin_index" may or
         // may not be the same as the pixel value.
         //---
         const char* scalar_type = kwl.find(prefix, "scalar_type");
         bool indexesArePixelValues = false;
         if ( scalar_type )
         {
            m_scalarType = ossimScalarTypeLut::instance()->getScalarTypeFromString(scalar_type);
            indexesArePixelValues = ossim::isInteger( m_scalarType );
         }

         create((int)bins, minValue, maxValue, m_nullValue, m_scalarType);

         ossim_int64* countsPtr = GetCounts();
         memset(countsPtr, '\0', bins*sizeof(ossim_int64));

         ossimString binsString = kwl.find(prefix, "bins");
         if(!binsString.empty())
         {
            std::vector<ossimDpt> result;
            ossim::toVector(result, binsString);
            if(!result.empty())
            {
               ossim_int32 idx = 0;
               ossim_int32 binIdx = 0;
               if ( indexesArePixelValues )
               {
                  for(idx = 0; idx < (ossim_int32)result.size();++idx)
                  {
                     // Get index from pixel value.
                     binIdx = GetIndex( (double)result[idx].x );
                     if( (binIdx >= 0) && (binIdx < (ossim_int32)bins) )
                     {
                        countsPtr[binIdx] = result[idx].y;
                     }
                  }
               }
               else
               {
                  for(idx = 0; idx < (ossim_int32)result.size();++idx)
                  {
                     binIdx = static_cast<ossim_int32>(result[idx].x);
                     if( (binIdx >= 0) && (binIdx < (ossim_int32)bins) )
                     {
                        countsPtr[binIdx] = result[idx].y;
                     }
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
         
         double* countsPtr = GetCounts();
         memset(countsPtr, '\0', bins*sizeof(double));
         for(idx = 0; idx < numberOfBins;++idx)
         {
            const char* binCount = kwl.find(prefix, ossimString("bin") + ossimString::toString(theNumberList[idx]));
            countsPtr[theNumberList[idx]] = (double)ossimString(binCount).toDouble();
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
      double minValue = 0.0;
      double maxValue = 0.0;
      std::vector<double> doubleValues;
      std::istringstream in(binValues->getText());
      ossimString vString;
      while(!in.fail())
      {
         in>>vString;
         if(!in.fail())
         {
            doubleValues.push_back(vString.toFloat64());
         }
      }
      count = (ossim_uint32)doubleValues.size();
     
      if(count)
      {
         minValue = 0;
         maxValue = count - 1;

         if(minValueNode.valid())
         {
            minValue = minValueNode->getText().toFloat64();
         }
         if(maxValueNode.valid())
         {
            maxValue = maxValueNode->getText().toFloat64();
         }

         // Fix to add/get null and scalar from xml.
         create(count, minValue, maxValue, m_nullValue, m_scalarType);
         ossim_int64* countsPtr = GetCounts();
         ossim_uint32 idx = 0;
         for(idx = 0; idx < count; ++idx)
         {
            countsPtr[idx] = (ossim_int64)doubleValues[idx];
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
         out << ossimString::toString(m_counts[idx]) << " ";
      }
      binValues->setText(out.str());
   }
   xmlNode->addChildNode(binValues.get());
   
   return true;
}


ossimScalarType ossimHistogram::getScalarType() const
{
   return m_scalarType;
}

void ossimHistogram::setScalarType( ossimScalarType scalar )
{
   m_scalarType = scalar;
}

const double& ossimHistogram::getNullValue() const
{
   return m_nullValue;
}

void ossimHistogram::setNullValue(const double& nullValue)
{
   m_nullValue = nullValue;
}

const ossim_uint64& ossimHistogram::getNullCount() const
{
   return m_nullCount;
}

void ossimHistogram::upNullCount( const ossim_uint64& count )
{
   m_nullCount += count;
}

void ossimHistogram::updateMinMax()
{
   m_vmin = GetMinVal();
   m_vmax = GetMaxVal();
}
