//*******************************************************************
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id$

#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/base/ossimContainerProperty.h>
#include <iostream>

static const char* TAGS_KW = "tags";

RTTI_DEF2(ossimNitfFileHeader, "ossimNitfFileHeader", ossimObject, ossimPropertyInterface);

std::ostream& ossimNitfImageOffsetInformation::print(std::ostream& out) const
{
   return out << "theImageHeaderOffset:          "
              << theImageHeaderOffset << std::endl
              << "theImageDataOffset:            "
              << theImageDataOffset;
}

ossimNitfFileHeader::ossimNitfFileHeader()
{
}

ossimNitfFileHeader::~ossimNitfFileHeader()
{
}


bool ossimNitfFileHeader::getTag(ossimNitfTagInformation& tagInfo,
                                 const ossimString& tagName)const
{
   if(theTagList.size())
   {
      for(ossim_uint32 i = 0; i < theTagList.size(); ++i)
      {
         if(theTagList[i].getTagName() == tagName)
         {
            tagInfo = theTagList[i];
            return true;
         }
      }
   }

   return false;
}

ossimRefPtr<ossimNitfRegisteredTag> ossimNitfFileHeader::getTagData(const ossimString &tagName)
{
   if (theTagList.size())
   {
      for (ossim_uint32 idx = 0; idx < theTagList.size(); ++idx)
      {
         if (theTagList[idx].getTagName() == tagName)
         {
            return theTagList[idx].getTagData();
         }
      }
   }

   return ossimRefPtr<ossimNitfRegisteredTag>();
}

const ossimRefPtr<ossimNitfRegisteredTag> ossimNitfFileHeader::getTagData(
    const ossimString &tagName) const
{
   if (theTagList.size())
   {
      for (ossim_uint32 idx = 0; idx < theTagList.size(); ++idx)
      {
         if (theTagList[idx].getTagName() == tagName)
         {
            return theTagList[idx].getTagData();
         }
      }
   }

   return ossimRefPtr<ossimNitfRegisteredTag>();
}

bool ossimNitfFileHeader::getDesInformation(ossimNitfDesInformation &desInfo,
                                            const ossimString &desId,
                                            bool exactMatch)
{
   if (theDesList.size())
   {
      for (ossim_uint32 i = 0; i < theDesList.size(); ++i)
      {
         if(exactMatch)
         {
            if (theDesList[i].getDesId() == desId)
            {
               desInfo = theDesList[i];
               return true;
            }
         }
         else
         {
            if(theDesList[i].getDesId().contains(desId))
            {
               desInfo = theDesList[i];
               return true;
            }
         }
      }
   }
   return false;
}

bool  ossimNitfFileHeader::hasImages()const
{
   return (getNumberOfImages() > 0);
}

bool  ossimNitfFileHeader::hasSymbols()const
{
   return (getNumberOfSymbols() > 0);
}

bool  ossimNitfFileHeader::hasGraphics()const
{
   return (getNumberOfGraphics() > 0);
}

bool  ossimNitfFileHeader::hasText()const
{
   return (getNumberOfTextSegments() > 0);
}

bool  ossimNitfFileHeader::hasLabels()const
{
   return (getNumberOfLabels() > 0);
}
bool ossimNitfFileHeader::hasDataExtSegments()const
{
   return (getNumberOfDataExtSegments() > 0);
}

void ossimNitfFileHeader::removeTag(const ossimString& tagName)
{
   ossim_uint32 idx = 0;
   for(idx = 0; idx < theTagList.size(); ++idx)
   {
      if(theTagList[idx].getTagName() == tagName)
      {
         theTagList.erase(theTagList.begin() + idx);
         return;
      }
   }
}
void ossimNitfFileHeader::addTag(const ossimNitfTagInformation& tag, bool unique)
{
   if(unique)
   {
      removeTag(tag.getTagName());
   }
   theTagList.push_back(tag);
}

