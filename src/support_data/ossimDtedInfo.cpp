//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: DTED Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <ossim/support_data/ossimDtedInfo.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimRegExp.h>


ossimDtedInfo::ossimDtedInfo()
{
}

ossimDtedInfo::~ossimDtedInfo()
{
}

bool ossimDtedInfo::open(std::shared_ptr<ossim::istream>& str,
                         const std::string& connectionString)
{
   bool result = false;
   if(!str) return false;
   // Test for extension, like dt0, dt1...
   ossimString ext = ossimFilename(connectionString).ext();
   ossimRegExp regExp("^[d|D][t|T][0-9]");
   m_dtedFileStr.reset();
   if ( regExp.find( ext.c_str() ) )
   {
      m_vol.parse(*str);
      m_hdr.parse(*str);
      m_uhl.parse(*str);
      m_dsi.parse(*str);
      m_acc.parse(*str);
      //---
      // Check for errors.  Must have uhl, dsi and acc records.  vol and hdr
      // are for magnetic tape only; hence, may or may not be there.
      //---
      if ( (m_uhl.getErrorStatus() == ossimErrorCodes::OSSIM_OK) &&
           (m_dsi.getErrorStatus() == ossimErrorCodes::OSSIM_OK) &&
           (m_acc.getErrorStatus() == ossimErrorCodes::OSSIM_OK) )
      {
         result = true;
         m_connectionString = connectionString;
         m_dtedFileStr = str;
      }
      else
      {
         m_connectionString.clear();
         m_dtedFileStr.reset();
      }
   }

   return result;
}

std::ostream& ossimDtedInfo::print(std::ostream& out) const
{
   std::string prefix = "dted.";
   if( m_vol.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      m_vol.print(out, prefix);
   }
   if( m_hdr.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      m_hdr.print(out, prefix);
   }
   if( m_uhl.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      m_uhl.print(out, prefix);
   }
   if( m_dsi.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      m_dsi.print(out, prefix);
   }
   if( m_acc.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      m_acc.print(out, prefix);
   }
   return out;
}

ossimRefPtr<ossimProperty> ossimDtedInfo::getProperty(
   const ossimString& name)const
{
   ossimRefPtr<ossimProperty> result = 0;

   //---
   // Look through dted records.
   // Must have uhl, dsi and acc records.  vol and hdr
   // are for magnetic tape only; hence, may or may not be there.
   //---
   //ossimDtedVol vol(m_dtedFileStr, 0);
   if( m_vol.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
   {
      if (name == "dted_vol_record")
      {
         ossimContainerProperty* box = new ossimContainerProperty();
         box->setName(name);

         std::vector<ossimString> list;
         m_vol.getPropertyNames(list);

         std::vector< ossimRefPtr<ossimProperty> > propList;

         std::vector<ossimString>::const_iterator i = list.begin();
         while (i != list.end())
         {
            ossimRefPtr<ossimProperty> prop = m_vol.getProperty( (*i) );
            if (prop.valid())
            {
               propList.push_back(prop);
            }
            ++i;
         }
         box->addChildren(propList);
         result = box;
      }
   }
   if (result.valid() == false)
   {
      //ossimDtedHdr hdr(m_dtedFileStr, vol.stopOffset());
      if( m_hdr.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
      {
         if (name == "dted_hdr_record")
         {
            ossimContainerProperty* box = new ossimContainerProperty();
            box->setName(name);
            
            std::vector<ossimString> list;
            m_hdr.getPropertyNames(list);
            
            std::vector< ossimRefPtr<ossimProperty> > propList;
            
            std::vector<ossimString>::const_iterator i = list.begin();
            while (i != list.end())
            {
               ossimRefPtr<ossimProperty> prop = m_hdr.getProperty( (*i) );
               if (prop.valid())
               {
                  propList.push_back(prop);
               }
               ++i;
            }
            box->addChildren(propList);
            result = box;
         }
      }
      if (result.valid() == false)
      {
        // ossimDtedUhl uhl(m_dtedFileStr, hdr.stopOffset());
         if( m_uhl.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
         {
            if (name == "dted_uhl_record")
            {
               ossimContainerProperty* box = new ossimContainerProperty();
               box->setName(name);
               
               std::vector<ossimString> list;
               m_uhl.getPropertyNames(list);
               
               std::vector< ossimRefPtr<ossimProperty> > propList;
               
               std::vector<ossimString>::const_iterator i = list.begin();
               while (i != list.end())
               {
                  ossimRefPtr<ossimProperty> prop = m_uhl.getProperty( (*i) );
                  if (prop.valid())
                  {
                     propList.push_back(prop); 
                  }
                  ++i;
               }
               box->addChildren(propList);
               result = box;
            }
         }
         if (result.valid() == false)
         {
            //ossimDtedDsi dsi(m_dtedFileStr, uhl.stopOffset());
            if( m_dsi.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
            {
               if (name == "dted_dsi_record")
               {
                  ossimContainerProperty* box =
                     new ossimContainerProperty();
                  box->setName(name);
                  
                  std::vector<ossimString> list;
                  m_dsi.getPropertyNames(list);
                  
                  std::vector< ossimRefPtr<ossimProperty> > propList;
                  
                  std::vector<ossimString>::const_iterator i = list.begin();
                  while (i != list.end())
                  {
                     ossimRefPtr<ossimProperty> prop =
                        m_dsi.getProperty( (*i) );
                     if (prop.valid())
                     {
                        propList.push_back(prop);
                     }
                     ++i;
                  }
                  box->addChildren(propList);
                  result = box;
               }
            }
            if (result.valid() == false)
            {
               //ossimDtedAcc acc(m_dtedFileStr, dsi.stopOffset());
               if( m_acc.getErrorStatus() == ossimErrorCodes::OSSIM_OK )
               {
                  if (name == "dted_acc_record")
                  {
                     ossimContainerProperty* box =
                        new ossimContainerProperty();
                     box->setName(name);
                     
                     std::vector<ossimString> list;
                     m_acc.getPropertyNames(list);
                     
                     std::vector< ossimRefPtr<ossimProperty> > propList;
                     
                     ossimRefPtr<ossimProperty> prop = 0;
                     std::vector<ossimString>::const_iterator i =
                        list.begin();
                     while (i != list.end())
                     {
                        ossimRefPtr<ossimProperty> prop =
                           m_acc.getProperty( (*i) );
                        if (prop.valid())
                        {
                           propList.push_back(prop);
                        }
                        ++i;
                     }
                     box->addChildren(propList);
                     result = box;
                  }
               }
            }
         }
      }
   }

   return result;
}

void ossimDtedInfo::getPropertyNames(
   std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back(ossimString("dted_vol_record"));
   propertyNames.push_back(ossimString("dted_hdr_record"));
   propertyNames.push_back(ossimString("dted_uhl_record"));
   propertyNames.push_back(ossimString("dted_dsi_record"));
   propertyNames.push_back(ossimString("dted_acc_record"));
}


