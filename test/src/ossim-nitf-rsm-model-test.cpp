//---
// File: ossim-nitf-rsm-model-test.cpp
//
// Description: Test app for ossimNitfRsmModel class.
// 
//---


#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/projection/ossimNitfRsmModel.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimRsmModel.h>
#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/support_data/ossimNitfRsmecaTag.h>
#include <ossim/support_data/ossimNitfRsmidaTag.h>
#include <ossim/support_data/ossimNitfRsmpcaTag.h>
#include <ossim/support_data/ossimNitfRsmpiaTag.h>
#include <ossim/support_data/ossimNitfTagFactoryRegistry.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static ossimRefPtr<ossimNitfRegisteredTag> getTag( const std::string& tagLine );
static ossimRefPtr<ossimNitfRsmModel> getModel( const ossimFilename& file );
static ossimRefPtr<ossimNitfRsmModel> getModelFromExtFile( const ossimFilename& file );
static ossimRefPtr<ossimNitfRsmModel> getModelFromImage( const ossimFilename& file );
static void testGpts( ossimRefPtr<ossimNitfRsmModel>& model, const ossimKeywordlist& kwl );
static void testIpts( ossimRefPtr<ossimNitfRsmModel>& model, const ossimKeywordlist& kwl );

int main(int argc, char *argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   if ( argc == 3 )
   {
      ossimFilename file = argv[1];
      ossimKeywordlist kwl;
      if ( kwl.addFile( argv[2] ) )
      {
         ossimRefPtr<ossimNitfRsmModel> model = getModel( file );
         if ( model.valid() )
         {
            testGpts( model, kwl );
            testIpts( model, kwl );
         }
         else
         {
            cerr << "Could not create model!" << endl;
         }
      }
      else
      {
         cerr << "Could not open keyword list: " << argv[2] << endl;
      }  
   }
   else
   {
      cout << "\n" << argv[0] << " <input-test-file> <options.kwl>"
           << "\ninput-test-file can be an input test file or a test.ext file.\n"
           << "\nPrints out debug info for model testing.."
           << "\ngpts test:"
           << "\nmodel->worldToLineSample(...), model->lineSampleToWorld(...)"
           << "\nipts test:"
           << "\nmodel->lineSampleHeightToWorld(...), model->worldToLineSample(...)"
           << "\noptions.kwl format example:\n"

           << "\ngtest_id0: E123456\n"
           << "gtest_gpt0: ( lat, lon, hgt )\n"

           << "gtest_gpt_id1: E123457\n"
           << "gtest_gpt1: ( lat, lon, hgt )\n"

           << "ggtest_pt_id2: E123458\n"
           << "gtest_gpt2: ( lat, lon, hgt )\n"

           << "gtest_gpt_id3: E123459\n"
           << "gtest_gpt3: ( lat, lon, hgt )\n"

           << "gtest_gpt_id4: E123459\n"
           << "gtest_pt4: ( lat, lon, hgt )\n"

           << "\nitest_height_units: feet\n"
         
           << "itest_id0: E123456\n"
           << "itest_ipt0: ( sample, line )\n"
           << "itest_hgt0: hgt\n"
         
           << "itest_id1: E123457\n"
           << "itest_ipt1: ( x, y )\n"
           << "itest_hgt1: hgt\n"

           << "itest_id2: E123458\n"
           << "itest_ipt2: ( x, y )\n"
           << "itest_hgt2: hgt\n"

           << "itest_id3: E123459\n"
           << "itest_ipt3: ( x, y )\n"
           << "itest_hgt3: hgt\n"

           << "itest_id4: E123459\n"
           << "itest_ipt4: ( x, y )\n"
           << "itest_hgt4: hgt\n"

           << "\nNotes:\n"
           << "* itest_hgt default = meters if units not specified.\n"
           << "* gpt0: ( lat, lon, hgt ) \"hgt\" is in meters.\n"
           << "* All test height output is in meters.\n"
           << "* \"rtd\"=round trip delta.\n"
           << endl;
   }

   return 0;
}


