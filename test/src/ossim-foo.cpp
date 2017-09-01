//---
//
// License: MIT
//
// File: ossim-foo.cpp
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "git checkout -- ossimfoo.cpp" if
//        you want to keep your working repository up to snuff.
//
// $Id$
//---

// ossim includes:  These are here just to save time/typing...
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimCsvFile.h>
#include <ossim/base/ossimDate.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimObjectFactory.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimUrl.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/base/ossimEcefVector.h>
#include <ossim/base/ossim2dBilinearTransform.h>

#include <ossim/imaging/ossimNitfTileSource.h>
#include <ossim/imaging/ossimBrightnessContrastSource.h>
#include <ossim/imaging/ossimBumpShadeTileSource.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/imaging/ossimFusionCombiner.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageSourceFilter.h>
#include <ossim/imaging/ossimImageToPlaneNormalFilter.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimIndexToRgbLutFilter.h>
#include <ossim/imaging/ossimRectangleCutFilter.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimSFIMFusion.h>
#include <ossim/imaging/ossimTwoColorView.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimCcfHead.h>

#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/support_data/ossimWkt.h>
#include <ossim/support_data/ossimCcfInfo.h>

#include <ossim/base/ossimBlockStreamBuffer.h>
#include <ossim/base/ossimBlockIStream.h>

// Put your includes here:

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>
#include <time.h>
#include <random>

int main(int argc, char *argv[])
{
   int returnCode = 0;
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);
   ossim_int32 i;
   ossimString tempString1;
   ossimApplicationUsage* au = ap.getApplicationUsage();

   ossimString usageString = ap.getApplicationName();
   usageString += " ";


   au->setCommandLineUsage(usageString);
   au->addCommandLineOption("--file", "Specify a file to test reading");
   au->addCommandLineOption("--blocksize", "Specify a blockSize for the stream. Default 32k");
   au->addCommandLineOption("--random-read", "Enable random read");
   au->addCommandLineOption("--random-read-buf", "Enable random read buf size");
   au->addCommandLineOption("--read-buf-size", "Specify the buf size");
   au->addCommandLineOption("--number-of-reads", "Specify the number of reads");
   au->addCommandLineOption("--block-read", "Enable block read");

   ossimArgumentParser::ossimParameter stringParam1(tempString1);
   ossimFilename file;
   ossim_int64 blockSize=128*1024; 
   ossim_uint32 numberOfReads = 1024*1024; 
   ossim_uint32 readBufSize = 32; 
   bool randomReadFlag = false;
   bool randomReadBuf  = false;
   bool blockRead  = false;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<ossim_float64> randomBufSize;
    std::uniform_real_distribution<ossim_float64> randomReadOffset;

   if( ap.read("--blocksize", stringParam1) )
   {
      blockSize = ossimString(tempString1).toUInt32();
   }
   if( ap.read("--file", stringParam1) )
   {
      file = tempString1;
   }
   if( ap.read("--random-read") )
   {
      randomReadFlag = true;
   }
   if( ap.read("--random-read-buf") )
   {
      randomReadBuf = true;
   }
   if( ap.read("--read-buf-size", stringParam1) )
   {
      readBufSize = tempString1.toUInt32();
   }
   if( ap.read("--block-read") )
   {
      blockRead = true;
   }

   std::cout << "FILE =" << file << "=\n";
   if(file.empty())
   {
      std::cout << "WRITING?????\n";
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return 0;
   }
   try
   {
      if(randomReadFlag)
      {
         std::cout << "file size ==== " << file.fileSize() << "\n";
         randomReadOffset = std::uniform_real_distribution<double>(1, file.fileSize()-readBufSize);
      }
      if(randomReadBuf)
      {
         randomBufSize = std::uniform_real_distribution<ossim_float64>(1, readBufSize);
      }
      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->createIstream(file,
                                                             std::ios_base::in|std::ios_base::binary);
   
     if(in)
      {
#if 1
         std::vector<char> buf(readBufSize);
         std::shared_ptr<ossim::istream> inputStream;
         if(blockRead)
         {
            std::cout << "DOING BLOCK READ!!!!\n";
            inputStream = std::make_shared<ossim::BlockIStream>(in,blockSize);
         }
         else
         {
            std::cout << "DOING NON BLOCK READS\n";
            inputStream = in;
         }

         ossim_int64 bytesRead = 0;

         ossim_int32 count = 0;

         if(randomReadFlag)
         {
            ossim_int64 offset = 0;
            std::cout << "DOING RANDOM READ\n";
            while(count < numberOfReads)
            {
               offset = static_cast<ossim_int64>(randomReadOffset(mt));
               if(!inputStream->good())
               {
                  inputStream->clear();
               }
               inputStream->seekg(offset);
               ossim_uint32 tempSize = buf.size();
               if(randomReadBuf)
               {
                  tempSize = static_cast<ossim_uint32>(randomBufSize(mt));

               }
               
               inputStream->read(&buf.front(), tempSize);
               bytesRead += inputStream->gcount();

               if((count%(4*1024))==0)
               {
              //    std::cout <<"bytesRead = " << bytesRead << "\n";
               }
               ++count;
            }
         }
         else
         {
            std::cout << "DOING SEQUENTIAL READ\n";
            while((count < numberOfReads)&&
               (inputStream->good()))
            {
               ossim_uint32 tempSize = buf.size();
               if(randomReadBuf)
               {
                  tempSize = static_cast<ossim_uint32>(randomBufSize(mt));

               }
               
               inputStream->read(&buf.front(), tempSize);
               // std::cout << "tellg: " << inputStream->tellg() << "\n";
               bytesRead += inputStream->gcount();
               
               ++count;
            }
         }
         std::cout << "NUMBER OF READS = " << count << "\n";
         std::cout << "BYTES READ = " << bytesRead << "\n";
         std::cout << "TELLG: " << inputStream->tellg() << "\n";
      }
      #endif
      // Put your code here.
   }
   catch(const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
      returnCode = 1;
   }
   catch( ... )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossim-foo caught unhandled exception!" << std::endl;
      returnCode = 1;
   }

   return returnCode;
}
