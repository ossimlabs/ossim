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
   ossimRefPtr<ossimNitfFile> file = new ossimNitfFile;
   
   if(!file->parseFile(nitfFile))
   {
      setErrorStatus();
      return false;
   }
   
   ossimRefPtr<ossimNitfImageHeader> ih = file->getNewImageHeader(entryIndex);
   if(!ih)
   {
      setErrorStatus();
      return false;
   }
   
   return parseImageHeader(ih.get());

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
      theImageID = m_iid.trim();
      
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
      lineSampleHeightToWorld(ip0, m_znrmo, v0);
      ossimDpt ip1 (theImageSize.samp-1.0, 0.0);
      lineSampleHeightToWorld(ip1, m_znrmo, v1);
      ossimDpt ip2 (theImageSize.samp-1.0, theImageSize.line-1.0);
      lineSampleHeightToWorld(ip2, m_znrmo, v2);
      ossimDpt ip3 (0.0, theImageSize.line-1.0);
      lineSampleHeightToWorld(ip3, m_znrmo, v3);
      
      theBoundGndPolygon = ossimPolygon (ossimDpt(v0), ossimDpt(v1), ossimDpt(v2), ossimDpt(v3));
      
      updateModel();
      
      // Set the ground reference point.
      lineSampleHeightToWorld(theRefImgPt, m_znrmo, theRefGndPt);

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

      // RSMECA:
      const ossimString RSMECA_TAG = "RSMECA";
      tag = ih->getTagData(RSMECA_TAG);
      if (tag.valid())
      {
         ossimRefPtr<ossimNitfRsmecaTag> rsmecaTag =
            dynamic_cast<ossimNitfRsmecaTag*>(tag.get());
         if ( rsmecaTag.valid() )
         {
            if ( initializeModel( rsmecaTag.get() ) )
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
                     if ( initializeModel( rsmidaTag.get() ) )
                     {
                        // RSMPCA:
                        const ossimString RSMPCA_TAG = "RSMPCA";                        
                        tag = ih->getTagData(RSMPCA_TAG);
                        if (tag.valid())
                        {
                           ossimRefPtr<ossimNitfRsmpcaTag> rsmpcaTag =
                              dynamic_cast<ossimNitfRsmpcaTag*>( tag.get() );
                           if ( rsmpcaTag.valid() )
                           {
                              if ( initializeModel( rsmpcaTag.get() ) )
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
                                       // Last tag, assign status.
                                       status = initializeModel( rsmpiaTag.get() );
                                    }
                                 }
                                 else if (traceDebug())
                                 {
                                    ossimNotify(ossimNotifyLevel_WARN)
                                       << "ossimNitfRsmModel::getRsmData WARNING!"
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
                              << "ossimNitfRsmModel::getRsmData WARNING!"
                              << "\nCould not find RSM tag: " << RSMPCA_TAG
                              << "\nAborting with error..."
                              << std::endl;
                        }
                     }
                  }
               }
               else if (traceDebug())
               {
                  ossimNotify(ossimNotifyLevel_WARN)
                     << "ossimNitfRsmModel::getRsmData WARNING!"
                     << "\nCould not find RSM tag: " << RSMIDA_TAG
                     << "\nAborting with error..." << std::endl;
               }
            }
         }
      }
      else if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossimNitfRsmModel::getRsmData WARNING!"
            << "\nCould not find RSM tag: " << RSMECA_TAG
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

bool ossimNitfRsmModel::initializeModel( const ossimNitfRsmidaTag* rsmidaTag )
{
   bool status = false;

   if ( rsmidaTag )
   {
      // TODO:
      status = true;
   }

   return status;
   
} // End: ossimNitfRsmModel::initializeModel( rsmidaTag )