ossimRefPtr<ossimNitfRegisteredTag> getTag( const std::string& tagLine )
{
   ossimRefPtr<ossimNitfRegisteredTag> result = 0;

   if ( tagLine.size() > 6 )
   {
      ossimString tagName = tagLine.substr(0, 6);
      result = ossimNitfTagFactoryRegistry::instance()->create( tagName );
      if ( result.valid() )
      {
         cout << "tag_name: " << tagName << "\n";
         
         istringstream is( tagLine );
         if ( is.good() )
         {
            is.seekg( 6 );
            char tagLength[6];
            tagLength[5] = '\0';
            is.read( tagLength, 5 );
            cout << "tag_length: " << tagLength << "\n";
            result->parseStream( is );

            result->print( cout, std::string("") );
         }
      }
      else
      {
         cerr << "unhandled_tag: " << tagName << endl;
      }
   }
   return result;
}

ossimRefPtr<ossimNitfRsmModel> getModel( const ossimFilename& file )
{
   ossimRefPtr<ossimNitfRsmModel> result = 0;
   if ( file.size() )
   {
      // Get downcased extension:
      std::string ext = file.ext().downcase().string();
      
      if ( ext == "ext" )
      {
         result = getModelFromExtFile( file );
      }
      else if ( ( ext == "ntf" ) || ( ext == "nitf" ) )
      {
         result = getModelFromImage( file );
      }
   }
   return result;
}

ossimRefPtr<ossimNitfRsmModel> getModelFromExtFile( const ossimFilename& file )
{
   ossimRefPtr<ossimNitfRsmModel> result = 0;

   // Test file with newline separated nitf tags only.
   ifstream is( file.c_str() );
   if ( is.good() )
   {
      const std::string RSMECA_TAG = "RSMECA";
      const std::string RSMIDA_TAG = "RSMIDA";
      const std::string RSMPCA_TAG = "RSMPCA";
      const std::string RSMPIA_TAG = "RSMPIA";
      
      ossimRefPtr<ossimNitfRsmecaTag> rsmecaTag = 0;
      ossimRefPtr<ossimNitfRsmidaTag> rsmidaTag = 0;
      ossimRefPtr<ossimNitfRsmpcaTag> rsmpcaTag = 0; 
      ossimRefPtr<ossimNitfRsmpiaTag> rsmpiaTag = 0;        
      std::vector< ossimRefPtr<ossimNitfRegisteredTag> > tags;
      
      while ( is.good() )
      {
         string tagLine;
         std::getline( is, tagLine );
         
         ossimRefPtr<ossimNitfRegisteredTag> tag = getTag( tagLine );
         if ( tag.valid() )
         {
            tags.push_back( tag );
            if ( tag->getTagName() ==  RSMECA_TAG )
            {
               rsmecaTag = dynamic_cast<ossimNitfRsmecaTag*>( tag.get() );
            }
            else if ( tag->getTagName() == RSMIDA_TAG )
            {
               rsmidaTag = dynamic_cast<ossimNitfRsmidaTag*>( tag.get() );
            }
            else if ( tag->getTagName() == RSMPCA_TAG )
            {
               rsmpcaTag = dynamic_cast<ossimNitfRsmpcaTag*>( tag.get() );
            }
            else if ( tag->getTagName() == RSMPIA_TAG )
            {
               rsmpiaTag = dynamic_cast<ossimNitfRsmpiaTag*>( tag.get() );
            }
         }
      }
      
      if ( rsmecaTag.valid() && rsmidaTag.valid() &&
           rsmpcaTag.valid() && rsmpiaTag.valid() )
      {
         result = new ossimNitfRsmModel();
         if ( result->initializeModel( rsmecaTag.get() ) )
         {
            if ( result->initializeModel( rsmidaTag.get() ) )
            {
               if ( result->initializeModel( rsmpcaTag.get() ) )
               {
                  if ( result->initializeModel( rsmpiaTag.get() ) )
                  {
                     cout << "Initialize from ext file success!" << endl;
                  }
                  else
                  {
                     result = 0;
                     cerr << "ossimNitfRsmModel::initializeModel(" << RSMPIA_TAG
                          << ") failed!" << endl;
                  }
               }
               else
               {
                  result = 0;
                  cerr << "ossimNitfRsmModel::initializeModel(" << RSMPCA_TAG
                       << ") failed!" << endl;
               }
            }
            else
            {
               result = 0;
               cerr << "ossimNitfRsmModel::initializeModel(" << RSMIDA_TAG
                    << ") failed!" << endl;
            }
         }
         else
         {
            result = 0;
            cerr << "ossimNitfRsmModel::initializeModel(" << RSMECA_TAG
                 << ") failed!" << endl;
         }
      }
      else
      {
         // At least one RSM tag was not found.
         if ( rsmecaTag.valid() == false )
         {
            cerr << RSMECA_TAG << " not found!" << endl;
         }
         if ( rsmidaTag.valid() == false )
         {
            cerr << RSMIDA_TAG << " not found!" << endl;
         }
         if ( rsmpcaTag.valid() == false )
         {
            cerr << RSMPCA_TAG << " not found!" << endl;
         }
         if ( rsmpiaTag.valid() == false )
         {
            cerr << RSMPIA_TAG << " not found!" << endl;
         }
      }
   }
   else
   {
      cerr << "Could not open: " << file << endl;
   }
         
   return result;
   
} // End: getModelFromExtFile(...)


