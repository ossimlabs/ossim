#include <cstdlib> /* for exit */
#include <iostream>

#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>


int main(int argc, char *argv[])
{
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help", "Shows help");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] ");
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   
   if(argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }
   ossimString extensions = "png,gif,j2k,jp2,jpg,jpeg,tif,tiff,nitf,ntf,ccf,til,img,toc,rpf,doq,doqq,dt0,dt1,dt2,dt3,dt4,dt5,dt6,hgt,dem,fst,ras";
   ossimString mimeTypes = "image/gif,image/jpg,image/png,image/jpx,image/jp2,image/jpeg,image/tiff,image/nitf,image/ntf,image/doq,image/doqq,image/dted,image/hgt,image/dem";
   std::vector<ossimString> extensionArray;
   std::vector<ossimString> mimeArray;

   extensions.split(extensionArray, ",");
   mimeTypes.split(mimeArray, ",");
   ossim_uint32 idx = 0;
   for(idx = 0; idx < extensionArray.size(); ++idx)
   {
      ossimImageWriterFactoryBase::ImageFileWriterList result;
      ossimImageWriterFactoryRegistry::instance()->getImageFileWritersBySuffix(result, extensionArray[idx]);
      
      std::cout << "extension  " << extensionArray[idx] << ": ";
      if(!result.empty())
      {
         for(ossim_uint32 resultIdx = 0; resultIdx < result.size(); ++resultIdx)
         {
            std::cout << result[resultIdx]->getClassName() << (((resultIdx+1)==result.size())?"":", ");
         }
      }
      std::cout << "\n";
   }
   for(idx = 0; idx < mimeArray.size(); ++idx)
   {
      ossimImageWriterFactoryBase::ImageFileWriterList result;
      ossimImageWriterFactoryRegistry::instance()->getImageFileWritersByMimeType(result, mimeArray[idx]);
      
      std::cout << "mimeType  " << mimeArray[idx] << ": ";
      if(!result.empty())
      {
         for(ossim_uint32 resultIdx = 0; resultIdx < result.size(); ++resultIdx)
         {
            std::cout << result[resultIdx]->getClassName() << (((resultIdx+1)==result.size())?"":", ");
         }
      }
      std::cout << "\n";
   }
   if(argc > 1)
   {
      for(idx = 1; static_cast<ossim_int32>(idx) < argc; ++idx)
      {
         ossimRefPtr<ossimImageFileWriter> writer = ossimImageWriterFactoryRegistry::instance()->createWriter(ossimFilename(argv[idx]));
         
         if(writer.valid())
         {
            std::cout << writer->getClassName() << ": Opened by extension for file " << argv[idx] << std::endl;
         }
      }
   }
   
   return 0;
}
