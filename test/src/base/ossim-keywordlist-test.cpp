#include <iostream>
#include <fstream>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimTempFilename.h>

using namespace std;

bool runTestForFileVariations()
{
   cout << "------------------ Running tests on different file variations ---------------- \n";
   bool test_failed = false;

   ossimTempFilename tempFile(".", "temp","txt",true,false);
   tempFile.generateRandomFile();
   
   {
      ofstream out(tempFile.c_str());
      
      out<< "";
      out.close();
      
      ossimKeywordlist kwl;
      cout << "Empty File Test? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = false;
      }
   }
   {
      ossimTempFilename tempFileDos(".", "tempDos","txt",true,false);
      tempFileDos.generateRandomFile();
      
      ofstream outUnix(tempFile.c_str());
      ofstream outDos(tempFileDos.c_str());
      
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
      cout << "Dos carriage returns and unix returns? ";
      
      if((kwlUnix.addFile(tempFile)&&kwlDos.addFile(tempFileDos))&&
         (kwlUnix.getMap() == kwlDos.getMap()))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   {
      ofstream out(tempFile.c_str());
      
      out<< "/adfakdfhkadkjh\n";
      out.close();
      
      ossimKeywordlist kwl;
      cout << "bad comment? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   {
      ofstream out(tempFile.c_str());
      
      out<< "//adfakdfhkadkjh\n";
      out.close();
      
      ossimKeywordlist kwl;
      cout << "good comment? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   {
      ofstream out(tempFile.c_str());
      
      out<< "//adfakdfhkadkjh";
      out.close();
      
      ossimKeywordlist kwl;
      cout << "good comment no end of line? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   {
      ofstream out(tempFile.c_str());
      
      out<< "/test:";
      out.close();
      
      ossimKeywordlist kwl;
      cout << "----------- 1 token lookahead problem, single slash start of line with valid key but no value ------------ \n";
      cout << "Accepted file? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }

      cout << "Verify key? ";
      const char* value = kwl.find("/test");
      if (value)
      {
         cout << "PASSED" << endl;
      }
      else
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   {
      ofstream out(tempFile.c_str());
      
      out<< "test   :";
      out.close();
      cout << "----------- trailing spaces for key trimmed ------------ \n";
      
      ossimKeywordlist kwl;
      cout << "Accepted file? ";
      if(kwl.addFile(tempFile))
      {
         cout << "PASSED" << endl;
      }
      else 
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }

      cout << "Verify key? ";
      const char* value = kwl.find("test");
      if (value)
      {
         cout << "PASSED" << endl;
      }
      else
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
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
      cout << "----------- Testing utility method trimAll values ------------ \n";
      cout << "Verify  value1? ";
      if (value1.trim() == ossimString(kwl.find("key1")))
         cout << "PASSED" << endl;
      else
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
      cout << "Verify  value2? ";
      if (value2.trim() == ossimString(kwl.find("key2")))
         cout << "PASSED" << endl;
      else
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
      cout << "Verify  value3? ";
      if (value3.trim() == ossimString(kwl.find("key3")))
         cout << "PASSED" << endl;
      else
      {
         cout << "FAILED" << endl;
         test_failed = true;
      }
   }
   return test_failed;
}

