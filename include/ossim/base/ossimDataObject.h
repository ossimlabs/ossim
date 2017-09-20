//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: ossimDataObject.h 23505 2015-09-08 19:04:19Z dburken $

#ifndef ossimDataObject_HEADER
#define ossimDataObject_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimDpt3d.h>

class ossimSource;

class OSSIMDLLEXPORT ossimDataObject : public ossimObject
{
public:
   ossimDataObject(ossimSource* source=0,
                   ossimDataObjectStatus status=OSSIM_STATUS_UNKNOWN);
   
   ossimDataObject(const ossimDataObject& rhs);
   virtual ~ossimDataObject();



   virtual ossim_uint32 getHashId()const=0;
   
   /**
    * Sets the owner of this Data object.
    */
   virtual void setOwner(ossimSource* aSource);

   virtual ossimSource* getOwner();

   virtual const ossimSource* getOwner() const;

   /**
    * Full list found in ossimConstants.h
    *
    * OSSIM_STATUS_UNKNOWN
    * OSSIM_NULL            Null data.  The buffers are never allocated
    * OSSIM_EMPTY           Allocated data but its empty.  Most of the
    *                       time this will be the default for an initialized
    *                       data object.
    * OSSIM_PARTIAL         Says that some of the object is empty and is only
    *                       partially full
    * OSSIM_FULL            This data is completey full
    */
   virtual void  setDataObjectStatus(ossimDataObjectStatus status) const;

   /**
    * @return Returns enumerated status:
    * OSSIM_STATUS_UNKNOWN = 0
    * OSSIM_NULL           = 1
    * OSSIM_EMPTY          = 2
    * OSSIM_PARTIAL        = 3
    * OSSIM_FULL           = 4
    */
   virtual ossimDataObjectStatus getDataObjectStatus() const;

   /**
    * @return Returns the status as a string, like "OSSIM_NULL".
    */
   virtual ossimString getDataObjectStatusString() const;

   virtual ossim_uint32 getObjectSizeInBytes()const;

   virtual ossim_uint32 getDataSizeInBytes()const=0;

   /**
    * This is to be overriden in the derived classes so they can check
    * If their data is null.
    */
   virtual bool isInitialize() const;

   /**
    * Will allow derived classes to initialize their buffers.  This class
    * does nothing with this and reset any internal attributes.
    */
   virtual void initialize()=0;
   
   virtual void assign(const ossimDataObject* rhs);

   virtual const ossimDataObject* operator =(const ossimDataObject* rhs);
   
   virtual const ossimDataObject& operator=(const ossimDataObject& rhs);

   virtual bool operator==(const ossimDataObject& rhs) const;
   virtual bool operator!=(const ossimDataObject& rhs) const;

   virtual std::ostream& print(std::ostream& out) const;
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

protected:

   //***
   // Note:  This object is not responsible for deletion of the ossimSource*
   //        "theSource".  It is simply a hook to his owner for things
   //        like unregister and so forth...
   //***
   ossimSource*                      theOwner;
   mutable ossimDataObjectStatus     theDataObjectStatus;

TYPE_DATA
};

#endif
