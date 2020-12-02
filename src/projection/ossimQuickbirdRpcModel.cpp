//*****************************************************************************
// FILE: ossimQuickbirdRpcModel.cpp
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION: Contains declaration of class ossimQuickbirdRpcModel. This
//    derived class implements the capability of reading Quickbird RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: ossimQuickbirdRpcModel.cpp 23564 2015-10-02 14:12:25Z dburken $

#include <ossim/projection/ossimQuickbirdRpcModel.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossim2dTo2dShiftTransform.h>
#include <ossim/support_data/ossimQuickbirdRpcHeader.h>
#include <ossim/support_data/ossimQuickbirdTile.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfRpcBase.h>
#include <ossim/support_data/ossimNitfUse00aTag.h>
#include <ossim/support_data/ossimNitfIchipbTag.h>
#include <ossim/support_data/ossimNitfPiaimcTag.h>
#include <ossim/imaging/ossimTiffTileSource.h>
#include <ossim/imaging/ossimQbTileFilesHandler.h>

static const char *RPC00A_TAG = "RPC00A";
static const char *RPC00B_TAG = "RPC00B";
// static const char *ICHIPB_TAG = "ICHIPB";
static const char *PIAIMC_TAG = "PIAIMC";
static const char *USE00A_TAG = "USE00A";

RTTI_DEF1(ossimQuickbirdRpcModel, "ossimQuickbirdRpcModel", ossimRpcModel);

//*************************************************************************************************
// Constructor
//*************************************************************************************************
ossimQuickbirdRpcModel::ossimQuickbirdRpcModel()
    : ossimRpcModel(),
      theSupportData(new ossimQuickbirdMetaData())
{
}

//*************************************************************************************************
// Constructor
//*************************************************************************************************
ossimQuickbirdRpcModel::ossimQuickbirdRpcModel(const ossimQuickbirdRpcModel &rhs)
    : ossimRpcModel(rhs),
      theSupportData(new ossimQuickbirdMetaData())
{
}

//*************************************************************************************************
//! Constructor for multiple tile-files sharing common RPC model initializes given pointer
//! to multi-tile-files handler.
//*************************************************************************************************
ossimQuickbirdRpcModel::ossimQuickbirdRpcModel(const ossimQbTileFilesHandler *handler)
    : ossimRpcModel(),
      theSupportData(new ossimQuickbirdMetaData())
{
   setErrorStatus();
   if (!handler)
      return;

   // Make the gsd nan so it gets computed.
   theGSD.makeNan();

   theImageClipRect = handler->getImageRectangle();

   ossimFilename imageFile = handler->getFilename();
   if (!parseRpcData(imageFile))
      return;

   finishConstruction();
   clearErrorStatus();
   return;
}

//*************************************************************************************************
// Destructor
//*************************************************************************************************
ossimQuickbirdRpcModel::~ossimQuickbirdRpcModel()
{
   theSupportData = 0;
}

//*************************************************************************************************
// Infamous DUP
//*************************************************************************************************
ossimObject *ossimQuickbirdRpcModel::dup() const
{
   return new ossimQuickbirdRpcModel(*this);
}

//*************************************************************************************************
//! Public method for parsing generic image file.
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseFile(const ossimFilename &file)
{
   bool result = true;
   while (1)
   {
      if (parseNitfFile(file))
         break;
      if (parseTiffFile(file))
         break;

      result = false;
      break;
   }

   return result;
}

