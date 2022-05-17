//---
// File: ossimNitfRsmModel.cpp
//
//  RP 
//  LIMITATIONS - This is supporting only the RSM features that have been
//  observed in current data samples and does not attempt to support the entire
//  RSM specification.
//  Examples of currently unsupported items include ->
//  1.  multiple RSMPC tags for different polynomials for separate image sections 
//  2.  Error Covariance (this may gain priority as we have access to RSMECA data)
//  3.  Illumination model
//  4.  Rectangular coodinate system conversion (RSDIDA GRNDD = "R")
//---

#include <ossim/projection/ossimNitfRsmModel.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/support_data/ossimNitfRsmecaTag.h>
#include <ossim/support_data/ossimNitfRsmecbTag.h>
#include <ossim/support_data/ossimNitfRsmidaTag.h>
#include <ossim/support_data/ossimNitfRsmpcaTag.h>
#include <ossim/support_data/ossimNitfRsmpiaTag.h>


RTTI_DEF1(ossimNitfRsmModel, "ossimNitfRsmModel", ossimRsmModel);

//---
// Define Trace flags for use within this file:
//---

static ossimTrace traceExec  ("ossimNitfRsmModel:exec");
static ossimTrace traceDebug ("ossimNitfRsmModel:debug");



ossimNitfRsmModel::ossimNitfRsmModel()
   :  ossimRsmModel()
{
}

ossimNitfRsmModel::ossimNitfRsmModel( const ossimNitfRsmModel& obj )
   : ossimRsmModel(obj)
{
}

const ossimNitfRsmModel& ossimNitfRsmModel::operator=( const ossimNitfRsmModel& rhs )
{
   if (this != &rhs)
   {
      ossimRsmModel::operator=(rhs);
   }
   return *this;
}

ossimNitfRsmModel::~ossimNitfRsmModel()
{
}

bool ossimNitfRsmModel::parseFile( const ossimFilename& nitfFile,
                                   ossim_uint32 entryIndex )
{
   bool status = false;

   if ( nitfFile.exists() )
   {
      ossimRefPtr<ossimNitfFile> file = new ossimNitfFile;   
      if( file->parseFile(nitfFile) )
      {
         ossimRefPtr<ossimNitfImageHeader> ih = file->getNewImageHeader(entryIndex);
         if ( ih.valid() )
         {
            status = parseImageHeader( ih.get() );
         }
      }
   }

   if( !status )
   {
      setErrorStatus();
   }
   
   return status;
}

