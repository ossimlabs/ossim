//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/imaging/ossimKMeansFilter.h>
#include <ossim/base/ossimFilenameProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimException.h>
#include <ossim/imaging/ossimImageDataFactory.h>
#include <ossim/imaging/ossimImageSourceSequencer.h>
#include <ossim/imaging/ossimImageStatisticsSource.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/imaging/ossimImageHistogramSource.h>
#include <ossim/imaging/ossimHistogramWriter.h>

RTTI_DEF1(ossimKMeansFilter, "ossimKMeansFilter", ossimImageSourceFilter);

static const char* NUM_GROUPINGS_KW = "num_groupings";

ossimKMeansFilter::ossimKMeansFilter()
:   ossimImageSourceFilter(),
    m_numClusters(0),
    m_tile(0),
    m_outputScalarType(OSSIM_SCALAR_UNKNOWN),
    m_initialized(false)
{
   setDescription("K-Means pixel classification filter.");
}

ossimKMeansFilter::ossimKMeansFilter(ossimImageSource* input_source,
                                     ossimMultiBandHistogram* histogram)
:   ossimImageSourceFilter(input_source),
    m_histogram (histogram),
    m_numClusters(0),
    m_tile(0),
    m_outputScalarType(OSSIM_SCALAR_UNKNOWN),
    m_initialized(false)
{
   setDescription("K-Means pixel classification filter.");
}

ossimKMeansFilter::~ossimKMeansFilter()
{
}

void ossimKMeansFilter::setInputHistogram(ossimMultiBandHistogram* histo)
{
   m_histogram = histo;
   m_initialized = 0;
}

ossimRefPtr<ossimImageData> ossimKMeansFilter::getTile(const ossimIrect& tileRect,
                                                       ossim_uint32 resLevel)
{
   if(!theInputConnection || (m_numClusters == 0))
      return 0;

   if (!m_initialized)
   {
      initialize();
      if (!m_initialized)
         return 0;
   }
   if (m_classifiers.empty() && !computeKMeans())
      return 0;

   ossimRefPtr<ossimImageData> inTile = theInputConnection->getTile(tileRect, resLevel);
   if (!inTile || !inTile->getBuf())
      return 0;

   if(!m_tile)
   {
      allocate();
      if (!m_tile)
         return 0;
   }

   m_tile->setImageRectangle(tileRect);
   m_tile->makeBlank();

   // Quick handling special case of empty input tile:
   if (inTile->getDataObjectStatus() == OSSIM_EMPTY)
      return m_tile;

   // Since a histogram is being used, the bin value reflects a range:
   ossimKMeansClustering* bandClusters = 0;
   const ossimKMeansClustering::Cluster* cluster = 0;
   ossim_uint8* outBuf; // TODO: Only K < 256 is currently supported
   double null_pixel = theInputConnection->getNullPixelValue();
   double pixel;
   ossimIpt ipt;
   ossim_uint32 offset = 0;
   ossim_uint32 numBands = getNumberOfInputBands();
   for (ossim_uint32 band=0; band<numBands; ++band)
   {
      // Need bin size of histogram since only center values were used in clustering:
      double delta = m_histogram->getHistogram(band)->GetBucketSize() / 2.0;
      bandClusters = m_classifiers[band].get();
      outBuf = (ossim_uint8*)(m_tile->getBuf(band));
      for (ipt.y=tileRect.ul().y; ipt.y<=tileRect.lr().y; ++ipt.y)
      {
         for (ipt.x=tileRect.ul().x; ipt.x<=tileRect.lr().x; ++ipt.x)
         {
            pixel = inTile->getPix(ipt, band);
            if (pixel != null_pixel)
            {
               // Determine its group and remap it using the group's DN:
               for (ossim_uint32 gid=0; gid<m_numClusters; ++gid)
               {
                  cluster = bandClusters->getCluster(gid);
                  if (!cluster)
                     continue;
                  if ((pixel >= (cluster->min-delta)) && (pixel <= (cluster->max+delta)))
                  {
                     outBuf[offset] = (ossim_uint8) m_pixelValues[gid];
                     break;
                  }
               }
            }
            ++offset;
         }
      }
   } // end loop over bands

   m_tile->validate();
   return m_tile;
}

void ossimKMeansFilter::allocate()
{
   if (!m_initialized)
   {
      initialize();
      if (!m_initialized)
         return;
   }

   m_tile = ossimImageDataFactory::instance()->create(this, getNumberOfInputBands(), this);
   if(!m_tile.valid())
      return;

   ossim_uint32 numBands = getNumberOfInputBands();
   if (m_numClusters && (m_classifiers.size() == numBands))
   {
      for (ossim_uint32 band=0; band<numBands; band++)
      {
         double min = m_classifiers[band]->getCluster(0)->min;
         double max = m_classifiers[band]->getCluster(0)->max;

         for (ossim_uint32 gid=1; gid<m_numClusters; gid++)
         {
            if (m_classifiers[band]->getCluster(gid)->min < min)
               min = m_classifiers[band]->getCluster(gid)->min;
            if (m_classifiers[band]->getCluster(gid)->max > max)
               max = m_classifiers[band]->getCluster(gid)->max;
         }
         m_tile->setMinPix(min, band);
         m_tile->setMaxPix(max, band);
      }
   }
   m_tile->initialize();
}