//*************************************************************************************************
//! Parses a NITF image file for RPC info. Returns TRUE if successful.
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseNitfFile(const ossimFilename &file)
{
   setErrorStatus();

   //---
   // ossimNitfFile::parseFile(...) checks the first eight byte so three calls
   // to parseFile are not necessary. drb - 21 Dec. 2016
   //---
   ossimRefPtr<ossimNitfFile> nitfFilePtr = new ossimNitfFile;
   if (nitfFilePtr->parseFile(file) == false)
   {
      return false;
   }

#if 0   
   ossimFilename nitfFile = file;
   ossimRefPtr<ossimNitfFile> nitfFilePtr = new ossimNitfFile;
   if(!nitfFilePtr->parseFile(nitfFile))
   {
      nitfFile = nitfFile.setExtension("NTF");
      if(!nitfFilePtr->parseFile(nitfFile))
      {
         nitfFile = nitfFile.setExtension("ntf");
         if(!nitfFilePtr->parseFile(nitfFile))
            return false;
      }
   }
#endif

   ossimRefPtr<ossimNitfImageHeader> ih = nitfFilePtr->getNewImageHeader(0);
   if (!ih)
      return false;

   theImageClipRect = ih->getImageRect();

   // Give preference to external RPC data file. Only consider external tiling data if RPC is
   // provided externally, otherwise expect ICHIPB in NITF:
   bool useInternalRpcTags = true;
   if (parseRpcData(file))
   {
      useInternalRpcTags = false;
      if (!parseTileData(file))
         return false;
   }

   // Check for IMD (metadata) file:
   parseMetaData(file);

   // Get the gsd.
   theGSD.line = ossim::nan();
   theGSD.samp = ossim::nan();

   ossimRefPtr<ossimNitfRegisteredTag> tag;
   tag = ih->getTagData(PIAIMC_TAG);
   if (tag.valid())
   {
      ossimNitfPiaimcTag *p = PTR_CAST(ossimNitfPiaimcTag, tag.get());
      if (p)
      {
         theGSD.line = p->getMeanGsdInMeters();
         theGSD.samp = theGSD.line;
      }
   }
   if (ossim::isnan(theGSD.line))
   {
      tag = ih->getTagData(USE00A_TAG);
      if (tag.valid())
      {
         ossimNitfUse00aTag *p = PTR_CAST(ossimNitfUse00aTag, tag.get());
         if (p)
         {
            theGSD.line = p->getMeanGsdInMeters();
            theGSD.samp = theGSD.line;
         }
      }
   }

   // If external RPC data file was correctly parsed, then we can bypass this code block. Otherwise
   // need to parse internal NITF tags for RPC data:
   if (useInternalRpcTags)
   {
      // Get the the RPC tag:
      ossimNitfRpcBase *rpcTag = NULL;

      // Look for the RPC00B tag first.
      tag = ih->getTagData(RPC00B_TAG);
      if (tag.valid())
         rpcTag = PTR_CAST(ossimNitfRpcBase, tag.get());

      if (!tag.valid())
      {
         // Look for RPC00A tag.
         tag = ih->getTagData(RPC00A_TAG);
         if (tag.valid())
            rpcTag = PTR_CAST(ossimNitfRpcBase, tag.get());
      }

      if (!rpcTag)
         return false;

      // Set the polynomial type.
      if (rpcTag->getRegisterTagName() == "RPC00B")
         thePolyType = B;
      else
         thePolyType = A;

      // Parse coefficients:
      for (ossim_uint32 i = 0; i < 20; ++i)
      {
         theLineNumCoef[i] = rpcTag->getLineNumeratorCoeff(i).toFloat64();
         theLineDenCoef[i] = rpcTag->getLineDenominatorCoeff(i).toFloat64();
         theSampNumCoef[i] = rpcTag->getSampleNumeratorCoeff(i).toFloat64();
         theSampDenCoef[i] = rpcTag->getSampleDenominatorCoeff(i).toFloat64();
      }

      // Initialize other items in tags:
      theLineScale  = rpcTag->getLineScale().toFloat64();
      theSampScale  = rpcTag->getSampleScale().toFloat64();
      theLatScale   = rpcTag->getGeodeticLatScale().toFloat64();
      theLonScale   = rpcTag->getGeodeticLonScale().toFloat64();
      theHgtScale   = rpcTag->getGeodeticHeightScale().toFloat64();
      theLineOffset = rpcTag->getLineOffset().toFloat64();
      theSampOffset = rpcTag->getSampleOffset().toFloat64();
      theLatOffset  = rpcTag->getGeodeticLatOffset().toFloat64();
      theLonOffset  = rpcTag->getGeodeticLonOffset().toFloat64();
      theHgtOffset  = rpcTag->getGeodeticHeightOffset().toFloat64();
      theBiasError  = rpcTag->getErrorBias().toFloat64();
      theRandError  = rpcTag->getErrorRand().toFloat64();
      theImageID    = ih->getImageId();
   }

   finishConstruction();
   clearErrorStatus();
   return true;
}