bool ossimNitfRsmModel::parseImageHeader(const ossimNitfImageHeader* ih)
{
   static const char MODULE[] = "ossimNitfRsmModel::getRsmData";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entering..." << std::endl;
   }

   bool status = false;

   if ( getRsmData(ih) )
   {
      
      theImageID = m_pca[0].m_iid; // tmp drb...
      theImageID = theImageID.trim();
      
      ossimIrect imageRect = ih->getImageRect();
      
      // Fetch Image Size:
      theImageSize.line = static_cast<ossim_int32>(imageRect.height());
      theImageSize.samp = static_cast<ossim_int32>(imageRect.width());
      
      // Assign other data members:
      theRefImgPt.line = theImageSize.line/2.0;
      theRefImgPt.samp = theImageSize.samp/2.0;
      
      // Assign the bounding image space rectangle:
      theImageClipRect = ossimDrect(0.0, 0.0, theImageSize.samp-1, theImageSize.line-1);
      
      ossimGpt v0, v1, v2, v3;
      ossimDpt ip0 (0.0, 0.0);
      lineSampleHeightToWorld(ip0, m_pca[0].m_znrmo, v0);
      ossimDpt ip1 (theImageSize.samp-1.0, 0.0);
      lineSampleHeightToWorld(ip1, m_pca[0].m_znrmo, v1);
      ossimDpt ip2 (theImageSize.samp-1.0, theImageSize.line-1.0);
      lineSampleHeightToWorld(ip2, m_pca[0].m_znrmo, v2);
      ossimDpt ip3 (0.0, theImageSize.line-1.0);
      lineSampleHeightToWorld(ip3, m_pca[0].m_znrmo, v3);
      
      theBoundGndPolygon = ossimPolygon (ossimDpt(v0), ossimDpt(v1), ossimDpt(v2), ossimDpt(v3));
      
      updateModel();
      
      // Set the ground reference point.
      lineSampleHeightToWorld(theRefImgPt, m_pca[0].m_znrmo, theRefGndPt);

      // Height could have nan if elevation is not set so check lat, lon individually.
      if ( ( theRefGndPt.isLatNan() == false ) && ( theRefGndPt.isLonNan() == false ) )
      {
         //---
         // This will set theGSD and theMeanGSD.  This model doesn't need these but
         // others do.
         //---
         try
         {
            computeGsd();

            // Set return status.
            status = true;
         }
         catch (const ossimException& e)
         {
            if (traceDebug())
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ossimNitfRpcModel::ossimNitfRpcModel DEBUG:\n"
                  << e.what() << std::endl;
            }
            setErrorStatus();
         }
      }
      else
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "ossimNitfRpcModel::ossimNitfRpcModel DEBUG:"
               << "\nGround Reference Point not valid(has nans)."
               << " Aborting with error..."
               << std::endl;
         }
         setErrorStatus();
      }
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimNitfRpcModel::parseFile DEBUG:"
            << "\nError parsing rsm tags.  Aborting with error."
            << std::endl;
      }
      setErrorStatus();
   }

   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status: " << ( status ? "true" : "false" ) << "\n";
   }

   return status;
   
} // End: ossimNitfRsmModel::parseImageHeader(const ossimNitfImageHeader* ih)

bool ossimNitfRsmModel::getRsmData(const ossimNitfImageHeader* ih)
{
   static const char MODULE[] = "ossimNitfRsmModel::getRsmData";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entering..." << std::endl;
   }
   
   bool status = false;
   
   if ( ih )
   {
      ossimRefPtr<ossimNitfRegisteredTag> tag = 0;

      // Find either RSMECB or RSMECA:
      bool error_cov_initialized = false;
      const ossimString RSMECB_TAG = "RSMECB";
      const ossimString RSMECA_TAG = "RSMECA";
      tag = ih->getTagData(RSMECB_TAG);
      if (tag.valid())
      {
         ossimRefPtr<ossimNitfRsmecbTag> rsmecbTag =
            dynamic_cast<ossimNitfRsmecbTag*>(tag.get());
         if ( rsmecbTag.valid() )
         {
            if ( initializeModel( rsmecbTag.get() ) )
               error_cov_initialized = true;
         }
      } else {
         tag = ih->getTagData(RSMECA_TAG);
         if (tag.valid())
         {
            ossimRefPtr<ossimNitfRsmecaTag> rsmecaTag =
               dynamic_cast<ossimNitfRsmecaTag*>(tag.get());
            if ( rsmecaTag.valid() )
            {
               if ( initializeModel( rsmecaTag.get() ) )
                  error_cov_initialized = true;
            }
         }
      }

      if (error_cov_initialized)
      {
         // RSMIDA:
         ossimString RSMIDA_TAG = "RSMIDA";
         tag = ih->getTagData(RSMIDA_TAG);
         if (tag.valid())
         {
            ossimRefPtr<ossimNitfRsmidaTag> rsmidaTag =
               dynamic_cast<ossimNitfRsmidaTag*>( tag.get() );
            if ( rsmidaTag.valid() )
            {
               if ( m_ida.initialize( rsmidaTag.get() ) )
               {
                  // RSMPIA:
                  const ossimString RSMPIA_TAG = "RSMPIA";
                  tag = ih->getTagData(RSMPIA_TAG);
                  if (tag.valid())
                  {
                     ossimRefPtr<ossimNitfRsmpiaTag> rsmpiaTag =
                        dynamic_cast<ossimNitfRsmpiaTag*>( tag.get() );
                     if ( rsmpiaTag.valid() )
                     {
                        if ( m_pia.initialize( rsmpiaTag.get() ) )
                        {
                           //---
                           // RSMPCA:
                           // Multiple tags if image is sectioned.
                           //---
                           const ossimString RSMPCA_TAG = "RSMPCA";
                           std::vector< const ossimNitfRegisteredTag* > tags;
                           ih->getTagData( RSMPCA_TAG, tags );

                           if ( tags.size() == m_pia.m_tnis )
                           {
                              for ( ossim_uint32 tagIndex = 0;
                                    tagIndex < m_pia.m_tnis;
                                    ++tagIndex )
                              {
                                 const ossimNitfRsmpcaTag* rsmpcaTag =
                                    dynamic_cast<const ossimNitfRsmpcaTag*>( tags[tagIndex] );
                                 if ( rsmpcaTag )
                                 {
                                    ossimRsmpca pca;
                                    if ( pca.initialize( rsmpcaTag ) )
                                    {
                                       m_pca.push_back( pca );
                                    }
                                    else if (traceDebug())
                                    {
                                       ossimNotify(ossimNotifyLevel_WARN)
                                          << "WARNING! RSMPCA[" << tagIndex << "] intitialization failed!"
                                          << std::endl;
                                    }
                                 }
                              }

                              // Call base ossimRsmModel::validate() for sanity check:
                              status = validate();
                           }

                        } // Matches: if ( m_pia.initialize( rsmpiaTag ) )
                     }
                  }
                  else if (traceDebug())
                  {
                     ossimNotify(ossimNotifyLevel_WARN)
                        << "\nCould not find RSM tag: " << RSMPIA_TAG
                        << "\nAborting with error..."
                        << std::endl;
                  }
               }
            }
         }
         else if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "\nCould not find RSM tag: " << RSMIDA_TAG
               << "\nAborting with error..." << std::endl;
         }
      }
      else if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmModel::getRsmData WARNING!"
            << "\nCould not find RSM tag: " << RSMECA_TAG << " or " << RSMECB_TAG
            << "\nAborting with error..." << std::endl;
      }
   }

   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status: " << ( status ? "true" : "false" ) << "\n";
   }

   return status;
   
} // End: ossimNitfRsmModel::getRsmData(const ossimNitfImageHeader* ih)