bool ossimNitfFileHeader::getTagInformation(ossimNitfTagInformation& tag,
                                            int idx) const
{
   bool result = false;
   
   if(static_cast<ossim_uint32>(idx) < theTagList.size())
   {
      tag = theTagList[idx];
      result = true;
   }
   
   return result;
}

int ossimNitfFileHeader::getNumberOfTags()const
{
   return (int)theTagList.size();
}

const std::vector<ossimNitfDesInformation> &ossimNitfFileHeader::getDesInfoList() const 
{
    return theDesList;
}

ossim_uint32 ossimNitfFileHeader::getTotalTagLength() const
{
   ossim_uint32 tagLength = 0;
   
   for(ossim_uint32 i = 0; i < theTagList.size(); ++i)
   {
      tagLength += theTagList[i].getTotalTagLength();
   }
   
   return tagLength;
}


void ossimNitfFileHeader::setProperty(ossimRefPtr<ossimProperty> /* property */)
{
}

ossimRefPtr<ossimProperty> ossimNitfFileHeader::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> result = 0;

   if(name == TAGS_KW)
   {
      ossim_uint32 idxMax = (ossim_uint32)theTagList.size();
      if(idxMax > 0)
      {
         ossimContainerProperty* containerProperty = new ossimContainerProperty;
         containerProperty->setName(name);
         ossim_uint32 idx = 0;

         result = containerProperty;
         for(idx = 0; idx < idxMax; ++idx)
         {
            ossimNitfTagInformation tagInfo;
            getTagInformation(tagInfo, idx);
            
            const ossimRefPtr<ossimNitfRegisteredTag> tagData = tagInfo.getTagData();
            if(tagData.valid())
            {
               ossimContainerProperty* containerPropertyTag = new ossimContainerProperty;
               containerPropertyTag->setName(tagInfo.getTagName());
               std::vector<ossimRefPtr<ossimProperty> > propertyList;
               tagData->getPropertyList(propertyList);
               
               containerPropertyTag->addChildren(propertyList);
               containerProperty->addChild(containerPropertyTag);
            }
            else
            {
               containerProperty->addStringProperty(tagInfo.getTagName(), "", true);
            }
         }
      }
   }
   
   return result;
}

void ossimNitfFileHeader::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back(TAGS_KW);
}

bool ossimNitfFileHeader::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
{
   ossimObject::saveState(kwl, prefix);
   
   bool result = true;
   ossimString tagsPrefix = prefix;
   ossim_uint32 i = 0;
   for(; i < theTagList.size(); ++i)
   {
      ossimRefPtr<ossimNitfRegisteredTag> tag = theTagList[i].getTagData();
      if (tag.valid())
      {
         // we eventually need to do our own prefix for this object and not let the calling object do any prefix definitions
//         ossimString newPrefix = tagsPrefix + tag->getRegisterTagName() + ".";

         tag->saveState(kwl, tagsPrefix);
      }
   }
   i = 0;
   ossimString desPrefix = prefix + "des";
   for (auto des : theDesList)
   {
      des.saveState(kwl, desPrefix+ossimString(i) + ".");
   }
   return result;
}

std::ostream& ossimNitfFileHeader::print(std::ostream& out,
                                         const std::string& prefix ) const
{
   return printTags(out, prefix);
}

std::ostream& ossimNitfFileHeader::printTags(std::ostream& out,
                                             const std::string& prefix) const
{
   ossim_uint32 i = 0;
   for (; i < theTagList.size(); ++i)
   {
      ossimRefPtr<ossimNitfRegisteredTag> tag = theTagList[i].getTagData();
      if (tag.valid())
      {
         tag->print(out, prefix);
      }
   }
   ossimString desPrefix = prefix + "des";
   for (i = 0; i < theDesList.size(); ++i)
   {
      theDesList[i].print(out, desPrefix + ossimString::toString(i) + ".");
   }

   return out;
}
