//*******************************************************************
//
// License:  See top level LICENSE.txt file.
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
//********************************************************************
// $Id: ossimHistogram.h 19799 2011-06-30 18:41:26Z gpotts $
//

#ifndef ossimHistogram_HEADER
#define ossimHistogram_HEADER
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

   
   virtual void deleteAll();
   
    float * m_vals;            // histogram x array
                             // (value = midpoint of each bucket)
    float * m_counts;          // histogram y array ie. count[i] is
                             // the number of pixels with value within range
                             // of bucket i

    int m_num;                 // number of indices

    float m_delta;             // "Width" of each bucket on value axis
    float m_vmin, m_vmax;        // Maximum and minimum values on plot
    mutable float m_mean;               // Mean value of the distribution
    mutable float m_standardDev;       // 
  protected:


   class ossimProprietaryHeaderInformation
   {
   public:
      ossimProprietaryHeaderInformation(){clear();}

      bool parseStream(istream& in);

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
      HISTOGRAM_FILL_THIN_PLATE = 1
   };
// Constructors
    ossimHistogram();
    ossimHistogram(int xres, float min, float max);
    ossimHistogram(float*, float*, int);
    ossimHistogram(const ossimHistogram& his); // Copy constructor
    ossimHistogram(const ossimHistogram*, float width); // Resampling constructor

    /**
     * Uses samples array to establish a histogram with numBins:
     */
    ossimHistogram(const double* samples, ossim_uint32 size, ossim_uint32 numBins);

    virtual int GetIndex(float)const;
// Other histogram formation operations    
   ossimHistogram* fillInteriorEmptyBins(int type=HISTOGRAM_FILL_THIN_PLATE)const;
    ossimHistogram* Scale(float scale_factor); // Scale Transformation
    ossimHistogram* CumulativeGreaterThanEqual()const;// From density to cumulative
    ossimHistogram* CumulativeLessThanEqual()const;// From density to cumulative
    //Suppress non-peak values.
   ossimHistogram* NonMaximumSupress(int radius = 1, bool cyclic = false);
   void create(int xres, float val1, float val2);
  
// Attribute accessors
    void UpCount(float newval, float occurences=1);
    float GetCount(float uval)const;
    float SetCount(float pixelval, float count);

    float GetMinVal()const;
    float GetMaxVal()const;
    float GetMaxCount()const;

    float GetRangeMin()const
    {
       return m_vmin;
    }
    float GetRangeMax()const
    {
       return m_vmax;
    }
    float * GetVals()
    {
      m_statsConsistent = 0; // Values might change.
      return m_vals; 
    }
    const float * GetVals()const
    {
      m_statsConsistent = 0; // Values might change.
      return m_vals; 
    }

    float * GetCounts()
    { 
      m_statsConsistent = 0; // m_counts might change.
      return m_counts; 
    }

   const float * GetCounts()const
    { 
      //m_statsConsistent = 0; // m_counts might change.
      return m_counts; 
    }

    int GetRes()const
    { return m_num; }

    float GetBucketSize()const { return m_delta; }

    float * GetMinValAddr()
    { return m_vals+GetIndex(GetMinVal());  }

    float * GetMinCountAddr()
    { return m_counts+GetIndex(GetMinVal());  }

    const float * GetMinValAddr()const
    { return m_vals+GetIndex(GetMinVal());  }

    const float * GetMinCountAddr()const
    { return m_counts+GetIndex(GetMinVal());  }

    float ComputeArea(float low, float high)const;// bounded area
    float ComputeArea()const;//total area

    /*!
     * Returns the fraction of accumulation up to and including "val" bucket
     * from min divided by the total accumulation.
     * returns OSSIM_FLT_NAN if "val" is not between GetMinVal and GetMaxVal.
     */
    float getLowFractionFromValue(float val) const;
    
    /*!
     * Returns the fraction of accumulation down to and including "val" bucket
     * from max divided by the total accumulation.
     * returns OSSIM_FLT_NAN if "val" is not between GetMin() and GetMax().
     */
    float getHighFractionFromValue(float val) const;
    
    //Find bounds that clip off a given percent of the area
    float LowClipVal(float clip_fraction)const;
    float HighClipVal(float clip_fraction)const;

    float GetValFromIndex(ossim_uint32 idx)const;
    float GetMinValFromIndex(ossim_uint32 idx)const;
    float GetMaxValFromIndex(ossim_uint32 idx)const;
    int GetValIndex(float val)const;

    float GetMean()const;
    float GetStandardDev()const;

    void Print()const;
    void Dump(char *)const;
    int  WritePlot(const char* fname)const;
   virtual ~ossimHistogram();


   virtual bool importHistogram(const ossimFilename& inputFile);
   virtual bool importHistogram(istream& in);
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(ossimRefPtr<ossimXmlNode> xmlNode)const;
   virtual bool loadState(const ossimRefPtr<ossimXmlNode> xmlNode);
TYPE_DATA   
};

#endif