//---
//
//  License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//---
// $Id$

#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_0.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_1.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_0.h>
#include <ossim/support_data/ossimNitfImageHeaderV2_1.h>
#include <ossim/support_data/ossimNitfTagFactoryRegistry.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/support_data/ossimRpfToc.h>

#include <iostream>
#include <iomanip>
#include <memory>

// Static trace for debugging
static ossimTrace traceDebug("ossimNitfFile:debug");

std::ostream& operator <<(std::ostream& out, const ossimNitfFile& data)
{
   return data.print(out);
}

std::ostream& ossimNitfFile::print(std::ostream& out,
                                   const std::string& prefix,
                                   bool printOverviews) const
{
   if(theNitfFileHeader.valid())
   {
      std::string pfx = prefix;
      pfx += "nitf.";
      theNitfFileHeader->print(out, pfx);
      
      ossim_int32 n = theNitfFileHeader->getNumberOfImages();
      for(ossim_int32 idx = 0; idx < n; ++idx)
      {
         ossimRefPtr<ossimNitfImageHeader> ih = getNewImageHeader(idx);
         if(ih)
         {
            bool printIt = true;
            
            if ( !printOverviews )
            {
               // Check the IMAG field.
               ossim_float64 imag;
               ih->getDecimationFactor(imag);
               if ( !ossim::isnan(imag) )
               {
                  if ( imag < 1.0)
                  {
                     printIt = false;
                  }
               }
            }

            if (printIt)
            {
               // Add our prefix onto prefix.
               std::string s = pfx;
               s += "image";
               s += ossimString::toString(idx).string();
               s += ".";
               
               ih->print(out, s);
            }
            ih = 0;
         }
      }

      //---
      // Check for RPF stuff:
      //---
      ossimNitfTagInformation info; 
      theNitfFileHeader->getTag(info, "RPFHDR");
      if(info.getTagName() == "RPFHDR")
      {
         // Open of the a.toc.
         ossimRefPtr<ossimRpfToc> toc = new ossimRpfToc;
         if ( toc->parseFile(getFilename()) ==
              ossimErrorCodes::OSSIM_OK )
         {
            pfx += "rpf.";
            toc->print(out, pfx, printOverviews);
         }
      }
      
   } // matches:  if(theNitfFileHeader.valid())

   return out;
   
}

std::ostream& ossimNitfFile::print(std::ostream& out,
                                   ossim_uint32 entryIndex,
                                   const std::string& prefix,
                                   bool printOverviews) const
{
   if(theNitfFileHeader.valid())
   {
      std::string pfx = prefix;
      pfx += "nitf.";
      theNitfFileHeader->print(out, pfx);
      
      if ( entryIndex < (ossim_uint32)theNitfFileHeader->getNumberOfImages() )
      {
         ossimRefPtr<ossimNitfImageHeader> ih = getNewImageHeader(entryIndex);
         if ( ih.valid() == true )
         {
            bool printIt = true;
            
            if ( !printOverviews )
            {
               // Check the IMAG field.
               ossim_float64 imag;
               ih->getDecimationFactor(imag);
               if ( !ossim::isnan(imag) )
               {
                  if ( imag < 1.0)
                  {
                     printIt = false;
                  }
               }
               
               //---
               // Now see if it's a cloud mask image.  Do not print
               // cloud mask images if the printOverviews is false.
               //---
               if ( printIt )
               {
                  if ( (ih->getCategory().trim(ossimString(" ")) ==
                        "CLOUD") &&
                       (ih->getRepresentation().trim(ossimString(" ")) ==
                        "NODISPLY") )
                  {
                     printIt = false;
                  }
               }
            }

            if (printIt)
            {
               // Add our prefix onto prefix.
               std::string s = pfx;
               s += "image";
               s += ossimString::toString(entryIndex).string();
               s += ".";
               
               ih->print(out, s);
            }
            ih = 0;
         }
      }

      //---
      // Check for RPF stuff:
      //---
      ossimNitfTagInformation info; 
      theNitfFileHeader->getTag(info, "RPFHDR");
      if(info.getTagName() == "RPFHDR")
      {
         // Open of the a.toc.
         ossimRefPtr<ossimRpfToc> toc = new ossimRpfToc;
         if ( toc->parseFile(getFilename()) ==
              ossimErrorCodes::OSSIM_OK )
         {
            pfx += "rpf.";
            toc->printHeader(out, pfx);
            toc->printTocEntry( out, entryIndex, pfx, printOverviews );
         }
      }
      
   } // matches:  if(theNitfFileHeader.valid())

   return out;
   
}

