//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef ossimKMeansFilter_HEADER
#define ossimKMeansFilter_HEADER

#include <ossim/imaging/ossimImageSourceFilter.h>
#include <vector>

class ossimImageData;
class ossimImageSourceSequencer;
class ossimHistogram;

/***************************************************************************************************
 *
 * This class provides a pixel classification capability using the traditional K-means scheme for
 * partitioning datasets into K distinct groups according to pixel value. The output tile consists
 * of the input pixels remapped according to their classification to a digital number representing
 * the group. Null pixels are ignored and left null.
 *
 * The caller can set the digital values representing the classification groups. Otherwise they are
 * mapped to integers 1, 2, ..., K.
 *
 * The output pixel type will be the minimum necessary to hold K groups + null pixel. Typically
 * this would be UInt8. Multiple bands are treated separately, so the number of output bands is
 * the same as the number of input bands.
 *
 * This filter works best if a histogram is available for the input source being connected, since
 * this avoids having to scan the full image rect of the input source to establish the grouping.
 * Alternatively, the input image must be scanned in full first in order to establish the K groups.
 * This initial scan happens with a call to initialize (or the first getTile call will insure it is
 * run).
 *
 * Important Note:
 * This filter necessarily considers the full image AOI for establishing the classification
 * pixel ranges before filtering the input tiles. In other words, it doesn't compute the grouping
 * on a per-tile basis, since that would cause each tile running through to use a different
 * classification criteria. Therefore, it is important that if a sub-image AOI is being used by the
 * output sink, the cut-rect filter needs to be inserted on the input-side of this filter so that it
 * only considers pixels in the AOI. Alternatively, the AOI (in pixel coordinates) can be specified
 * to this class directly and only those pixels in the AOI will be used in computing the groupings.
 *
 **************************************************************************************************/
class OSSIM_DLL ossimKMeansFilter : public ossimImageSourceFilter
{
public:
   struct ClassificationGroup
   {
      double min;      // Pixels in group are => this, used for detecting convergence
      double max;      // Pixels in group are < this, used for detecting convergence
      double mean;
      double sigma;
      double new_mean;
      double n;        // number of samples collected for running mean
      ossim_uint32 dn; // Output pixel digital number representing the group
   };

   ossimKMeansFilter();

   /**
    * Quicker constructor handles connections to source and optional histogram.
    */
   ossimKMeansFilter(ossimImageSource* input_source, ossimMultiBandHistogram* histogram=0);

   ~ossimKMeansFilter();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& origin,
                                               ossim_uint32 resLevel=0);
   
   virtual ossimScalarType getOutputScalarType() const;
   
   /**
    * Sets the input source's histogram for quicker K-means analysis.
    */
   void setInputHistogram(ossimMultiBandHistogram* histo);

   /**
    * Defines how many classification groups will be resolved. The input connection should be made
    * prior so that the number of bands is known.
    * LIMITATION: Curently, only K values < 256 are supported.
    */
   void setNumGroups(ossim_uint32 K);
   
   /**
    * Optionally define the output digital numbers for each group to be used for remapping the
    * input pixels. K is the size of the array, resetting any previously set K value.
    * LIMITATION: Curently, only UInt8 supported, so dns and K values must be 0-255.
    */
   void setGroupPixelValues(const ossim_uint32* dns, ossim_uint32 K);
   
   /**
    * Callers may be interested in reporting the group statistics computed by this class. Returns
    * the groups (of size K) for the band index specified.
    */
   void getBandClassGroups(ClassificationGroup* groups, ossim_uint32& K, ossim_uint32 band=0);

   virtual double getNullPixelValue(ossim_uint32 band=0)const;
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;

   virtual void initialize();

   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=NULL)const;
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=NULL);

   void setVerbose(bool verbose=true) { m_verbose = verbose; }

protected:
   bool computeKMeans();
   bool computeKMeans(ossimImageSourceSequencer* sequencer, ossim_uint32 band);
   bool computeKMeans(ossimHistogram* band_histo, ossim_uint32 band);

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   void clear();
   
   ossimRefPtr<ossimMultiBandHistogram> m_histogram;
   ossim_uint32 m_numGroups; // a.k.a. K ;)
   ossim_uint32 m_numBands;
   std::vector<double> m_minPixelValue;
   std::vector<double> m_maxPixelValue;
   ossimRefPtr<ossimImageData> m_tile;
   ClassificationGroup** m_classGroups;
   ossimScalarType m_outputScalarType;
   int m_initLevel;
   bool m_verbose;

TYPE_DATA
};

#endif /* #ifndef ossimKMeansFilter_HEADER */
