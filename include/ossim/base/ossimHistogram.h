//---
//
// License: MIT
//
// See top level LICENSE.txt file for more.
//
// Author: Ken Melero 
//         Orginally developed by:
//                   Copyright (c) 1997 TargetJr Consortium
//               GE Corporate Research and Development (GE CRD)
//                             1 Research Circle
//                            Niskayuna, NY 12309
//         Adapted from:  IUE v4.1.2
// Description: 
//      A Histogram contains an array of "buckets", which represent finite
// segments of some value axis, along with a corresponding array of
// frequency m_counts for each of these buckets.
//
//---
// $Id$
//

#ifndef ossimHistogram_HEADER
#define ossimHistogram_HEADER 1
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimXmlNode.h>
class OSSIMDLLEXPORT ossimHistogram : public ossimObject
{
  private:

   mutable int m_statsConsistent; // A 2 bit state flag  Mean =1 | StandDev = 2

  protected:

   /**
    * @brief Updates m_vmin and m_vmax. This will be the value for the first
    * and last bin with a count in it.
    */
   void updateMinMax();

   virtual void deleteAll();
   
   double * m_vals;            // histogram x array
   // (value = midpoint of each bucket)
   ossim_int64* m_counts;   // histogram y array ie. count[i] is
                             // the number of pixels with value within range
                             // of bucket i

   int m_num;                 // number of indices

   double m_delta;             // "Width" of each bucket on value axis
   double m_vmin, m_vmax;        // Maximum and minimum values on plot
   mutable double m_mean;               // Mean value of the distribution
   mutable double m_standardDev;       //

   //---
   // For counting nulls only.
   // Nulls counted separately, i.e. not stored in a m_counts bin, as they are
   // not used in any computations.
   //---
   double        m_nullValue;
   ossim_uint64  m_nullCount;

   //---
   // This was added to determine if pixel values are integers.
   // If so save the state in this form for each bin: "(pixelValue,count)"
   // else: "(binIndex,count)"
   //---
   ossimScalarType m_scalarType;

   class ossimProprietaryHeaderInformation
   {
   public:
      ossimProprietaryHeaderInformation(){clear();}

      bool parseStream(std::istream& in);

      long getNumberOfBins()
         {
            return m_numberOfBins.toLong();
         }
      void clear()
         {
            m_fileType      = "";
            m_version       = "";
            m_mapperType    = "";
            m_numberOfBins  = "";
         }
      ossimString m_fileType;
      ossimString m_version;
      ossimString m_mapperType;
      ossimString m_numberOfBins;
   };
   
  public:
   enum FillAlgorithmType
   {
      HISTOGRAM_FILL_DEFAULT    = 0,
      HISTOGRAM_FILL_THIN_PLATE = 1,
      HISTOGRAM_FILL_LINEAR     = 2
   };

   // Constructors
   ossimHistogram();
   ossimHistogram(int xres, double min, double max, double nullValue, ossimScalarType scalar);
   // ossimHistogram(double*, double*, int);
   ossimHistogram(const ossimHistogram& his); // Copy constructor
   ossimHistogram(const ossimHistogram*, double width); // Resampling constructor

   /**
    * Uses samples array to establish a histogram with numBins:
    */
   ossimHistogram(const double* samples,
                  ossim_uint32 size,
                  ossim_uint32 numBins);

   virtual int GetIndex(double)const;

   // Other histogram formation operations

   /**
    * @brief Fill empty bins.
    *
    * This is used for code that does not like holes. The ossim-gui
    * HistogramWidget being one of them.
    *
    * @param interiorOnly If true only bins between first bin with a
    * count, and last bin with a count will be filled. If false, any
    * bins with a zero count will be set to a count of one.
    *
    * @param type Fill type. Currently thin plate spline is the only
    * type used regardless of setting.
    *
    * @return A new histogram from the this.
    */
   ossimRefPtr<ossimHistogram> fillEmptyBins(bool interiorOnly,
                                 int type=HISTOGRAM_FILL_THIN_PLATE)const;