//*************************************************************************************************
//! Parses a tagged TIFF image file for RPC info. Returns TRUE if successful.
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseTiffFile(const ossimFilename &file)
{
   setErrorStatus();

   // Make the gsd nan so it gets computed.
   theGSD.makeNan();

   ossimFilename tiffFile = file;
   ossimRefPtr<ossimTiffTileSource> tiff = new ossimTiffTileSource();
   if (!tiff->open(file))
   {
      return false;
   }

   theImageClipRect = tiff->getImageRectangle();

   parseMetaData(file);

   // TIFF format expects the RPC and TILE info to be provided externally:
   if (!parseRpcData(file))
      return false;

   // If no TIL data present, assumes full image:
   if (!parseTileData(file))
      return false;

   finishConstruction();
   clearErrorStatus();
   return true;
}

//*************************************************************************************************
//! Given some base name for the image data, parses the associated RPC data file. Returns TRUE
//! if successful
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseRpcData(const ossimFilename &base_name)
{
   ossimFilename rpcFile(base_name);

   // Make the gsd nan so it gets computed.
   theGSD.makeNan();

   // There are three possibilities for RPC data files: either each image file has its own RPC data
   // file, or a single RPC file is provided for a multi-tile scene.
   while (1)
   {
      rpcFile.setExtension("RPB");
      if (findSupportFile(rpcFile)) break;
      
      rpcFile.setExtension("RPA");
      if (findSupportFile(rpcFile)) break;
      
      rpcFile.setExtension("XML");
      if (findSupportFile(rpcFile)) break;

      // SkySat or BlackSky images - START
      ossimFilename blackSkyRPC = rpcFile.noExtension().append( "_rpc.txt" );

      if (findSupportFile(blackSkyRPC)) {
         // std::cout << "HERE: Found BlackSky" << std::endl;
         rpcFile = blackSkyRPC;
         break;
      }

      ossimFilename skySatRPC = rpcFile.before("_file_format").append( "_RPC.TXT" );

      if (findSupportFile(skySatRPC)) {
         // std::cout << "HERE: Found SkySat" << std::endl;
         rpcFile = skySatRPC;
         break;
      }
      // SkySat or BlackSky images - END

      return false;
   }

   // An RPC file was located, open it:
   m_qbRpcHeader = std::make_shared<ossimQuickbirdRpcHeader>();
   if (!m_qbRpcHeader->open(rpcFile))
   {
      // std::cout << "HERE: Cannot Open: " << rpcFile << std::endl;
      m_qbRpcHeader = 0;
      return false;
   }


   if (m_qbRpcHeader->isAPolynomial())
      thePolyType = A;
   else
      thePolyType = B;

   std::copy(m_qbRpcHeader->theLineNumCoeff.begin(), m_qbRpcHeader->theLineNumCoeff.end(), theLineNumCoef);
   std::copy(m_qbRpcHeader->theLineDenCoeff.begin(), m_qbRpcHeader->theLineDenCoeff.end(), theLineDenCoef);
   std::copy(m_qbRpcHeader->theSampNumCoeff.begin(), m_qbRpcHeader->theSampNumCoeff.end(), theSampNumCoef);
   std::copy(m_qbRpcHeader->theSampDenCoeff.begin(), m_qbRpcHeader->theSampDenCoeff.end(), theSampDenCoef);

   theLineScale = m_qbRpcHeader->theLineScale;
   theSampScale = m_qbRpcHeader->theSampScale;
   theLatScale = m_qbRpcHeader->theLatScale;
   theLonScale = m_qbRpcHeader->theLonScale;
   theHgtScale = m_qbRpcHeader->theHeightScale;
   theLineOffset = m_qbRpcHeader->theLineOffset;
   theSampOffset = m_qbRpcHeader->theSampOffset;
   theLatOffset = m_qbRpcHeader->theLatOffset;
   theLonOffset = m_qbRpcHeader->theLonOffset;
   theHgtOffset = m_qbRpcHeader->theHeightOffset;
   theBiasError  = m_qbRpcHeader->theErrBias;
   theRandError  = m_qbRpcHeader->theErrRand;
   theImageID = rpcFile.fileNoExtension();

   if (theImageSize.length() == 0)
   {
      theImageSize.x = 2 * (int) theSampOffset;
      theImageSize.y = 2 * (int) theLineOffset;
      theRefImgPt = ossimDpt(theSampOffset, theLineOffset);
   }

   finishConstruction();

   return true;
}

