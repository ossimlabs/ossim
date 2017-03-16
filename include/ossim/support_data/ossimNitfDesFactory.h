//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: ossimNitfDesFactory.h 17207 2010-04-25 23:21:14Z dburken $
#ifndef ossimNitfDesFactory_HEADER
#define ossimNitfDesFactory_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>

class ossimNitfRegisteredDes;

class OSSIM_DLL ossimNitfDesFactory : public ossimObject
{
public:
   ossimNitfDesFactory();
   virtual ~ossimNitfDesFactory();
   virtual ossimRefPtr<ossimNitfRegisteredDes> create(const ossimString &desName)const=0;
private:
   /*!
    * Hide this.
    */
   ossimNitfDesFactory(const ossimNitfDesFactory & /* rhs */){}

   /*!
    * Hide this.
    */ 
   ossimNitfDesFactory& operator =(const ossimNitfDesFactory & /* rhs */){return *this;}
TYPE_DATA
};
#endif