   ossimRefPtr<ossimHistogram> Scale(double scale_factor); // Scale Transformation

   // From density to cumulative
   ossimRefPtr<ossimHistogram> CumulativeGreaterThanEqual()const;
   ossimRefPtr<ossimHistogram> CumulativeLessThanEqual()const;

   //Suppress non-peak values.
   ossimRefPtr<ossimHistogram> NonMaximumSupress(int radius = 1, bool cyclic = false);

   // void create(int xres, float val1, float val2);
   void create(int bins, double minValue, double maxValue,
               double nullValue, ossimScalarType scalar);

   // Attribute accessors
   void UpCount(double newval, double occurences=1);
   double GetCount(double uval)const;
   double SetCount(double pixelval, double count);

   double GetMinVal()const;
   double GetMaxVal()const;
   double GetMaxCount()const;

   double GetRangeMin()const
   {
      return m_vmin;
   }
   double GetRangeMax()const
   {
      return m_vmax;
   }
   double * GetVals()
   {
      m_statsConsistent = 0; // Values might change.
      return m_vals; 
   }
   const double * GetVals()const
   {
      m_statsConsistent = 0; // Values might change.
      return m_vals; 
   }

   ossim_int64 * GetCounts()
   {
      m_statsConsistent = 0; // m_counts might change.
      return m_counts; 
   }

   const ossim_int64 * GetCounts()const
   {
      //m_statsConsistent = 0; // m_counts might change.
      return m_counts; 
   }

   int GetRes()const
   { return m_num; }

   double GetBucketSize()const { return m_delta; }

   double * GetMinValAddr()
   { return m_vals+GetIndex(GetMinVal());  }

   ossim_int64 * GetMinCountAddr()
   { return m_counts+GetIndex(GetMinVal());  }

   const double * GetMinValAddr()const
   { return m_vals+GetIndex(GetMinVal());  }

   const ossim_int64 * GetMinCountAddr()const
   { return m_counts+GetIndex(GetMinVal());  }

   double ComputeArea(double low, double high)const;// bounded area
   double ComputeArea()const;//total area

   /*!
    * Returns the fraction of accumulation up to and including "val" bucket
    * from min divided by the total accumulation.
    * returns OSSIM_FLT_NAN if "val" is not between GetMinVal and GetMaxVal.
    */
   double getLowFractionFromValue(double val) const;

   /*!
    * Returns the fraction of accumulation down to and including "val" bucket
    * from max divided by the total accumulation.
    * returns OSSIM_FLT_NAN if "val" is not between GetMin() and GetMax().
    */
   double getHighFractionFromValue(double val) const;

   //Find bounds that clip off a given percent of the area
   double LowClipVal(double clip_fraction)const;
   double HighClipVal(double clip_fraction)const;

   double GetValFromIndex(ossim_uint32 idx)const;
   double GetMinValFromIndex(ossim_uint32 idx)const;
   double GetMaxValFromIndex(ossim_uint32 idx)const;
   int GetValIndex(double val)const;

   double GetMean()const;
   double GetStandardDev()const;

   void Print()const;
   void Dump(char *)const;
   int  WritePlot(const char* fname)const;
   virtual ~ossimHistogram();

   virtual bool importHistogram(const ossimFilename& inputFile);
   virtual bool importHistogram(std::istream& in);
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(ossimRefPtr<ossimXmlNode> xmlNode)const;
   virtual bool loadState(const ossimRefPtr<ossimXmlNode> xmlNode);

   ossimScalarType getScalarType() const;
   void setScalarType( ossimScalarType scalar );

   const double& getNullValue() const;
   void setNullValue(const double& nullValue);
   const ossim_uint64& getNullCount() const;
   void upNullCount( const ossim_uint64& count );

   // Inlined for pixel loops.
   void upNullCount()
   {
      ++m_nullCount;
   }
private:
   ossimRefPtr<ossimHistogram> fillEmptyBinsThinPlate(bool fillInterior)const;
   ossimRefPtr<ossimHistogram> fillEmptyBinsLinear(bool fillInterior)const;
TYPE_DATA   
};

#endif
