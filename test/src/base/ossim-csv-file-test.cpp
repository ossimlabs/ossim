#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimCsvFile.h>
#include <iostream>
#include <iterator>
int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   
   if(argc == 1)
   {
      std::cout << "Usage: " << argv[0] << " <csv_file> " << std::endl;
      return 0;
   }
   ossimRefPtr<ossimCsvFile> csvFile = new ossimCsvFile;
   ossimRefPtr<ossimCsvFile::Record> record;
   if(csvFile->open(ossimFilename(argv[1])))
   {
      if(csvFile->readHeader())
      {
         std::copy(csvFile->fieldHeaderList().begin(),
                   csvFile->fieldHeaderList().end(),
                   std::ostream_iterator<ossimString>(std::cout, " "));
         while((record = csvFile->nextRecord()).valid())
         {
            // you can print by index or by header field name
            //
            std::cout << (*record)[0] << std::endl;         
            std::cout << (*record)[csvFile->fieldHeaderList()[0]] << std::endl;         
         }
      }
   }
   return 0;
}