bool ossimNitfFile::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
{
   bool result = theNitfFileHeader.valid(); 
   
   if(theNitfFileHeader.valid())
   {
      theNitfFileHeader->saveState(kwl, prefix);
   }
   ossim_int32 n = theNitfFileHeader->getNumberOfImages();
   for(ossim_int32 idx = 0; idx < n; ++idx)
   {
      ossimRefPtr<ossimNitfImageHeader> ih = getNewImageHeader(idx);
      ossimString newPrefix = prefix + "image" + ossimString::toString(idx) + ".";
#if 1     
      ih->saveState(kwl, newPrefix);
#else
      if ( (ih->getCategory().trim(ossimString(" ")) !=
            "CLOUD") ||
          (ih->getRepresentation().trim(ossimString(" ")) !=
           "NODISPLY") )
      {
         ih->saveState(kwl, newPrefix);
      }
#endif
   }
   return result;
}

ossimNitfFile::ossimNitfFile()
   : theFilename(""),
     theNitfFileHeader(0)
{
}

ossimNitfFile::~ossimNitfFile()
{
   theNitfFileHeader = 0;
}

bool ossimNitfFile::parseFile(const ossimFilename& file)
{
   bool result = false;
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimNitfFile::parseFile: entered...\n";
   }
   
   //if( file.exists() )
   {
      // Open up a stream to the file.
      std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
         createIstream( file, std::ios_base::in | std::ios_base::binary );

      if ( str )
      {
         result = parseStream( file, *str );   
      }
      else
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "DEBUG ossimNitfFile::parseFile: "
               << "Could not open file:  " << file.c_str() << "\n";
         }
      }
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossimNitfFile::parseFile: "
         << "exit status: " << (result?"true":"false") << "\n";
   }

   return result;
}

bool ossimNitfFile::parseStream( const ossimFilename& file,
                                 ossim::istream& in )
{
   bool result = false;
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfFile::parseStream: DEBUG entered...\n";
   }
   
   if(theNitfFileHeader.valid())
   {
      theNitfFileHeader = 0;
   }

   char temp[10];
   in.read(temp, 9);
   in.seekg(0, std::ios::beg);
   temp[9] ='\0';
   
   theFilename = file;

   ossimString s(temp);
   if(s == "NITF02.00")
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG: NITF Version 2.0"
            << std::endl;
      }
      theNitfFileHeader = new ossimNitfFileHeaderV2_0;
   }
   else if ( (s == "NITF02.10") || (s == "NSIF01.00") )
   {
      if(traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG: NITF Version 2.1"
            << std::endl;
      }
      theNitfFileHeader = new ossimNitfFileHeaderV2_1;
   }
   else
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG ossimNitfFile::parseFile: "
            << "Not an NITF file!\n";
      }
   }

   if( theNitfFileHeader.valid() )
   {
      try
      {
         theNitfFileHeader->parseStream(in);
         result = true;
      }
      catch( const ossimException& e )
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimNitfFile::parseStream caught exception:\n"
               << e.what()
               << std::endl;
         }
      }
   }
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimNitfFile::parseStream: DEBUG\n"
         << "exit status: " << (result?"true":"false") << "\n";
   }
   return result;
}

const ossimNitfFileHeader* ossimNitfFile::getHeader() const
{
   return theNitfFileHeader.get();
}

ossimNitfFileHeader* ossimNitfFile::getHeader() 
{
   return theNitfFileHeader.get();
}

ossimIrect ossimNitfFile::getImageRect()const
{
   if(theNitfFileHeader.valid())
   {
      return theNitfFileHeader->getImageRect();
   }

   return ossimIrect(ossimIpt(0,0), ossimIpt(0,0));
}

