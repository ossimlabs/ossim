#include <iostream>
#include <fstream>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimTempFilename.h>


void runTestForFileVariations()
{
   std::cout << "------------------ Running tests on different file variations ---------------- \n";
   
   ossimTempFilename tempFile(".", "temp","txt",true,false);
   tempFile.generateRandomFile();
   
   {
      
      std::ofstream out(tempFile.c_str());
      
      out<< "";
      out.close();
      
      ossimKeywordlist kwl;
      std::cout << "Empty File Test? ";
      if(kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
   }
   {
      ossimTempFilename tempFileDos(".", "tempDos","txt",true,false);
      tempFileDos.generateRandomFile();
      
      std::ofstream outUnix(tempFile.c_str());
      std::ofstream outDos(tempFileDos.c_str());
      
      outUnix<< "key1: value1\n";
      outUnix<< "key2: value2\n";
      outUnix<< "key3: value3\n";
      outUnix<< "key4: value4\n";
      outUnix<< "key5: value5\n";
      outUnix.close();
      outDos<< "key1: value1\r";
      outDos<< "key2: value2\r";
      outDos<< "key3: value3\r";
      outDos<< "key4: value4\r";
      outDos<< "key5: value5\r";
      outDos.close();
      
      ossimKeywordlist kwlUnix;
      ossimKeywordlist kwlDos;
      std::cout << "Dos carriage returns and unix returns? ";
      
      if((kwlUnix.addFile(tempFile)&&kwlDos.addFile(tempFileDos))&&
         (kwlUnix.getMap() == kwlDos.getMap()))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
   }
   {
      std::ofstream out(tempFile.c_str());
      
      out<< "/adfakdfhkadkjh\n";
      out.close();
      
      ossimKeywordlist kwl;
      std::cout << "bad comment? ";
      if(!kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
   }
   {
      std::ofstream out(tempFile.c_str());
      
      out<< "//adfakdfhkadkjh\n";
      out.close();
      
      ossimKeywordlist kwl;
      std::cout << "good comment? ";
      if(kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
   }
   {
      std::ofstream out(tempFile.c_str());
      
      out<< "//adfakdfhkadkjh";
      out.close();
      
      ossimKeywordlist kwl;
      std::cout << "good comment no end of line? ";
      if(kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
   }
   {
      std::ofstream out(tempFile.c_str());
      
      out<< "/test:";
      out.close();
      
      ossimKeywordlist kwl;
      std::cout << "----------- 1 token lookahead problem, single slash start of line with valid key but no value ------------ \n";
      std::cout << "Accepted file? ";
      if(kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
      std::cout << "Verify key? " << (kwl.find("/test")?"passed\n":"failed\n");
   }
   {
      std::ofstream out(tempFile.c_str());
      
      out<< "test   :";
      out.close();
      std::cout << "----------- trailing spaces for key trimmed ------------ \n";
      
      ossimKeywordlist kwl;
      std::cout << "Accepted file? ";
      if(kwl.addFile(tempFile))
      {
         std::cout << "passed" << std::endl;
      }
      else 
      {
         std::cout << "failed" << std::endl;
      }
      std::cout << "Verify key? " << (kwl.find("test")?"passed\n":"failed\n");
   }
   {

      ossimKeywordlist kwl;
      ossimString value1 ="   --leading and trailing---   ";
      ossimString value2 ="   --trailing---";
      ossimString value3 ="--leading---   ";
      kwl.add("","key1", value1);
      kwl.add("","key2", value2);
      kwl.add("","key3", value3);
      kwl.trimAllValues();
      std::cout << "----------- Testing utility method trimAll values ------------ \n";
      std::cout << "Verify  value1? " << ((value1.trim() == ossimString(kwl.find("key1")))?"passed":"failed") << std::endl;
      std::cout << "Verify  value2? " << ((value2.trim() == ossimString(kwl.find("key2")))?"passed":"failed") << std::endl;
      std::cout << "Verify  value3? " << ((value3.trim() == ossimString(kwl.find("key3")))?"passed":"failed") << std::endl;
   }
}

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);
   ossimString complicatedHtmlEmbed = "<html>\n\
   <head>\n\
   <title>OMAR Login</title>\n\
   <link rel=\"stylesheet\" href=\"/omar/css/main.css\"/>\n\
   <link rel=\"stylesheet\" href=\"/omar/css/omar-2.0.css\"/>\n\
   <link rel=\"stylesheet\" href=\"/omar/css/main.css\"/>\
   <link rel=\"stylesheet\" href=\"/omar/css/omar-2.0.css\"/>\n\
   <link rel=\"stylesheet\" type=\"text/css\" href=\"richui-0.8/js/yui/reset-fonts-grids/reset-fonts-grids.css\"/>\n\
   <link rel=\"stylesheet\" type=\"text/css\" href=\"richui-0.8/js/yui/layout/assets/skins/sam/layout.css\"/>\n\
   \n\
   \n\
   \n\
   <style>\n\
   /*\n\
   margin and padding on body element\n\
   can introduce errors in determining\n\
   element position and are not recommended;\n\
   we turn them off as a foundation for YUI\n\
   CSS treatments.\n\
   */ \n\
   body {\n\
   margin: 0;\n\
   padding: 0;\n\
   }\n\
   \n\
   /* Set the background color */\n\
   .yui-skin-sam .yui-layout {\n\
   background-color: #FFFFFF;\n\
   }\n\
   \n\
   /* Style the body */\n\
   .yui-skin-sam .yui-layout .yui-layout-unit div.yui-layout-bd {\n\
   background-color: #FFFFFF;\n\
   }\n\
   \n\
   </style>\n\
   </head>\n\
   ";
   ossimString testValueAllBlanks = "          ";
   ossimString testValueLeftBlanks = "  blanks are to the left";
   ossimString testValueRightBlanks = "blanks are to the right    ";
   ossimString testValueRightBlanksEol = "\n\n\n\n\nblanks are to the right   \n\n ";
   ossimKeywordlist kwl;
   ossimKeywordlist kwl2;
   
   kwl.add("test1.", "value", "value no new line", true);
   kwl.add("test2.", "value", "value \n\n\non separate\nlines", true);
   kwl.add("test3.", "value", "value no new line", true);
   kwl.add("test4.", "value", "", true);
   kwl.add("testAllBlanks.",   "value", testValueAllBlanks, true);
   kwl.add("testLeftBlanks.",  "value", testValueLeftBlanks, true);
   kwl.add("testRightBlanks.", "value", testValueRightBlanks, true);
   kwl.add("testRightBlanks.", "value", testValueRightBlanks, true);
   kwl.add("testRightBlanksEol.", "value", testValueRightBlanksEol, true);
   kwl.add("testZ.", "value", "", true);
   kwl.add("complicatedHtmlEmbed.", "value", complicatedHtmlEmbed, true);
   ossimTempFilename tempFile(".", "temp","txt",true,false);
   tempFile.generateRandomFile();
   std::cout << "tempFile === " << tempFile << std::endl;
   std::ofstream out(tempFile.c_str());
   out << kwl << std::endl;
   out.close();
   
   kwl2.addFile(tempFile.c_str());
   
   std::cout << "---------------------- Original KWL --------------------\n";
   std::cout << kwl << std::endl;
   std::cout << "---------------------- Testing Original KWL Read back in from file --------------------\n";
   std::cout << kwl2 << std::endl;
   std::cout << "---------------------- Testing Original KWL toString no arguments--------------------\n";
   std::cout << kwl.toString() << std::endl;
   std::cout << "---------------------- Testing Original KWL toString with arguments--------------------\n";
   ossimString result;
   kwl.toString(result);
   std::cout << result << std::endl;
   std::cout << "---------------------- Testing environment variable expand for add variable--------------------\n";
   ossimKeywordlist kwl3;
   kwl3.setExpandEnvVarsFlag(true);
   kwl3.add("test1.", "value", "$(HOME) no new line", true);
   kwl3.add("test1.", "value", "$(HOME) no $(HOME) new line", true);
   std::cout << kwl3 << std::endl;
   std::cout << "------------------ Running sanity tests to make sure that triple quotes are retained ---------------- \n";
 
   std::cout << "kwl1 == kwl2? " << ((kwl.getMap() == kwl2.getMap())?"passed":"failed") << std::endl;
   std::cout << "testValueAllBlanks preserved? " << ((ossimString(kwl2.find("testAllBlanks.value"))==testValueAllBlanks)?"passed":"failed") << std::endl;
   std::cout << "testValueLeftBlanks preserved? " << ((ossimString(kwl2.find("testLeftBlanks.value"))==testValueLeftBlanks)?"passed":"failed") << std::endl;
   std::cout << "testValueRightBlanks preserved? " << ((ossimString(kwl2.find("testRightBlanks.value"))==testValueRightBlanks)?"passed":"failed") << std::endl;
   std::cout << "testRightBlanksEol preserved? " << ((ossimString(kwl2.find("testRightBlanksEol.value"))==testValueRightBlanksEol)?"passed":"failed") << std::endl;
   std::cout << "complicatedHtmlEmbed preserved? " << ((ossimString(kwl2.find("complicatedHtmlEmbed.value"))==complicatedHtmlEmbed)?"passed":"failed") << std::endl;
   runTestForFileVariations();
}