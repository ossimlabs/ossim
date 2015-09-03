//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Test app for ossimString class.
//
// $Id: ossim-string-test.cpp 19871 2011-07-26 11:27:01Z gpotts $
//----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <map>
#include <utility>
#include <iterator>
using namespace std;

#include <ossim/base/ossimString.h>
#include <ossim/base/ossimRegExp.h>

void testString()
{
   std::cout << "Testing string\n";
   
   ossim_float64 d = 0.123456789012345;

   cout << setiosflags(std::ios::scientific)
        << "\nd = 0.123456789012345"
        << "\nossimString::toString(d, 15) "
        << ossimString::toString(d, 15)
        << "\nossimString::toString(d, 15, true) "
        << ossimString::toString(d, 15, true) << "\n";

   d = -32767;
   
   cout << "\nd = -32767"
        << "\nossimString::toString(d, 0) "
        << ossimString::toString(d, 0)
        << "\nossimString::toString(d, 0, true) "
        << ossimString::toString(d, 0, true)

        << "\nossimString::toString(d, 2) "
        << ossimString::toString(d, 2)
        << "\nossimString::toString(d, 2, true) "
        << ossimString::toString(d, 2, true) << "\n";

   
   d = 123.123;
   
   cout << "\nd = 123.123\n"

        << "\nossimString::toString(d) "
        << ossimString::toString(d)
       
        << "\nossimString::toString(d, 0) "
        << ossimString::toString(d, 0)
        << "\nossimString::toString(d, 0, true) "
        << ossimString::toString(d, 0, true)

        << "\nossimString::toString(d, 3) "
        << ossimString::toString(d, 3)
        << "\nossimString::toString(d, 3, true) "
        << ossimString::toString(d, 3, true)

        << "\nossimString::toString(d, 4) "
        << ossimString::toString(d, 4)
        << "\nossimString::toString(d, 4, true) "
        << ossimString::toString(d, 4, true);

   d = 123;
   
   cout << "\nd = 123\n"
        << "\nossimString::toString(d, 0) "
        << ossimString::toString(d, 0)
        << "\nossimString::toString(d, 0, true) "
        << ossimString::toString(d, 0, true)

        << "\nossimString::toString(d, 3) "
        << ossimString::toString(d, 3)
        << "\nossimString::toString(d, 3, true) "
        << ossimString::toString(d, 3, true);

 
   // ossimString os1("1.23456E-10");
   // ossim_float64 f1 = os1.toFloat64();
   ossim_float64 f1 = 1.12345678e-10;
//    ossimString os2 = ossimString::toString(f1,
//                                            10,     // precision
//                                            true,  // trim flag
//                                            true); // scientific flag
   ossimString os2 = ossimString::toString(f1,
                                           10);     // precision

   cout << setiosflags(std::ios::scientific)
        << setprecision(10) 
        << "\nf1 = " << f1
        << "\nossimString::toString(f1,10,true,true): " << os2
        << endl;

   d = 0;
   cout << setiosflags(std::ios::fixed)
        << "\nd = 0"
        << "\nossimString::toString(d, 12, true):  "
      // << ossimString::toString(d, 12, true)
        << ossimString::toString(d, 12)
        << std::endl;

   d = 0.1;
   cout << setiosflags(std::ios::fixed)
        << "\nd = 0.1"
        << "\nossimString::toString(d, true):  "
        << ossimString::toString(d, true)
        << std::endl;

   d = 0.123;
   cout << setiosflags(std::ios::fixed)
        << "\nd = 0.123"
        << "\nossimString::toString(d, 12, true):  "
      // << ossimString::toString(d, 12, true)
        << ossimString::toString(d, 12)
        << std::endl;
   
   d = 123.0;
   cout << setiosflags(std::ios::fixed)
        << "\nd = 123.0"
        << "\nossimString::toString(d, 12, true):  "
      // << ossimString::toString(d, 12, true)
        << ossimString::toString(d, 12)
        << std::endl;
   
   d = 123.456;
   cout << setiosflags(std::ios::fixed)
        << "\nd = 123.456"
        << "\nossimString::toString(d, 12, true):  "
      // << ossimString::toString(d, 12, true)
        << ossimString::toString(d, 12)
        << "\n" << std::endl;

   ossimString os1 = "LE7116034000502150.H1";
   os2 = os1.substitute(ossimString("."), ossimString("_"), false);
   cout << "os1:  " << os1
        << "\ncommand:  ossimString os2 = os1.substitute(ossimString(\".\"), ossimString(\"_\"), false);"
        << "\nos2:  " << os2
        << endl;

   std::string s3 = "abc";
   ossimString os3(s3);
   cout << "ossimString constructed with std::string abc: " << os3 << endl;

   const char* nullCharStr = 0;
   ossimString os4(nullCharStr);
   cout << "ossimString constructed with null char string: " << os4 << endl;

   const char* abcCharStr = "abc";
   ossimString os5(abcCharStr);
   cout << "ossimString constructed with abc char string: " << os5 << endl;

   char a = 'a';
   ossimString os6(a);
   cout << "ossimString constructed with char a: " << os6 << endl;
     
   ossimString os7(5, a);
   cout << "ossimString constructed with 5 char a: " << os7 << endl;

   os7 = a;
   cout << "ossimString assigned char a: " << os7 << endl;

   os7 = nullCharStr;
   cout << "ossimString assigned null string: " << os7 << endl;

   os7 = abcCharStr;
   cout << "ossimString assigned abc char string: " << os7 << endl;

   os7 = s3;
   cout << "ossimString assigned std::string abc: " << os7 << endl;

   os7 = os5;
   cout << "ossimString assigned ossimString abc: " << os7 << endl;

   cout << "ossimString -> numeric test:\n";

   os7 = "240";
   ossim_uint8 pix = os7.toUInt8();
   cout << "toUInt8 test: string = 240, result: " << int(pix) << endl;

   ossimString A ("NAD83_Michigan_South_ft");
   ossimString B ("NAD83 / Michigan South (ft)");
   ossimString separators ("_ /()");
   vector<ossimString> AS = A.split(separators,true);
   vector<ossimString> BS = B.split(separators,true);
   if (AS == BS)
      cout << "split test 1: PASSED"<<endl;
   else
      cout<<"split test 1: FAILED"<<endl;

   ossimString splitTestBlank("A B  C");
   std::vector<ossimString> sliptTestBlankResult = splitTestBlank.split(" ", false);
   std::vector<ossimString> sliptTestBlankResultCompare;
   sliptTestBlankResultCompare.push_back("A");
   sliptTestBlankResultCompare.push_back("B");
   sliptTestBlankResultCompare.push_back("");
   sliptTestBlankResultCompare.push_back("C");
   
   if(sliptTestBlankResult==sliptTestBlankResultCompare)
   {
      cout << "split test blank: PASSED"<<endl;
   }
   else 
   {
      cout << "split test blank: FAILED"<<endl;
   }
   ossimString splitTestEnd("A B C");
   std::vector<ossimString> sliptTestEndResult = splitTestEnd.split(" C", false);
   std::vector<ossimString> sliptTestEndCompare;
   sliptTestEndCompare.push_back("A");
   sliptTestEndCompare.push_back("B");
   sliptTestEndCompare.push_back("");
   
   if(sliptTestEndResult==sliptTestEndCompare)
   {
      cout << "split test end: PASSED"<<endl;
   }
   else 
   {
      cout << "split test end: FAILED"<<endl;
   }
   
   cout << "test of std::map<ossimString, ossimString, ossimStringLtstr>\n";
   std::map<ossimString, ossimString, ossimStringLtstr> testMap;
   testMap.insert( std::make_pair(ossimString("red"), ossimString("1")) );
   testMap.insert( std::make_pair(ossimString("green"), ossimString("2")) );
   testMap.insert( std::make_pair(ossimString("blue"), ossimString("3")) );
   std::map<ossimString, ossimString, ossimStringLtstr>::const_iterator i =
      testMap.find(ossimString("green"));

   if ( i != testMap.end() )
   {
      cout << "Found green in map."
           << "\n(*i).first: " << (*i).first
           << "\n(*i).second: " << (*i).second
           << endl;
   }
   else
   {
      cout << "test of std::map<ossimString, ossimString, ossimStringLtstr> FAILED\n";
   }
   
   ossimString testCompare = "test";
   bool compareEquals1 = testCompare == "test";
   bool compareEquals2 = "test" == testCompare;
   
   if(!compareEquals1)
   {
      std::cout<<"test of ossimString == const char* FAILED\n";
   }
   if(!compareEquals2)
   {
      std::cout<<"test of const char* == ossimString FAILED\n";
   }
}

