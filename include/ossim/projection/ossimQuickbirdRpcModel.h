//*****************************************************************************
// FILE: ossimQuickbirdRpcModel.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// DESCRIPTION: Contains declaration of class ossimQuickbirdRpcModel. This
//    derived class implements the capability of reading Quickbird RPC support
//    data.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id$
#ifndef ossimQuickbirdRpcModel_HEADER
#define ossimQuickbirdRpcModel_HEADER 1

#include <ossim/projection/ossimRpcModel.h>
#include <memory>

class ossimFilename;
class ossimQuickbirdMetaData;
class ossimQuickbirdRpcHeader;
class ossimQbTileFilesHandler;

/*!****************************************************************************
 *
 * CLASS:  ossimQuickbirdRpcModel
 *
 *****************************************************************************/
class ossimQuickbirdRpcModel : public ossimRpcModel
{
public:
   ossimQuickbirdRpcModel();
   ossimQuickbirdRpcModel(const ossimQuickbirdRpcModel &rhs);

   //! Initializes
   ossimQuickbirdRpcModel(const ossimQbTileFilesHandler *handler);
   ~ossimQuickbirdRpcModel();

   virtual ossimObject *dup() const;
   virtual bool saveState(ossimKeywordlist &kwl,
                          const char *prefix = 0) const;

   virtual bool loadState(const ossimKeywordlist &kwl,
                          const char *prefix = 0);

   bool parseFile(const ossimFilename& file);
   
   /**
    * @brief Looks for DigitalGlobe style dot TIL, IMD and RPB or RPA files.
    * This does not open up a tiff reader just to get the image rectangle.
    * @baseFile
    * @imageRect
    * @return true on success, false on error.
    */
   bool parseFiles(const ossimFilename& baseFile, const ossimDrect& imageRect);
   

   void setSupportData(ossimQuickbirdMetaData *supportData);
   ossimQuickbirdMetaData *getSupportData();
   const ossimQuickbirdMetaData *getSupportData() const;

   /** Initializes this with contents of RPC file. IMPORTANT: This is not enough to fully initialize
    * the model. The image size and reference (mid) point need to be set before calling this method
    * so that the GSD is properly computed. A guesstimate of image size is made based
    * on line/samp offset params, but this will probably be wrong. Returns TRUE if successful. */
   bool parseRpcData(const ossimFilename &file);

protected:
   bool parseNitfFile(const ossimFilename &file);
   bool parseTiffFile(const ossimFilename &file);
   bool parseMetaData(const ossimFilename &file);
   bool parseTileData(const ossimFilename &file);

   //! Given an initial filename with case-agnostic extension, this method searches first for an
   //! image-specific instance of that file (i.e., with _R*C* in the filename) before considering
   //! the mosaic-global support file (_R*C* removed). If a file is found, the argument is modified
   //! to match the actual filename and TRUE is returned. Otherwise, argument filename is left
   //! unchanged and FALSE is returned.
   bool findSupportFile(ossimFilename &file) const;

   void finishConstruction();

   ossimRefPtr<ossimQuickbirdMetaData> theSupportData;
   std::shared_ptr<ossimQuickbirdRpcHeader> m_qbRpcHeader;

TYPE_DATA
};

#endif
