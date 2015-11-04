//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: ossim-get-pixel-test.cpp
//
// Test code to spit out a pixel value at a given band, line, sample.
// 
//----------------------------------------------------------------------------
// $Id: ossim-get-pixel-test.cpp 22073 2013-01-04 21:27:39Z dburken $

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>

#include <iostream>
#include <vector>
using namespace std;

static void usage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);
   
   ossimApplicationUsage* au = ap.getApplicationUsage();
   
   // Set app name.
   au->setApplicationName(ap.getApplicationName());
   
   au->setDescription(ossimString("Returns pixel value at the given sample and line.\nNotes:\nBand, line and sample are zero based.\nUse an \"a\" for <band> if you want to see all bands.\n"));
   
   ossimString usageString = ap.getApplicationName();
   usageString += " [options] <imagefile> <band> <sample> <line>\n";
   au->setCommandLineUsage(usageString);
   
   au->addCommandLineOption("-e","<entry_index> Entry index to get pixel from.  Only significant with multi entry data only.");

   au->addCommandLineOption("--reader-prop","Passes a name=value pair to the reader(s) for setting it's property.  Any number of these can appear on the line.");
   
   // Write usage.
   au->write(ossimNotify(ossimNotifyLevel_INFO));
}

struct readerProp
{
   ossimString name;
   ossimString value;
};

/**
 * @brief Adds band to bandList returning true on success, false on range error
 * with error message to standard out.
 */
static bool addBand( ossim_uint32 band,
                     ossim_uint32 bandCount,
                     vector<ossim_uint32>& bandList );

int main(int argc, char *argv[])
{
   // Initialize:
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   // Argument check:
   if ( (ap.argc() < 5) || ap.read("-h") || ap.read("--help") )
   {
      usage(ap);
      return 1;
   }

   // Read options:
   
   // For argument parser reads.
   ossimString ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   // Entry index:
   ossim_uint32 entryIndex = 0;
   if ( ap.read("-e", sp1) )
   {
      entryIndex = ts1.toUInt32();
   }

   // Set the reader properties if any.
   std::vector<readerProp> readerProps;
   while(ap.read("--reader-prop", sp1))
   {
      std::vector<ossimString> splitArray;
      ts1.split(splitArray, "=");
      if(splitArray.size() == 2)
      {
         readerProp prop;
         prop.name = splitArray[0];
         prop.value = splitArray[1];
         readerProps.push_back(prop);
      }
   }

   ap.reportRemainingOptionsAsUnrecognized();

   // Check the input file:
   ossimFilename imageFile = argv[1];
   if ( !imageFile.exists() )
   {
      usage(ap);
      return 1;
   }

   // Open up an image handler.
   ossimRefPtr<ossimImageHandler> ih
      = (ossimImageHandler*)ossimImageHandlerRegistry::instance()->open( imageFile );
   if ( ih.valid() == false )
   {
      cerr << "Could not open: " << imageFile << endl;
      return 1;
   }

   if ( entryIndex )
   {
      if ( ih->setCurrentEntry( entryIndex ) == false )
      {
         cerr << "Invalid entry: " << entryIndex << endl;
         return 1;
      }
   }
   
   // Set the reader properties if any.
   if ( readerProps.size() )
   {
      std::vector<readerProp>::const_iterator i = readerProps.begin();
      while( i != readerProps.end() )
      {
         ossimRefPtr<ossimProperty> prop = new ossimStringProperty( (*i).name, (*i).value );
         ih->setProperty(prop);
         ++i;
      }
   }

   vector<ossim_uint32> bandList;
   ossimString bandString = argv[2];

   const ossim_uint32 BANDS = ih->getNumberOfOutputBands();
   if ( bandString.downcase() == "a" )
   {
      for ( ossim_uint32 band = 0; band < BANDS; ++band)
      {
         if ( addBand( band, BANDS, bandList ) == false )
         {
            return 1;
         }
      }
   }
   else
   {
      ossim_int32 band = bandString.toUInt32();
      if ( addBand( band, BANDS, bandList ) == false )
      {
         return 1;
      }
   }
      
   ossimIpt pt;
   pt.x = ossimString::toInt32(argv[3]);
   pt.y = ossimString::toInt32(argv[4]);
   
   ossimIrect rect(pt.x, pt.y, pt.x+1, pt.y+1);
   ossimRefPtr<ossimImageData> id = ih->getTile(rect, 0);

   if ( id.valid() )
   {
      cout << "image_file: " << imageFile
           << "\nsample:     " << pt.x
           << "\nline:       " << pt.y;

      vector<ossim_uint32>::const_iterator i = bandList.begin();
      while ( i != bandList.end() )
      {
         cout << "\nvalue[" << (*i) << "]: " << id->getPix( pt, (*i) );
         ++i;
      }
      cout << endl;
   }
   else
   {
      cerr << "getTile request failed!" << endl;
      return 1;
   }
   
   return 0;
}

bool addBand( ossim_uint32 band, ossim_uint32 bandCount, vector<ossim_uint32>& bandList )
{
   bool result = true;
   if ( band < bandCount )
   {
      bandList.push_back(band);
   }
   else
   {
      cerr << "Band number " << band << " is out of range!" << endl;
      result = false;
   }
   return result;
}
