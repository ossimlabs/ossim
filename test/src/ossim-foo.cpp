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
#include <ossim/base/ossimTimer.h>
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
#include <ossim/support_data/ImageHandlerState.h>

#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/support_data/ossimWkt.h>

#include <ossim/base/Barrier.h>
#include <ossim/base/Block.h>
#include <ossim/base/Thread.h>
#include <ossim/base/ItemCache.h>
#include <ossim/support_data/TiffHandlerState.h>
#include <ossim/support_data/ImageHandlerStateRegistry.h>
// Put your includes here:

// System includes:
#include <cmath>
#include <memory>
#include <sstream>
#include <iostream>



#if 0
namespace ossim{
   class JsonValue
   {
   public:
   };

   class JsonString : public JsonValue
   {
   public:
   };

   class JsonArray : public JsonValue
   {
   public:
   };

   class JsonObject : public JsonValue
   {
   public:
      typedef std::map<ossimString, std::shared_ptr<JsonValue> > ObjectMap;

      std::shared_ptr<const JsonValue> operator [] (const ossimString& key)const
      {
         return 0;
      }
      void add(const ossimString& key, std::shared_ptr<JsonValue> value)
      {
         m_map.insert(std::make_pair(key, value));
      }

   protected:
      ObjectMap m_map;      
   };

   class Json
   {
   public:
      bool parse(const ossimFilename& file);
      bool parse(ossim::istream& in);
      bool parse(const ossimString& buf);

      bool parseObject(ossim::istream& in);
      bool parseNumber(ossim::istream& in);
      bool parseArray(ossim::istream& in);
      bool parseValue(ossim::istream& in);
      bool parseString(ossim::istream& in);
      bool parseStringValue(ossim::istream& in);
   protected:
      void skipWhitespace(ossim::istream& in)const;
      //std::map<std::string, std::shared_ptr<JsonValue> obj;
   };
   void Json::skipWhitespace(ossim::istream& in)const
   {
      int c = in.peek();
      while( !in.fail() &&
            ( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ) )
      {
         in.ignore(1);
         c = in.peek();
      }
   }