ossimRefPtr<ossimNitfRsmModel> getModelFromImage( const ossimFilename& file )
{
   ossimRefPtr<ossimNitfRsmModel> result = 0;

   ossimRefPtr<ossimImageHandler> ih =
      ossimImageHandlerRegistry::instance()->open(file,
                                                  true,   // try suffix first
                                                  false); // open overview
   if ( ih.valid() )
   {
      ossimRefPtr<ossimImageGeometry> geom = ih->getImageGeometry();
      if ( geom.valid() )
      {
         ossimRefPtr<ossimProjection> proj = geom->getProjection();
         if ( proj.valid() )
         {
            result = dynamic_cast<ossimNitfRsmModel*>( proj.get() );
         }
      }
   }
   
   return result;  
}

void testGpts( ossimRefPtr<ossimNitfRsmModel>& model, const ossimKeywordlist& kwl )
{
   if ( model.valid() )
   {
      cout << std::setfill(' ') << setiosflags(ios::left);
      
      const std::string  ID_KW  = "gtest_id";
      const std::string  GPT_KW = "gtest_gpt";
      const ossim_uint32 POINTS = kwl.numberOf( ID_KW.c_str() );
      
      cout << "\ngtest begin ********************************\n\n"
           << "number_of_points_world_points: " << POINTS << "\n";
      
      ossim_uint32 foundPts = 0;
      ossim_uint32 i = 0;
      
      std::string key;
      std::string value;
      
      while ( foundPts < POINTS )
      {
         // ID:
         key = ID_KW + ossimString::toString( i ).string();
         value = kwl.findKey( key );
         if ( value.size() )
         {
            cout << "gtest_id" << std::setw(6) << i << ":  " << value << "\n";
         }
      
         // World point :
         key = GPT_KW + ossimString::toString( i ).string();
         value = kwl.findKey( key );
      
         if ( value.size() )
         {
            ossimDpt ipt; // image point
            ossimGpt wpt; // world point
            ossimGpt rtp; // round trip point
            ossimDpt rtd; // round trip delta;
         
            wpt.toPoint( value );

            cout << "gtest_gpt" << std::setw(5) << i << ":  " << wpt << "\n";

            model->worldToLineSample( wpt, ipt );

            if ( wpt.hasNans() == false )
            {
               model->lineSampleHeightToWorld( ipt, wpt.hgt, rtp );
            
               rtd.x = wpt.lon - rtp.lon;
               rtd.y = wpt.lat - rtp.lat;
            
               cout << "gtest_ipt" << std::setw(5) << i << ":  " << ipt << "\n"
                    << "gtest_rtp" << std::setw(5) << i << ":  " << rtp << "\n"
                    << "gtest_rtd" << std::setw(5) << i << ":  " << rtd << "\n\n";  
            }
            else
            {
               cerr << "model->worldToLineSample(...) result has nans!\n"
                    << wpt << endl;
            }
         
            ++foundPts;
         }
      
         ++i;
      
         if ( i > POINTS+100 )
         {
            break;
         }
      }

      cout << "\ngtest end **********************************\n\n";
   }
   
} // End: testGpts