//*************************************************************************************************
//! Initializes the support data member with metadata file info. Returns TRUE if successful
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseMetaData(const ossimFilename &base_name)
{

   ossimFilename metadataFile(base_name);
   metadataFile.setExtension("IMD");
   if (!findSupportFile(metadataFile))
      return false;

   if (!theSupportData.valid())
      theSupportData = new ossimQuickbirdMetaData();

   if (!theSupportData->open(metadataFile))
   {
      theSupportData = 0; // ossimRefPtr
#if 0                     /* This should be wrapped in trace. drb */
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimQuickbirdRpcModel::parseMetaData WARNING:"
         << "\nCould not open IMD file.  Sensor ID unknown." << std::endl;
#endif
      return false;
   }

   theSensorID = theSupportData->getSatID();
   return true;
}

//*************************************************************************************************
//! Reads the TIL file for pertinent info. Returns TRUE if successful
//*************************************************************************************************
bool ossimQuickbirdRpcModel::parseTileData(const ossimFilename &image_file)
{
   ossimFilename tileFile(image_file);
   tileFile.setExtension("TIL");

   // The TIL file is optional. Consider the image to be the full image if not present:
   if (!findSupportFile(tileFile))
      return true;

   ossimQuickbirdTile tileHdr;
   if (!tileHdr.open(tileFile))
      return false;

   ossimQuickbirdTileInfo info;
   if (!tileHdr.getInfo(info, image_file.file()))
      return false;

   //   if((info.theUlXOffset != OSSIM_INT_NAN) && (info.theUlYOffset != OSSIM_INT_NAN) &&
   //      (info.theLrXOffset != OSSIM_INT_NAN) && (info.theLrYOffset != OSSIM_INT_NAN) &&
   //      (info.theLlXOffset != OSSIM_INT_NAN) && (info.theLlYOffset != OSSIM_INT_NAN) &&
   //      (info.theUrXOffset != OSSIM_INT_NAN) && (info.theUrYOffset != OSSIM_INT_NAN))
   //   {
   //      theImageClipRect = ossimIrect(ossimIpt(info.theUlXOffset, info.theUlYOffset),
   //                                    ossimIpt(info.theUrXOffset, info.theUrYOffset),
   //                                    ossimIpt(info.theLrXOffset, info.theLrYOffset),
   //                                    ossimIpt(info.theLlXOffset, info.theLlYOffset));
   //   }
   //   else if ((info.theUlXOffset != OSSIM_INT_NAN) && (info.theUlYOffset != OSSIM_INT_NAN) &&
   //      (theImageClipRect.width() != OSSIM_INT_NAN) && (theImageClipRect.height() != OSSIM_INT_NAN))
   //   {
   //      theImageClipRect = ossimIrect(info.theUlXOffset, info.theUlYOffset,
   //                                    info.theUlXOffset+theImageClipRect.width()-1,
   //                                    info.theUlYOffset+theImageClipRect.height()-1);
   //   }

   // Define the RPC model's 2D transform for chipped imagery. Note that the TIL file will only
   // define an offset, not a full affine.  Can only use the tile's UL corner:
   ossimDpt ul (info.theUlXOffset, info.theUlYOffset);

   //********************************************
   // gpotts: I am not confident that the TIL is needed for the RPC WV02.  The
   // RPC has a line  and offset and what I have seen they are for the local
   // image not it's tile location. Added chack and will only use if its quickbird
   //
   //
   if (m_qbRpcHeader)
   {
      if (m_qbRpcHeader->isGlobal() && (!ul.hasNans()))
      {
         setImageOffset(ul);
      }
   }

      return true;
}