ossimNitfImageHeader* ossimNitfFile::getNewImageHeader(
   ossim_uint32 imageNumber)const
{
   ossimNitfImageHeader* result = 0;

   if(theNitfFileHeader.valid())
   {
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->
         createIstream(theFilename, std::ios::in|std::ios::binary);
      
      if ( in )
      {
         result = getNewImageHeader( *in, imageNumber );
      }
   }
   
   return result;
}

ossimNitfImageHeader* ossimNitfFile::getNewImageHeader(
   ossim::istream& in, ossim_uint32 imageNumber)const
{
   ossimNitfImageHeader* result = 0;

   if(theNitfFileHeader.valid())
   {
      try // getNewImageHeader can throw exception on parse.
      {
         result = theNitfFileHeader->getNewImageHeader(imageNumber, in);
      }
      catch( const ossimException& e )
      {
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimNitfFile::getNewImageHeader caught exception:\n"
               << e.what()
               << std::endl;
         }
         result = 0;
      }
   }

   return result;
}

ossimNitfSymbolHeader* ossimNitfFile::getNewSymbolHeader(
   ossim_uint32 symbolNumber)const
{
   ossimNitfSymbolHeader* result = 0;

   if(theNitfFileHeader.valid())
   {
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->
         createIstream(theFilename, std::ios::in|std::ios::binary);

      if ( in )
      {
         result = getNewSymbolHeader( *in, symbolNumber );
      }
   }
   
   return result;
}

ossimNitfSymbolHeader* ossimNitfFile::getNewSymbolHeader(
   ossim::istream& in, ossim_uint32 symbolNumber)const
{
   ossimNitfSymbolHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      result = theNitfFileHeader->getNewSymbolHeader(symbolNumber, in);
   }
   return result;
}

ossimNitfLabelHeader* ossimNitfFile::getNewLabelHeader(
   ossim_uint32 labelNumber)const
{
   ossimNitfLabelHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->
         createIstream(theFilename, std::ios::in|std::ios::binary);
      
      if ( in )
      {
         result = getNewLabelHeader( *in, labelNumber );
      }
   }
   
   return result;
}

ossimNitfLabelHeader* ossimNitfFile::getNewLabelHeader(
   ossim::istream& in, ossim_uint32 labelNumber)const
{
   ossimNitfLabelHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      result = theNitfFileHeader->getNewLabelHeader(labelNumber, in);
   }
   return result;
}

ossimNitfTextHeader* ossimNitfFile::getNewTextHeader(
   ossim_uint32 textNumber)const
{
   ossimNitfTextHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->
         createIstream(theFilename, std::ios::in|std::ios::binary);

      if ( in )
      {
         result = getNewTextHeader( *in, textNumber );
      }
   }
   return result;
}

ossimNitfTextHeader* ossimNitfFile::getNewTextHeader(
   ossim::istream& in, ossim_uint32 textNumber)const
{
   ossimNitfTextHeader* result = 0;
   if(theNitfFileHeader.valid())
   {
      result = theNitfFileHeader->getNewTextHeader(textNumber, in);
   }
   return result;
}

ossimNitfDataExtensionSegment* ossimNitfFile::getNewDataExtensionSegment(
   ossim_uint32 dataExtNumber)const
{
   ossimNitfDataExtensionSegment* result = 0;
   if(theNitfFileHeader.valid())
   {
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->
         createIstream(theFilename, std::ios::in|std::ios::binary);

      if ( in )
      {
         result = getNewDataExtensionSegment( *in, dataExtNumber );
      }
   }
   return result;
}

ossimNitfDataExtensionSegment* ossimNitfFile::getNewDataExtensionSegment(
   ossim::istream& in, ossim_uint32 dataExtNumber)const
{
   ossimNitfDataExtensionSegment* result = 0;
   if(theNitfFileHeader.valid())
   {
      result = theNitfFileHeader->getNewDataExtensionSegment(dataExtNumber, in);
   }
   return result;
}

ossimString ossimNitfFile::getVersion()const
{
   if(theNitfFileHeader.valid())
   {
      return ossimString(theNitfFileHeader->getVersion());
   }
   
   return ossimString("");
}

ossimFilename ossimNitfFile::getFilename() const
{
   return theFilename;
}
