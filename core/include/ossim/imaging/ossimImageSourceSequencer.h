//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossimImageSourceSequencer.h 23081 2015-01-14 21:38:42Z dburken $
#ifndef ossimImageSourceSequencer_HEADER
#define ossimImageSourceSequencer_HEADER 1

#include <ossim/imaging/ossimImageSource.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimConnectableObjectListener.h>


class OSSIMDLLEXPORT ossimImageSourceSequencer
   :
      public ossimImageSource,
      public ossimConnectableObjectListener
{
public:
   ossimImageSourceSequencer(ossimImageSource* inputSource=NULL,
                             ossimObject* owner=NULL);

   virtual ~ossimImageSourceSequencer();
  /*!
    * This will return the number of tiles within the
    * area of interest.
    */
   ossim_int64 getNumberOfTiles()const;

   /*!
    * Will return the number of tiles along the
    * x or horizontal direction.
    */
   ossim_int64 getNumberOfTilesHorizontal()const;

   /*!
    * Will return the number of tiles along the
    * y or vertical direction.
    */
   ossim_int64 getNumberOfTilesVertical()const;

   /*!
    * This must be called.  We can only initialize this
    * object completely if we know all connections
    * are valid.  Some other object drives this and so the
    * connection's initialize will be called after.  The job
    * of this connection is to set up the sequence.  It will
    * default to the bounding rect.  The area of interest can be
    * set to some other rectagle (use setAreaOfInterest).
    */
   virtual void initialize();

   /*!
    * Will set the current area of interest.
    */
   virtual void setAreaOfInterest(const ossimIrect& areaOfInterest);

   /*!
    * Just returns the current area of interest.
    */
   const ossimIrect& getAreaOfInterest()const;

   /*!
    * Will set the internal pointers to the upperleft
    * tile number.  To go to the next tile in the sequence
    * just call getNextTile.
    */
   virtual void setToStartOfSequence();

   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& rect,
                                               ossim_uint32 resLevel=0);

   /*!
    * Will allow you to get the next tile in the sequence.
    * Note the last tile returned will be an invalid
    * ossimRefPtr<ossimImageData>.  Callers should be able to do:
    * 
    * ossimRefPtr<ossimImageData> id = sequencer->getNextTile();
    * while (id.valid())
    * {
    *    doSomething;
    *    id = sequencer->getNextTile();
    * }
    * 
    */
   virtual ossimRefPtr<ossimImageData> getNextTile(ossim_uint32 resLevel=0);

   virtual bool getTileOrigin(ossim_int64 id, ossimIpt& origin)const;

   /*!
    * @brief Establishes a tile rect given tile ID.
    * @param tile_id
    * @param rect Rectangle to initialize.
    * @return true if valid; else, false.
    */
   bool getTileRect(ossim_int64 tile_id, ossimIrect& rect) const;

   virtual ossimRefPtr<ossimImageData> getTile(ossim_int64 id,
                                               ossim_uint32 resLevel=0);

   virtual ossimIrect getBoundingRect(ossim_uint32 resLevel=0)const;
   virtual void getDecimationFactor(ossim_uint32 resLevel,
                                    ossimDpt& result) const;
   virtual void getDecimationFactors(vector<ossimDpt>& decimations) const;
   virtual ossim_uint32 getNumberOfDecimationLevels()const;
   
   /*!
    * Returns the number of bands available from the input.
    */
   virtual ossim_uint32 getNumberOfInputBands()const;   

   virtual ossimScalarType getOutputScalarType() const;
      
   virtual ossim_uint32 getTileWidth() const;
   virtual ossim_uint32 getTileHeight() const;
   
   virtual void slaveProcessTiles();
   virtual bool isMaster()const;

   virtual ossimIpt getTileSize()const;
   virtual void setTileSize(const ossimIpt& tileSize);
   virtual void setTileSize(ossim_int32 width, ossim_int32 height);
   
   virtual void connectInputEvent(ossimConnectionEvent& event);
   virtual void disconnectInputEvent(ossimConnectionEvent& event);
   
   virtual bool canConnectMyInputTo(ossim_int32 inputIndex,
                                    const ossimConnectableObject* object)const;
   
   virtual double getNullPixelValue(ossim_uint32 band=0)const;
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;
   
protected:
   ossimImageSource*  theInputConnection;
   ossimRefPtr<ossimImageData> theBlankTile;
   /*!
    * Is the area of interest.  The default will
    * 
    */
   ossimIrect theAreaOfInterest;

   /*!
    * Called during initialize.
    */
   ossimIpt theTileSize;

   //---
   // These need to be big(64 bit) for high resolution tile servers. Made
   // signed for ease of going to/from ipt and irect which are both signed.
   //---
   ossim_int64 theNumberOfTilesHorizontal;
   ossim_int64 theNumberOfTilesVertical;
   ossim_int64 theCurrentTileNumber;

   virtual void updateTileDimensions();

TYPE_DATA
};

#endif
