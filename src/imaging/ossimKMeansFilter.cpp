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
    m_numGroups(0),
    m_numBands(0),
    m_tile(0),
    m_classGroups(0),
    m_outputScalarType(OSSIM_SCALAR_UNKNOWN),
    m_initLevel(0),
    m_verbose(false)
{
   setDescription("K-Means pixel classification filter.");
}

ossimKMeansFilter::ossimKMeansFilter(ossimImageSource* input_source,
                                     ossimMultiBandHistogram* histogram)
:   ossimImageSourceFilter(input_source),
    m_histogram (histogram),
    m_numGroups(0),
    m_numBands(0),
    m_tile(0),
    m_classGroups(0),
    m_outputScalarType(OSSIM_SCALAR_UNKNOWN),
    m_initLevel(0),
    m_verbose(false)
{
   setDescription("K-Means pixel classification filter.");
}

ossimKMeansFilter::~ossimKMeansFilter()
{
   if (m_classGroups)
   {
      for (ossim_uint32 i=0; i<m_numBands; ++i)
         delete [] m_classGroups[i];
   }
}

void ossimKMeansFilter::setInputHistogram(ossimMultiBandHistogram* histo)
{
   m_histogram = histo;
   m_initLevel = 0;
}

ossimRefPtr<ossimImageData> ossimKMeansFilter::getTile(const ossimIrect& tileRect,
                                                       ossim_uint32 resLevel)
{
   if(!theInputConnection || (m_numGroups == 0))
      return 0;

   if (m_initLevel != 2)
   {
      if (!computeKMeans())
         return 0;
   }

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

   ossim_uint8* outBuf; // TODO: Only K < 256 is currently supported
   double null_pixel = theInputConnection->getNullPixelValue();
   double pixel;
   ossimIpt ipt;
   ossim_uint32 offset = 0;
   for (ossim_uint32 band=0; band<m_numBands; ++band)
   {
      outBuf = (ossim_uint8*)(m_tile->getBuf(band));

      ClassificationGroup* group = m_classGroups[band]; // for shorthand referencing of groups
      for (ipt.y=tileRect.ul().y; ipt.y<=tileRect.lr().y; ++ipt.y)
      {
         for (ipt.x=tileRect.ul().x; ipt.x<=tileRect.lr().x; ++ipt.x)
         {
            pixel = inTile->getPix(ipt, band);
            if (pixel != null_pixel)
            {
               // Determine its group and remap it using the group's DN:
               for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
               {
                  if ((pixel >= group[gid].min) && (pixel <= group[gid].max))
                  {
                     outBuf[offset] = (ossim_uint8) group[gid].dn;
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
   if(!theInputConnection)
      return;

   m_tile = ossimImageDataFactory::instance()->create(this, getNumberOfInputBands(), this);
   if(!m_tile.valid())
      return;

   if (m_numGroups && m_classGroups)
   {
      for (ossim_uint32 band=0; band<m_numBands; band++)
      {
         double min = m_classGroups[band][0].min;
         double max = m_classGroups[band][0].max;

         for (ossim_uint32 group=1; group<m_numGroups; group++)
         {
            if (m_classGroups[band][group].min < min)
               min = m_classGroups[band][group].min;
            if (m_classGroups[band][group].max > max)
               max = m_classGroups[band][group].max;
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
   ossimImageSourceFilter::initialize();
   m_tile = 0;

   if ( !theInputConnection )
      return;

   // If an input histogram was provided, use it. Otherwise compute one:
   m_numBands = getNumberOfInputBands();
   if (!m_histogram.valid())
   {
      ossimRefPtr<ossimImageHistogramSource> histoSource = new ossimImageHistogramSource;
      histoSource->connectMyInputTo(theInputConnection);
      histoSource->setComputationMode(OSSIM_HISTO_MODE_FAST);
      histoSource->setMaxNumberOfRLevels(1);
      histoSource->execute();
      m_histogram = histoSource->getHistogram()->getMultiBandHistogram(0);
   }

   if (m_histogram.valid())
   {
      for (ossim_uint32 band=0; band<m_numBands; band++)
      {
         ossimRefPtr<ossimHistogram> h = m_histogram->getHistogram(band);
         m_minPixelValue.push_back(h->GetMinVal());
         m_maxPixelValue.push_back(h->GetMaxVal());
      }
   }

   m_initLevel = 1;
}

bool ossimKMeansFilter::computeKMeans()
{
   ostringstream xmsg;
   if (m_numGroups == 0)
   {
      xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Number of groups has not been initialized!";
      throw ossimException(xmsg.str());
   }

   if (m_initLevel < 1)
      initialize();

   bool kosher = true;
   if (m_histogram.valid())
   {
      cout<<"\nUsing histogram to comput K-means..."<<endl;

      // Use the fast method:
      for (ossim_uint32 band=0; (band<m_numBands) && kosher; band++)
      {
         ossimRefPtr<ossimHistogram> band_histo = m_histogram->getHistogram(band);
         if (!band_histo.valid())
         {
            xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Null band histogram returned!";
            throw ossimException(xmsg.str());
         }
         kosher = computeKMeans(band_histo.get(), band);
      }
   }
   else
   {
      // Need to sequence through the input data to compute groups:
      ossimRefPtr<ossimImageSourceSequencer> sequencer = new ossimImageSourceSequencer;
      sequencer->connectMyInputTo(theInputConnection);

      // Allocate the output tile:
      m_tile = ossimImageDataFactory::instance()->create(this, this);
      if(m_tile.valid())
         m_tile->initialize();
      else
         return false;

      // Run the classifier for each band:
      for (ossim_uint32 band=0; (band<m_numBands) && kosher; band++)
         kosher = computeKMeans(sequencer.get(), band);
   }

   if (m_verbose)
   {
      if (kosher)
      {
         for (ossim_uint32 band=0; band<m_numBands; band++)
         {
            ClassificationGroup* group = m_classGroups[band];
            if ((m_numBands > 1))
               cout<<"For band "<< band << "," << endl;

            for (ossim_uint32 gid=0; gid<m_numGroups; gid++)
            {
               cout<<"group["<<gid<<"] n        = "<<group[gid].n<<endl;
               cout<<"         mean     = "<<group[gid].mean<<endl;
               cout<<"         min      = "<<group[gid].min<<endl;
               cout<<"         max      = "<<group[gid].max<<endl;
               cout<<"         dn       = "<<group[gid].dn<<endl;
               cout << endl;
            }
         }
      }
      else
      {
         cout<<"ossimKMeansFilter:"<<__LINE__<<" No K-means clustering data available."<<endl;
      }
   }

   if (kosher)
      m_initLevel = 2;
   return kosher;
}

bool ossimKMeansFilter::computeKMeans(ossimImageSourceSequencer* sequencer, ossim_uint32 band)
{
   if (m_numGroups == 0)
      return false;

   // Initialize groups with even spread and initial means:
   ClassificationGroup* group = m_classGroups[band]; // for shorthand referencing of groups
   double overall_min = m_minPixelValue[band];
   double overall_max = m_maxPixelValue[band];
   double interval = (overall_max - overall_min) / m_numGroups;
   double mean_i = overall_min + interval / 2.0;  // initial mean for group 0;
   double bound = overall_min;
   for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
   {
      group[gid].min = bound;
      bound += interval;
      group[gid].max = bound;
      group[gid].mean = mean_i;
      group[gid].new_mean = 0;
      mean_i += interval;
      group[gid].n = 0;
   }

   ossimIrect tileRect;
   ossimRefPtr<ossimImageData> tile;
   double pixel, delta, min_delta;
   ossimIpt ipt;
   ossim_uint32 best_gid, n, iters = 0, max_iters = 20;
   bool converged = false;
   double null_pixel = theInputConnection->getNullPixelValue();

   // Loop until converged on best solution:
   while (!converged && (iters < max_iters))
   {
      converged = true; // prove otherwise
      ++iters;

      // reset sequencer to start from first tile:
      sequencer->setToStartOfSequence();
      tile = sequencer->getNextTile();

      // Loop over all tiles in the AOI:
      while (tile.valid())
      {
         tileRect = tile->getImageRectangle();
         for (ipt.y=tileRect.ul().y; ipt.y<tileRect.lr().y; ++ipt.y)
         {
            for (ipt.x=tileRect.ul().x; ipt.x<tileRect.lr().x; ++ipt.x)
            {
               pixel = tile->getPix(ipt, band);
               if (pixel == null_pixel)
                  continue;

               // Find the current group:
               best_gid = 0;
               min_delta = overall_max - overall_min;
               for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
               {
                  delta = fabs(pixel - group[gid].mean);
                  if (delta < min_delta)
                  {
                     min_delta = delta;
                     best_gid = gid;
                  }
               }

               // Determined best group given current means. Update the running mean for this group
               // to reflect sample:
               n = group[best_gid].n;
               if (n == 0)
                  group[best_gid].new_mean = pixel;
               else
                  group[best_gid].new_mean = (group[best_gid].new_mean*n + pixel)/(double(n+1));
               group[best_gid].n++;

            } // End loop over X in tile
         } // End loop over Y in tile

         tile = sequencer->getNextTile();
      }

      // Finished processing all input pixels for this iteration. Update the means:
      for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
      {
         if (fabs(group[gid].mean - group[gid].new_mean) > 0.1 )
            converged = false;

         if (m_verbose)
         {
            cout<<"group["<<gid<<"].mean     = "<<group[gid].mean<<endl;
            cout<<"group["<<gid<<"].new_mean = "<<group[gid].new_mean<<endl;
            cout<<"group["<<gid<<"].n        = "<<group[gid].n<<endl;
         }
         if (group[gid].n)
            group[gid].mean = group[gid].new_mean;
         group[gid].n = 0;
         group[gid].new_mean = 0;
      }

   } // End overall loop for convergence:

   return true;
}

bool ossimKMeansFilter::computeKMeans(ossimHistogram* histogram, ossim_uint32 band)
{
   int numBins = histogram->GetRes();
   const float* val = histogram->GetVals();
   const float* pop = histogram->GetCounts();
   const double half_bucket = 0.5 * histogram->GetBucketSize();

   //Initialize groups with even spread and initial means:
   ClassificationGroup* group = m_classGroups[band]; // for shorthand referencing of groups
   double overall_min = m_minPixelValue[band];
   double overall_max = m_maxPixelValue[band];
   double interval = (overall_max - overall_min) / m_numGroups;
   double mean_i = overall_min + interval / 2.0;  // initial mean for group 0;
   double bound = overall_min;
   double null_pixel = theInputConnection->getNullPixelValue();

   for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
   {
      group[gid].min = bound;
      bound += interval;
      group[gid].max = bound;
      group[gid].mean = mean_i;
      group[gid].new_mean = 0;
      mean_i += interval;
      group[gid].n = 0;
   }

   double delta, min_delta;
   ossimIpt ipt;
   ossim_uint32 best_gid, n, np, iters = 0, max_iters = 20;
   bool converged = false;

   // Loop until converged on best solution:
   while (!converged && (iters < max_iters))
   {
      converged = true; // prove otherwise
      ++iters;

      for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
      {
         group[gid].min = overall_max;
         group[gid].max = overall_min;
      }

      for (ossim_uint32 i=0; i<numBins; i++)
      {
         if ((pop[i] == 0) || (val[i] == null_pixel))
            continue;

         // Find the current group:
         best_gid = 0;
         min_delta = overall_max - overall_min;
         for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
         {
            delta = fabs(val[i] - group[gid].mean);
            if (delta < min_delta)
            {
               min_delta = delta;
               best_gid = gid;
            }
         }

         // Possible update of min/max for current group:
         if (val[i] < group[best_gid].min)
            group[best_gid].min = val[i] - half_bucket;
         else if (val[i] > group[best_gid].max)
            group[best_gid].max = val[i] + half_bucket;

         // Accumulate sample for the new mean for this group:
         group[best_gid].n += pop[i];
         group[best_gid].new_mean += pop[i]*val[i];

      } // End loop over bins

      // Finished processing all input pixels for this iteration. Update the means:
      for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
      {
         // Compute new mean from accumulation:
         if (group[gid].n)
            group[gid].new_mean /= group[gid].n;

         if (fabs(group[gid].mean - group[gid].new_mean) > 0.01 )
            converged = false;

         if (m_verbose)
         {
            cout<<"group["<<gid<<"].mean     = "<<group[gid].mean<<endl;
            cout<<"group["<<gid<<"].new_mean = "<<group[gid].new_mean<<endl;
            cout<<"group["<<gid<<"].n        = "<<group[gid].n<<endl;
            cout<<"group["<<gid<<"].min      = "<<group[gid].min<<endl;
            cout<<"group["<<gid<<"].max      = "<<group[gid].max<<endl;
            cout << endl;
         }
         if (group[gid].n)
            group[gid].mean = group[gid].new_mean;
         group[gid].n = 0;
         group[gid].new_mean = 0;
      }
   } // End overall loop for convergence:

   return true;
}

void ossimKMeansFilter::clear()
{
   if ((m_numGroups == 0) || (m_classGroups == 0))
      return;

   for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
      delete [] m_classGroups[gid];

   delete [] m_classGroups;
   m_classGroups = 0;
   m_numGroups = 0;
   m_initLevel = 0;
}

void ossimKMeansFilter::setNumGroups(ossim_uint32 K)
{
   if (K > 255)
   {
      ostringstream xmsg;
      xmsg << "ossimKMeansFilter:"<<__LINE__<<" Requested K="<<K<<" but only max 255 supported!";
      throw ossimException(xmsg.str());
   }
   clear();

   if (m_initLevel < 1)
      initialize();

   // Allocate the group array and run classifier on each band:
   m_numGroups = K;
   if (m_numBands == 0)
      return;

   m_classGroups = new ClassificationGroup* [m_numBands];
   bool kosher = true;
   for (ossim_uint32 band=0; (band<m_numBands) && kosher; band++)
   {
      m_classGroups[band] = new ClassificationGroup [m_numGroups];
      for (ossim_uint32 gid=0; gid<m_numGroups; gid++)
         m_classGroups[band][gid].dn = gid + 1; // Initialize to default remap values
   }

   m_initLevel = 1;
}

void ossimKMeansFilter::setGroupPixelValues(const ossim_uint32* dns, ossim_uint32 K)
{
   if (K != m_numGroups)
      setNumGroups(K);

   if ((m_numBands == 0) || (dns == 0))
      return;

   for (ossim_uint32 band=0; band<m_numBands; band++)
   {
      for (ossim_uint32 gid=0; gid<m_numGroups; ++gid)
         m_classGroups[band][gid].dn = dns[gid];
   }
}

void ossimKMeansFilter::getBandClassGroups(ossimKMeansFilter::ClassificationGroup* groups,
                                           ossim_uint32& K, ossim_uint32 band)
{
   if ((band < m_numBands) && m_classGroups)
   {
      groups = m_classGroups[band];
      K = m_numGroups;
   }
   else
   {
      groups = 0;
      K = 0;
   }
}

bool ossimKMeansFilter::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   if (m_numGroups == 0)
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
   if (m_numGroups > 0)
   {
      if (m_numGroups <=255)
         myType = OSSIM_UINT8;
      else
         myType = OSSIM_UINT16; // Can't have more than 65535 groups! NOT YET SUPPORTED
   }
   return myType;
}

double ossimKMeansFilter::getNullPixelValue(ossim_uint32 /* band */ )const
{
   return 0.0;
}

double ossimKMeansFilter::getMinPixelValue(ossim_uint32 /* band */)const
{
   return 1.0;
}

double ossimKMeansFilter::getMaxPixelValue(ossim_uint32 /* band */)const
{
   return 255.0;
}