//*************************************************************************************************
//! Collects common code among all parsers
//*************************************************************************************************
void ossimQuickbirdRpcModel::finishConstruction()
{
   theImageSize.line = theImageClipRect.height();
   theImageSize.samp = theImageClipRect.width();
   theRefImgPt.line = theImageClipRect.midPoint().y;
   theRefImgPt.samp = theImageClipRect.midPoint().x;
   theRefGndPt.lat = theLatOffset;
   theRefGndPt.lon = theLonOffset;
   theRefGndPt.hgt = theHgtOffset;

   //---
   // NOTE:  We must call "updateModel()" to set parameter used by base
   // ossimRpcModel prior to calling lineSampleHeightToWorld or all
   // the world points will be same.
   //---
   updateModel();

   ossimGpt v0, v1, v2, v3;
   lineSampleHeightToWorld(theImageClipRect.ul(), theHgtOffset, v0);
   lineSampleHeightToWorld(theImageClipRect.ur(), theHgtOffset, v1);
   lineSampleHeightToWorld(theImageClipRect.lr(), theHgtOffset, v2);
   lineSampleHeightToWorld(theImageClipRect.ll(), theHgtOffset, v3);

   theBoundGndPolygon = ossimPolygon(ossimDpt(v0), ossimDpt(v1), ossimDpt(v2), ossimDpt(v3));

   // Set the ground reference point using the model.
   lineSampleHeightToWorld(theRefImgPt, theHgtOffset, theRefGndPt);

   if (theGSD.hasNans())
   {
      try
      {
         // This will set theGSD and theMeanGSD. Method throws ossimException.
         computeGsd();
      }
      catch (const ossimException &e)
      {
         ossimNotify(ossimNotifyLevel_WARN)
             << "ossimQuickbirdRpcModel::finishConstruction -- caught exception:\n"
             << e.what() << std::endl;
      }
   }
}

bool ossimQuickbirdRpcModel::saveState(ossimKeywordlist &kwl,
                                       const char *prefix) const
{
   // sanity check only.  This shoulc always be true
   if (theSupportData.valid())
   {
      ossimString supportPrefix = ossimString(prefix) + "support_data.";
      theSupportData->saveState(kwl, supportPrefix);
   }

   return ossimRpcModel::saveState(kwl, prefix);
}

bool ossimQuickbirdRpcModel::loadState(const ossimKeywordlist &kwl,
                                       const char *prefix)
{
   // sanity check only.  This shoulc always be true
   if (theSupportData.valid())
   {
      ossimString supportPrefix = ossimString(prefix) + "support_data.";
      theSupportData->loadState(kwl, supportPrefix);
   }
   return ossimRpcModel::loadState(kwl, prefix);
}

//*************************************************************************************************
// Given an initial filename with case-agnostic extension, this method searches first for an
// image-specific instance of that file (i.e., with R*C* in the filename) before considering
// the mosaic-global support file (R*C* removed). If a file is found, the argument is modified to
// match the actual filename and TRUE is returned. Otherwise, argument filename is left unchanged
// and FALSE is returned.
//*************************************************************************************************
bool ossimQuickbirdRpcModel::findSupportFile(ossimFilename &filename) const
{
   ossimFilename f(filename);
   ossimString extension = f.ext();
   if (f.exists())
   {
      filename = f;
      return true;
   }
   // Search for support file with same basename as image:
   extension.upcase();
   f.setExtension(extension);
   if (f.exists())
   {
      filename = f;
      return true;
   }
   extension.downcase();
   f.setExtension(extension);
   if (f.exists())
   {
      filename = f;
      return true;
   }

   // None found so far, search for mosaic-global support file:
   f = f.replaceAllThatMatch("_R[0-9]+C[0-9]+");
   if (f.exists())
   {
      filename = f;
      return true;
   }
   extension.upcase();
   f.setExtension(extension);
   if (f.exists())
   {
      filename = f;
      return true;
   }

   // Modify argument to match good filename:
   return false;
}