bool ossimNitfRsmModel::initializeModel( const ossimNitfRsmecaTag* rsmecaTag )
{
   bool status = false;

   if ( rsmecaTag )
   {
      // TODO:
      status = true;
   }

   return status;
   
} // End: ossimNitfRsmModel::initializeModel( rsmecaTag )


bool ossimNitfRsmModel::initializeModel( const ossimNitfRsmecbTag* rsmecbTag )
{
   bool status = false;

   if ( rsmecbTag )
   {
      // TODO:
      status = true;
   }

   return status;

} // End: ossimNitfRsmModel::initializeModel( rsmecbTag )


ossimObject* ossimNitfRsmModel::dup() const
{
   return new ossimNitfRsmModel(*this);
}

bool ossimNitfRsmModel::saveState(ossimKeywordlist& kwl,
                                  const char* prefix) const
{
   
   static const char MODULE[] = "ossimNitfRsmModel::saveState";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   //---
   // Hand off to base class for common stuff:
   //
   // Note: Class type "ossimKeywordNames::TYPE_KW" is saved in
   // ossimOject::saveState.
   //---
   bool status = ossimRsmModel::saveState(kwl, prefix);

   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " exited...\n";
   }

   return status;
}

bool ossimNitfRsmModel::loadState( const ossimKeywordlist& kwl,
                                   const char* prefix ) 
{
   static const char MODULE[] = "ossimNitfRsmModel::loadState";
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }

   bool status = false;

   // Check for type match before preceeding:
   std::string myPrefix = ( prefix ? prefix : "" );
   std::string type = kwl.findKey( myPrefix, std::string(ossimKeywordNames::TYPE_KW) );

   if ( type == "ossimNitfRsmModel" )
   {
      // Pass on to the base-class for parsing first:
      status = ossimRsmModel::loadState(kwl, prefix);
   }
   
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (status?"true":"false") << "\n";
   }
   
   return status;
}