void ossimKMeansFilter::initialize()
{
   // This assigns theInputConnection if one is there.
   m_initialized = false;
   ossimImageSourceFilter::initialize();
   m_tile = 0;

   if ( !theInputConnection )
      return;

   // If an input histogram was provided, use it. Otherwise compute one:
   if (!m_histogram.valid())
   {
      ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
      histoSource->connectMyInputTo(theInputConnection);
      histoSource->setComputationMode(OSSIM_HISTO_MODE_FAST);
      histoSource->setMaxNumberOfRLevels(1);
      histoSource->execute();
      m_histogram = histoSource->getHistogram()->getMultiBandHistogram(0);
   }

   if (!m_histogram.valid())
   {
      ostringstream xmsg;
      xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Could not establish a histogram. Cannot "
            "initialize filter";
      throw ossimException(xmsg.str());
   }

   ossim_uint32 numBands = getNumberOfInputBands();
   for (ossim_uint32 band=0; band<numBands; band++)
   {
      ossimRefPtr<ossimHistogram> h = m_histogram->getHistogram(band);
      m_minPixelValue.push_back(h->GetRangeMin());
      m_maxPixelValue.push_back(h->GetRangeMax());
   }
   m_initialized = true;
}

bool ossimKMeansFilter::computeKMeans()
{
   ostringstream xmsg;
   if (m_numClusters == 0)
   {
      xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Number of groups has not been initialized!";
      throw ossimException(xmsg.str());
   }

   if (!m_initialized)
      initialize();

   ossim_uint32 numBands = getNumberOfInputBands();
   for (ossim_uint32 band=0; band<numBands; band++)
   {
      ossimRefPtr<ossimHistogram> band_histo = m_histogram->getHistogram(band);
      if (!band_histo.valid())
      {
         xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Null band histogram returned!";
         throw ossimException(xmsg.str());
      }

      ossimRefPtr<ossimKMeansClustering> classifier = new ossimKMeansClustering;
      classifier->setVerbose();
      classifier->setNumClusters(m_numClusters);
      classifier->setSamples(band_histo->GetVals(), band_histo->GetRes());
      classifier->setPopulations(band_histo->GetCounts(), band_histo->GetRes());
      if (!classifier->computeKmeans())
      {
         cout<<"ossimKMeansFilter:"<<__LINE__<<" No K-means clustering data available."<<endl;
         break;
      }
      m_classifiers.push_back(classifier);
   }

   return (m_classifiers.size() == numBands);
}


void ossimKMeansFilter::clear()
{
   m_classifiers.clear();
   m_numClusters = 0;
   m_initialized = false;
}

void ossimKMeansFilter::setNumClusters(ossim_uint32 K)
{
   if (K > 255)
   {
      ostringstream xmsg;
      xmsg << "ossimKMeansFilter:"<<__LINE__<<" Requested K="<<K<<" but only max 255 supported!";
      throw ossimException(xmsg.str());
   }
   clear();

   // Define default replacement pixel values (unless already done):
   m_numClusters = K;
   if (m_pixelValues.size() != m_numClusters)
   {
      m_pixelValues.clear();
      for (ossim_uint32 i=1; i<=m_numClusters; ++i)
         m_pixelValues.push_back(i);
   }
}

void ossimKMeansFilter::setClusterPixelValues(const ossim_uint32* dns, ossim_uint32 K)
{
   if (dns == 0)
      return;

   if (K != m_numClusters)
      setNumClusters(K);

   m_pixelValues.clear();
   for (ossim_uint32 i=0; i<m_numClusters; ++i)
      m_pixelValues.push_back(dns[i]);
}

const ossimKMeansClustering* ossimKMeansFilter::getBandClassifier(ossim_uint32 band) const
{
   if (band < m_classifiers.size())
      return m_classifiers[band].get();
   return 0;
}

bool ossimKMeansFilter::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   if (m_numClusters == 0)
      return true;

   // TODO: Need to implement

   bool rtn_stat = ossimImageSourceFilter::saveState(kwl, prefix);
   return rtn_stat;
}

bool ossimKMeansFilter::loadState(const ossimKeywordlist& orig_kwl, const char* prefix)
{
   bool return_state = true;
   //ossimKeywordlist kwl (orig_kwl); // need non-const copy

   // TODO: Need to implement

   return_state &= ossimImageSourceFilter::loadState(orig_kwl, prefix);

   return return_state;
}

ossimScalarType ossimKMeansFilter::getOutputScalarType() const
{
   ossimScalarType myType = OSSIM_SCALAR_UNKNOWN;

   if (m_numClusters > 255)
      myType = OSSIM_UINT16; // Can't have more than 65535 groups! NOT YET SUPPORTED
   else if (m_numClusters > 0)
         myType = OSSIM_UINT8;

   return myType;
}

double ossimKMeansFilter::getMinPixelValue(ossim_uint32 band)const
{
   if (band < m_minPixelValue.size())
      return m_minPixelValue[band];
   return 1;
}

double ossimKMeansFilter::getMaxPixelValue(ossim_uint32 band)const
{
   if (band < m_maxPixelValue.size())
      return m_maxPixelValue[band];
   return 255.0;
}