void testRegularExpression()
{
   std::cout << "Testing regular expression" << std::endl;
   ossimString aString1("Garbage Hi There garbage");
   cout << aString1.match(aString1) << endl;

   ossimString aString2("ab123QQ59ba");
   cout << aString2.match("[^ab1-9]") << endl;

   ossimString aString3("That's OK for me. OK for---you?");
   cout << aString3.match("O(.*r)") << endl;

   ossimString aString4("That's OK for me. OK for---you?");
   cout << aString4.match("[A-Za-z' \\.]*") << endl;

   ossimString aString5(" kasdfj OK---- for ---- --  ------ me. OK for");
   cout << aString5.replaceAllThatMatch("--*[A-Z|a-z| ]*", "*PPPPP*") << endl;

   ossimRegExp reEx("^[d|D][t|T][0-9]");
   
   cout << "\nTest ossimRegExp:\nregular expression=\"^[d|D][t|T][0-9]\"\n";

   aString5 = "dtb";
   cout << "String \"dtb\": " << (reEx.find(aString5.c_str())?"matches":"doesn't match")
        << endl;
   aString5 = "dt0";
   cout << "String \"dt0\": " << (reEx.find(aString5.c_str())?"matches":"doesn't match")
        << endl;
   aString5 = "dT5";
   cout << "String \"dT5\": " << (reEx.find(aString5.c_str())?"matches":"doesn't match")
        << endl;

   ossimRegExp reEx2("[d|D][t|T][0-9]$");

   aString5 = "n32.dt2";
   cout << "String \"n32.dt2\": " << (reEx2.find(aString5.c_str())?"matches":"doesn't match")
        << endl;
}

int main()
{
   testString();
   testRegularExpression();
   
   return 0;
}