bool runIncludeTest()
{
   cout << "----------- Testing #include directive handling ------------ \n";


   ossimTempFilename t1(".", "f1","kwl",true,false);
   ossimTempFilename t2(".", "f2","kwl",true,false);
   ossimTempFilename t3(".", "f3","kwl",true,false);
   ossimTempFilename t4(".", "f4","kwl",true,false);

   t1.generateRandomFile();
   t2.generateRandomFile();
   t3.generateRandomFile();
   t4.generateRandomFile();

   ofstream f1 (t1.chars());
   ofstream f2 (t2.chars());
   ofstream f3 (t3.chars());
   ofstream f4 (t4.chars());

   f1<<"f1k1: f1v1\nf1k2: f1v2\n#include "<<t2<<"\n#include \""<<t3<<"\""<<endl;
   f2<<"f2k1: f2v1\n#include "<<t4<<"\nf2k2: f2v2"<<endl;
   f3<<"f3k1: f3v1\nf3k2: f3v2"<<endl;
   f4<<"f4k1: f4v1\nf4k2: f4v2"<<endl;

   f1.close();
   f2.close();
   f3.close();
   f4.close();

   ossimKeywordlist kwl (t1.chars());
   kwl.print(cout);

   ossimString value;
   bool test_failed = true;
   while (true)
   {
      value = kwl.find("f1k1");
      if (value != "f1v1")
         break;
      value = kwl.find("f1k2");
      if (value != "f1v2")
         break;
      value = kwl.find("f2k1");
      if (value != "f2v1")
         break;
      value = kwl.find("f2k2");
      if (value != "f2v2")
         break;
      value = kwl.find("f3k1");
      if (value != "f3v1")
         break;
      value = kwl.find("f3k2");
      if (value != "f3v2")
         break;
      value = kwl.find("f4k1");
      if (value != "f4v1")
         break;
      value = kwl.find("f4k2");
      if (value != "f4v2")
         break;

      test_failed = false;
      break;
   }

   if (!test_failed)
      cout << "#include test PASSED."<<endl;
   else
      cout << "#include test FAILED."<<endl;

   return test_failed;
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
   cout << "tempFile === " << tempFile << endl;
   ofstream out(tempFile.c_str());
   out << kwl << endl;
   out.close();
   
   kwl2.addFile(tempFile.c_str());
   
   cout << "---------------------- Original KWL --------------------\n";
   cout << kwl << endl;
   cout << "---------------------- Testing Original KWL Read back in from file --------------------\n";
   cout << kwl2 << endl;
   cout << "---------------------- Testing Original KWL toString no arguments--------------------\n";
   cout << kwl.toString() << endl;
   cout << "---------------------- Testing Original KWL toString with arguments--------------------\n";
   ossimString result;
   kwl.toString(result);
   cout << result << endl;
   cout << "---------------------- Testing environment variable expand for add variable--------------------\n";
   ossimKeywordlist kwl3;
   kwl3.setExpandEnvVarsFlag(true);
   kwl3.add("test1.", "value", "$(HOME) no new line", true);
   kwl3.add("test1.", "value", "$(HOME) no $(HOME) new line", true);
   cout << kwl3 << endl;
   cout << "------------------ Running sanity tests to make sure that triple quotes are retained ---------------- \n";
 
   cout << "kwl1 == kwl2? " << ((kwl.getMap() == kwl2.getMap())?"PASSED":"FAILED") << endl;
   cout << "testValueAllBlanks preserved? " << ((ossimString(kwl2.find("testAllBlanks.value"))==testValueAllBlanks)?"PASSED":"FAILED") << endl;
   cout << "testValueLeftBlanks preserved? " << ((ossimString(kwl2.find("testLeftBlanks.value"))==testValueLeftBlanks)?"PASSED":"FAILED") << endl;
   cout << "testValueRightBlanks preserved? " << ((ossimString(kwl2.find("testRightBlanks.value"))==testValueRightBlanks)?"PASSED":"FAILED") << endl;
   cout << "testRightBlanksEol preserved? " << ((ossimString(kwl2.find("testRightBlanksEol.value"))==testValueRightBlanksEol)?"PASSED":"FAILED") << endl;
   cout << "complicatedHtmlEmbed preserved? " << ((ossimString(kwl2.find("complicatedHtmlEmbed.value"))==complicatedHtmlEmbed)?"PASSED":"FAILED") << endl;
   bool test_failed = runTestForFileVariations();
   test_failed |= runIncludeTest();

   if (!test_failed)
      cout<<"\nAll tests PASSED.\n"<<endl;
   else
      cout<<"\nEnountered at least one FAILED.\n"<<endl;

   return test_failed;
}
