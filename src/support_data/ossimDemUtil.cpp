//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License: MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Ken Melero
//         Orginally written by Jamie Moyers (jmoyers@geeks.com)
//         Adapted from the package KDEM.
// Description: This class provides some simple utilities for DEMs.
//
//********************************************************************
// $Id$

#include <ossim/support_data/ossimDemUtil.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <cstring>
#include <iostream>
#include <memory>

static const char DEM_TYPE_KW[] = "dem_type";
static const char USGS_DEM_KW[] = "usgs_dem";

bool ossimDemUtil::isUsgsDem(const ossimFilename& file)
{
   bool result = false;

   ossimString ext = file.ext();
   ext.downcase();
   if (ext == "dem")
   {
      result = true;
   }
   else
   {
      // Look for file.omd
      ossimFilename kwlFile = file;
      kwlFile.setExtension("omd");
      if (! kwlFile.exists() )
      {
         kwlFile.setExtension("OMD");
      }
      
      if ( kwlFile.exists() )
      {
         ossimKeywordlist kwl(kwlFile);
         if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_OK)
         {
            const char* lookup = kwl.find(DEM_TYPE_KW);
            if (lookup)
            {
               ossimString s = lookup;
               s.downcase();
               if (s == USGS_DEM_KW)
               {
                  result = true;
               }
            }
         }
      }
   }
   
   if ( result )
   {
      
      // Open up the file for reading.
      std::shared_ptr<ossim::istream> is = ossim::StreamFactoryRegistry::instance()->
         createIstream(file, std::ios_base::in | std::ios_base::binary);

   }
      
   return result;
}

bool ossimDemUtil::isUsgsDem(std::shared_ptr<ossim::istream> str, 
                             const std::string& connectionString)
{
   bool result = false;
   ossimFilename tempFile = connectionString;
   ossimString ext = tempFile.ext();
   ext.downcase();
   if (ext == "dem")
   {
      result = true;
   }
   else
   {
      // Look for file.omd
      ossimFilename kwlFile = tempFile;
      kwlFile.setExtension("omd");
      if (! kwlFile.exists() )
      {
         kwlFile.setExtension("OMD");
      }
      
      if ( kwlFile.exists() )
      {
         ossimKeywordlist kwl(kwlFile);
         if (kwl.getErrorStatus() == ossimErrorCodes::OSSIM_OK)
         {
            const char* lookup = kwl.find(DEM_TYPE_KW);
            if (lookup)
            {
               ossimString s = lookup;
               s.downcase();
               if (s == USGS_DEM_KW)
               {
                  result = true;
               }
            }
         }
      }
   }
   if(result&&str&&str->good())
   {
      //---
      // SPECIAL HACK (drb):
      // Check some bytes and make sure there is no binary data.
      // There are files out there with .dem extension that are binary
      // rasters.
      //---
      ossim_uint8* ubuf = new ossim_uint8[512];
      str->read((char*)ubuf, 512);
      for (int i = 0; i < 512; ++i)
      {
         if (ubuf[i] > 0x7f)
         {
            result = false;
            break;
         }
      }
      delete [] ubuf;
      ubuf = 0;
   }
   else
   {
      result = false;
   }

   return result;   
}

bool ossimDemUtil::getRecord(ossim::istream& s, std::string& strbuf, long reclength)
{
   char* buf = new char[reclength + 1];

   bool flag = ossimDemUtil::getRecord(s,buf,reclength);
   if (flag == true)
      strbuf = buf;

   delete [] buf;

   return flag;
}

bool ossimDemUtil::getRecord(ossim::istream& s, char* buf, long reclength)
{
   // buf is assumed to be at least reclength+1 in size.

   if (!s)
      return false;

   long curpos = 0;
   buf[curpos] = s.get();
   while ((buf[curpos] != EOF) && 
          (buf[curpos] != '\n') &&
          (curpos < reclength-1))
   {
      curpos++;
      buf[curpos] = s.get();
   }
   buf[curpos] = '\0';

   if (s.peek() == '\n')
      s.get();

   return true;

}

bool
ossimDemUtil::getDouble(std::string const& strbuf,
                        long const startpos,
                        long const width,
                        double& val)
{
   if ((startpos + width - 1) > (long)(strbuf.length()))
      return false;

   // Convert FORTRAN 'D' exponent indicator to 'E'.
   std::string tempbuf(strbuf.substr(startpos,width));
   for (unsigned int i = 0; i < tempbuf.length(); i++)
      if (tempbuf[i] == 'D')
         tempbuf[i] = 'E';

   val = atof(tempbuf.c_str());
   return true;
}

long ossimDemUtil::getLong(char* const strbuf, // string to extract long from
                           long const startpos,  // starting position of field
                           long const width)     // width of field
{
   char temp[1024];
   std::strncpy(temp,strbuf+startpos,width);
   temp[width] = '\0';
   return atol(temp);
}