   bool Json::parse(ossim::istream& in)
   {
      return parseObject(in);
   }
   bool Json::parse(const ossimString& buf)
   {
      std::istringstream in(buf.c_str());

      return parse(in);
   }
   bool Json::parseStringValue(ossim::istream& in)
   {
      skipWhitespace(in);
      if(in.good())
      {
         if(parseString(in))
         {
            skipWhitespace(in);
            if(in.good())
            {
               if(in.peek() == ':')
               {
                  std::cout << ":";
                  in.ignore();
                  if(parseValue(in))
                  {

                  }
                  else
                  {
                     return false;
                  }
               }
            }
         }
         else
         {
            return false;
         }
      }

      return in.good();
   }
   bool Json::parseString(ossim::istream& in)
   {
      skipWhitespace(in);
      if(in.peek() == '"')
      {
         std::cout << "\"";
         in.ignore();
         while(in.good())
         {
            int c = in.peek();
            if(c == '\\')
            {
               in.ignore();
               c = in.peek();
               switch(c)
               {
                  case '"':
                  {
                     c = '"';
                     break;
                  }
                  case 'b':
                  {
                     c = '\b';
                     break;
                  }
                  case '\\':
                  {
                     c = '\\';
                     break;
                  }
                  case '/':
                  {
                     c = '/';
                     break;
                  }
                  case 'f':
                  {
                     c = '\f';
                     break;
                  }
                  case 'n':
                  {
                     c = '\n';
                     break;
                  }
                  case 'r':
                  {
                     c = '\r';
                     break;
                  }
                  case 't':
                  {
                     c = '\t';
                     break;
                  }
                  case 'u':
                  {
                     return false;
                     break;
                  }
                  default:
                  {
                     return false;
                  }
               }
            }
            in.ignore();
            std::cout << (char)c;
            if(c == '"')
            {
               return true;
            }
         }
      }
      return false;
   }
   bool Json::parseNumber(ossim::istream& in)
   {
      bool done=false;
      bool afterDecimal = false;
      skipWhitespace(in);
      int c = in.peek();
      int sign = 1;
      if(c == '-')
      {
         // starts with minus
         std::cout << (char)c;
         in.ignore();
         c = in.peek();
         sign = -1;
      }
      while(in.good() && !done)
      {
         if(std::isdigit(c))
         {
            std::cout << (char)c;
            in.ignore();
         }
         else if(c == '.' &&!afterDecimal)
         {
            std::cout << (char)c;
            in.ignore();
            if(!std::isdigit(in.peek()))
            {
               return false;
            }
            afterDecimal = true;
         }
         else if(c == 'e' || c == 'E')
         {
            std::cout << (char)c;
            in.ignore();
            c = in.peek();
            afterDecimal = true;
            if(c == '+')
            {
               std::cout << (char)c;
               in.ignore();
            }
            else if(c == '-')
            {
               std::cout << (char)c;
               in.ignore();
            }
            else if(!std::isdigit(c) )
            {
               return false;
            }
         }
         else
         {
            done = true;
         }
         c = in.peek();
      }
      return in.good();
   }
   bool Json::parseValue(ossim::istream& in)
   {
      skipWhitespace(in);

      int c = in.peek();

      if(c == '"')
      {
         return parseString(in);
      }
      else if(c == '{')
      {
         return parseObject(in);
      }
      else if(c == 't')
      {
         char buf[4];
         in.read(buf, 4);
         if(!in.good()||(ossimString(buf, buf+4)!="true"))
         {
            return false;
         }
         std::cout << "true";
         return in.good();
      }
      else if(c == 'f')
      {
         char buf[5];
         in.read(buf, 5);
         if(!in.good()||(ossimString(buf, buf+5)!="false"))
         {
            return false;
         }
         std::cout << "false";
         return in.good();
      }
      else if(c == 'n')
      {
         char buf[4];
         in.read(buf, 4);
         if(!in.good()||(ossimString(buf, buf+4)!="null"))
         {
            return false;
         }
         std::cout << "null";

         return in.good();
      }
      else if(c=='-' || std::isdigit(c))
      {
         parseNumber(in);
         return in.good();
      }
      else if(c == '[')
      {
         if(parseArray(in))
         {
            return in.good();
         }
      }
      return false;
   }
   bool Json::parseArray(ossim::istream& in)
   {
      skipWhitespace(in);
      int c = in.peek();
      if(c == '[')
      {
         std::cout << "[";
         in.ignore();
         c = in.peek();

         if(c == ']')
         {
            in.ignore();
            std::cout << "]";
            return true;
         }
         else
         {
            bool finished = false;

            do{
               if(parseValue(in))
               {
                  skipWhitespace(in);
                  c = in.peek();
                  in.ignore();
                  if(c == ']')
                  {
                     finished = true;
                     std::cout << "]";
                  }
                  else if(c != ',')
                  {
                     return false;
                  }
                  else
                  {
                     std::cout << ",";

                  }
               }

            }while(in.good()&&!finished);
         }
      }
      return in.good();
   }
   bool Json::parseObject(ossim::istream& in)
   {
      skipWhitespace(in);
      if(!in.good())
      {
         return false;
      }
      else if(in.peek() == '{')
      {
         std::cout << "{";
         in.ignore();
         skipWhitespace(in);
         if(in.peek() == '}')
         {
            std::cout << "}";
            in.ignore();
            return true;
         }
         else
         {
            bool keepGoing = true;
            do{
               if(!parseStringValue(in))
               {
                  return false;
               }
               skipWhitespace(in);
               if(in.peek()!=',')
               {
                  keepGoing = false;
               }
               else
               {
                  std::cout << ",";
                  in.ignore();
               }
            }while(in.good() && (keepGoing));

            if(!in.good())
            {
               return false;
            }
         }
         skipWhitespace(in);
      }
      else
      {
         skipWhitespace(in);
      }
      std::cout << (char)in.peek();
      if(in.peek() == '}')
      {
         in.ignore();
         return true;
      }
      
      return false;
   }

}
#endif
int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   try
   {
#if 0
     ossim2dBilinearTransform lsb(ossimDpt(322412111.000000000000000, 86442174.000000000000000),
                                  ossimDpt(322412112.000000000000000, 86442174.000000000000000),
                                  ossimDpt(322412112.000000000000000, 86442175.000000000000000),
                                  ossimDpt(322412111.000000000000000, 86442175.000000000000000),
                                  ossimDpt(31037.499999818603101, 761.499999969978376),
                                  ossimDpt(31041.499999947303877, 761.499999969978376),
                                  ossimDpt(31041.499999947303877, 765.499999923811401),
                                  ossimDpt(31037.499999818603101, 765.499999923811401)
                                  );
#endif
 #if 1
      ossimTimer::Timer_t t1 = ossimTimer::instance()->tick();
      ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
      ossimTimer::Timer_t t2 = ossimTimer::instance()->tick();

      std::cout << "Open 1: " << ossimTimer::instance()->delta_s(t1,t2) << "\n";

      t1 = ossimTimer::instance()->tick();
      ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
      t2 = ossimTimer::instance()->tick();
      std::cout << "Open 2: " << ossimTimer::instance()->delta_s(t1,t2) << "\n";


      t1 = ossimTimer::instance()->tick();
      ossimImageHandlerRegistry::instance()->open(ossimFilename(argv[1]));
      t2 = ossimTimer::instance()->tick();
      std::cout << "Open 3: " << ossimTimer::instance()->delta_s(t1,t2) << "\n";
#endif
/*      
      ossim_uint32 maxStates=10;
      ossim_uint32 minStates=8;
      ossim::ItemCache<ossim::ImageHandlerState> stateCache;
      ossim_uint32 idx = 0;
      stateCache.setMinAndMaxItemsToCache(minStates, maxStates);
      while(idx < maxStates)
      {
         ossimString id = ossimString::toString(idx);
         stateCache.addItem(id, std::make_shared<ossim::TiffHandlerState>()); 
         ++idx;  
      }
      if(stateCache.getItem(ossimString::toString(0)))
      {

      }
      if(stateCache.getItem(ossimString::toString(maxStates-1)))
      {

      }
      stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 
      stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 
      stateCache.addItem(ossimString::toString(idx++), std::make_shared<ossim::TiffHandlerState>()); 
*/
      //stateCache.printLruIds(std::cout);
      #if 0
      ossim::Json json;

      std::shared_ptr<ossim::istream> in = ossim::StreamFactoryRegistry::instance()->createIstream(argv[1]);
      if(in)
      {
         if(json.parse(*in))
         {
            std::cout << "Parsed Jason Object\n";
         }
      }
      #endif
    // add code
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