void testIpts( ossimRefPtr<ossimNitfRsmModel>& model, const ossimKeywordlist& kwl )
{
   if ( model.valid() )
   {
      cout << std::setfill(' ') << setiosflags(ios::left);

      const std::string  ID_KW      = "itest_id";
      const std::string  IPT_KW     = "itest_ipt";
      const std::string  IPT_HGT_KW = "itest_hgt";
      const ossim_uint32 POINTS     = kwl.numberOf( ID_KW.c_str() );
   
      // Test data height values can be in feet.
      ossimUnitType heightUnits = OSSIM_METERS;
      std::string key = "itest_height_units";
      std::string value = kwl.findKey( key );
      if ( value.size() )
      {
         cout << key << ": " << value << "\n";
         if ( value == "feet" )
         {
            heightUnits = OSSIM_FEET;
         }
      }
      cout << "\nitest begin ********************************\n\n"
           << "number_of_line_sample_points: " << POINTS << "\n";
      
      ossim_uint32 foundPts = 0;
      ossim_uint32 i = 0;

      while ( foundPts < POINTS )
      {
         // ID:
         key = ID_KW + ossimString::toString( i ).string();
         value = kwl.findKey( key );
         if ( value.size() )
         {
            cout << "itest_id" << std::setw(6) << i << ":  " << value << "\n";
         }
      
         // Image point, sample, line:
         key = IPT_KW + ossimString::toString( i ).string();
         value = kwl.findKey( key );
      
         if ( value.size() )
         {
            ossimDpt ipt; // image point
            ossimGpt wpt; // world point
            ossimDpt rtp; // round trip point
            ossimDpt rtd; // round trip delta;
         
            ipt.toPoint( value );
         
            cout << "itest_ipt" << std::setw(5) << i << ":  " << value << "\n";
         
            // Get the height above ellipsoid:
            ossim_float64 hgt = 0.0;
            key = IPT_HGT_KW + ossimString::toString( i ).string();
            value = kwl.findKey( key );
            if ( value.size() )
            {
               ossimString os ( value );
               hgt = os.toFloat64();
            
               if ( heightUnits == OSSIM_FEET )
               {
                  hgt *= MTRS_PER_FT;
               }
            }
            else
            {
               cerr << "missing height above ellipsoid for point!  Using 0.0."
                    << endl;
            }
         
            cout << "itest_hgt" << std::setw(5) << i << ":  " << value << "\n";
         
            model->lineSampleHeightToWorld( ipt, hgt, wpt );
            if ( wpt.hasNans() == false )
            {
               model->worldToLineSample( wpt, rtp );
            
               rtd = ipt - rtp;
            
               cout << "itest_wpt" << std::setw(5) << i << ":  " << wpt << "\n"
                    << "itest_rtp" << std::setw(5) << i << ":  " << rtp << "\n"
                    << "itest_rtd" << std::setw(5) << i << ":  " << rtd << "\n\n";
            }
            else
            {
               cerr << "model->worldToLineSample(...) result has nans!\n"
                    << wpt << endl;
            }
         
            ++foundPts;
         }
      
         ++i;
      
         if ( i > POINTS+100 )
         {
            break;
         }
      }

      cout << "\ntestIpts end **********************************\n\n";
   }
   
} // End: testIpts