bool ossimNitfRsmModel::initializeModel( const ossimNitfRsmpcaTag* rsmpcaTag )
{
   bool status = false;

   if ( rsmpcaTag )
   {
      m_iid = rsmpcaTag->getIid();
      m_edition = rsmpcaTag->getEdition();
      m_rsn = rsmpcaTag->getRsn().toUInt32();
      m_csn = rsmpcaTag->getCsn().toUInt32();
      
      // Supporting only the single polynomial set case right now, so fail otherwise
      if (m_rsn == 1 && m_csn == 1)
      {
         m_rfep = rsmpcaTag->getRfep().toFloat64();
         m_cfep = rsmpcaTag->getCfep().toFloat64();
         m_rnrmo = rsmpcaTag->getRnrmo().toFloat64();
         m_cnrmo = rsmpcaTag->getCnrmo().toFloat64();
         m_xnrmo = rsmpcaTag->getXnrmo().toFloat64();
         m_ynrmo = rsmpcaTag->getYnrmo().toFloat64();
         m_znrmo = rsmpcaTag->getZnrmo().toFloat64();
         m_rnrmsf = rsmpcaTag->getRnrmsf().toFloat64();
         m_cnrmsf = rsmpcaTag->getCnrmsf().toFloat64();
         m_xnrmsf = rsmpcaTag->getXnrmsf().toFloat64();
         m_ynrmsf = rsmpcaTag->getYnrmsf().toFloat64();
         m_znrmsf = rsmpcaTag->getZnrmsf().toFloat64();
         
         m_rnpwrx = rsmpcaTag->getRnpwrx().toUInt32();
         m_rnpwry = rsmpcaTag->getRnpwry().toUInt32();
         m_rnpwrz = rsmpcaTag->getRnpwrz().toUInt32();
         m_rntrms = rsmpcaTag->getRntrms().toUInt32();
         m_rnpcf.resize(m_rntrms);
         for (ossim_uint32 i = 0; i < m_rntrms; ++i)
         {
            m_rnpcf[i] = rsmpcaTag->getRnpcf(i).toFloat64();
         }
         
         m_rdpwrx = rsmpcaTag->getRdpwrx().toUInt32();
         m_rdpwry = rsmpcaTag->getRdpwry().toUInt32();
         m_rdpwrz = rsmpcaTag->getRdpwrz().toUInt32();
         m_rdtrms = rsmpcaTag->getRdtrms().toUInt32();
         m_rdpcf.resize(m_rdtrms);
         for (ossim_uint32 i = 0; i < m_rdtrms; ++i)
         {
            m_rdpcf[i] = rsmpcaTag->getRdpcf(i).toFloat64();
         }
         
         m_cnpwrx = rsmpcaTag->getCnpwrx().toUInt32();
         m_cnpwry = rsmpcaTag->getCnpwry().toUInt32();
         m_cnpwrz = rsmpcaTag->getCnpwrz().toUInt32();
         m_cntrms = rsmpcaTag->getCntrms().toUInt32();
         m_cnpcf.resize(m_cntrms);
         for (ossim_uint32 i = 0; i < m_cntrms; ++i)
         {
            m_cnpcf[i] = rsmpcaTag->getCnpcf(i).toFloat64();
         }
         
         m_cdpwrx = rsmpcaTag->getCdpwrx().toUInt32();
         m_cdpwry = rsmpcaTag->getCdpwry().toUInt32();
         m_cdpwrz = rsmpcaTag->getCdpwrz().toUInt32();
         m_cdtrms = rsmpcaTag->getCdtrms().toUInt32();
         m_cdpcf.resize(m_cdtrms);
         for (ossim_uint32 i = 0; i < m_cdtrms; ++i)
         {
            m_cdpcf[i] = rsmpcaTag->getCdpcf(i).toFloat64();
         }
         
         status = true;
      }
      else if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimNitfRsmModel::getRsmData DEBUG:"
            << "\nRow or Column Section Number not equal to 1: CSN is "
            << ossimString::toString(m_csn) << ", RSN is " << ossimString::toString(m_rsn)
            << "\nAborting with error..."
            << std::endl;
      }
   }
   
   return status;
   
} // End: ossimNitfRsmModel::initializeModel( rsmpcaTag )

bool ossimNitfRsmModel::initializeModel( const ossimNitfRsmpiaTag* rsmpiaTag )
{
   bool status = false;

   if ( rsmpiaTag )
   {
      // TODO:
      status = true;
   }

   return status;
   
} // End: ossimNitfRsmModel::initializeModel( rsmpiaTag )

ossimObject* ossimNitfRsmModel::dup() const
{
   return new ossimNitfRsmModel(*this);
}

//---**************************************************************************
//  METHOD: ossimNitfRsmModel::saveState()
//  
//  Saves the model state to the KWL. This KWL also serves as a geometry file.
//  
//---**************************************************************************
bool ossimNitfRsmModel::saveState(ossimKeywordlist& kwl,
                                  const char* prefix) const
{
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimNitfRsmModel::saveState(): entering..." << std::endl;
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
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimNitfRsmModel::saveState(): returning..." << std::endl;
   }

   return status;
}

bool ossimNitfRsmModel::loadState( const ossimKeywordlist& kwl,
                                   const char* prefix ) 
{
   if (traceExec())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimNitfRsmModel::loadState(): entering..." << std::endl;
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
         << "DEBUG ossimNitfRsmModel::loadState(): exit status: "
         << ( status ? "true" : "false" ) << std::endl;
   }
   
   return status;
}
