//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Author:  David Burken
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.
//
// $Id: ossim-foo.cpp 19900 2011-08-04 14:19:57Z dburken $
//----------------------------------------------------------------------------

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimKeywordlist.h>

#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimConstants.h>  // ossim contants...
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimNotify.h>

#include <ossim/base/ossimByteStreamBuffer.h>

// Put your includes here:

#include <iostream>
#include <ostream>
#include <istream>
using namespace std;


int main(int argc, char *argv[])
{
    ossimArgumentParser ap(&argc, argv);
    ossimInit::instance()->addOptions(ap);
    ossimInit::instance()->initialize(ap);
    
    try
    {
        {
            // testing shared buffer access and writing
            //
            char sharedBuf[] = "ABC";
            ossimByteStreamBuffer buf(sharedBuf, 3, true);
            
            buf.setBuf(sharedBuf, 3, true);
            
            std::istream in(&buf);
            std::cout << "Read single byte test? " << (((char)in.get()=='A')?"Passed":"Failed") << std::endl;
            in.seekg(3);
            std::cout << "Read passed input? " << ((in.get()==-1)?"Passed":"Failed") << std::endl;
            
        }
        {
            // dynamic buffer  I/O
            ossimByteStreamBuffer buf;
            std::iostream inout(&buf);
            
            inout.put(0x00);
            std::cout << "Read dynamically added byte? " << ((inout.get()==0x00)?"Passed":"Failed") << std::endl;
            inout << "ABCDEF" << std::endl;
            inout.seekg(1);
            std::cout << "Read after << operation? " << (((char)inout.get()=='A')?"Passed":"Failed") << std::endl;
            inout.seekg(1);
            std::cout << "Tellg? " << (((ossim_int64)inout.tellg()==1)?"Passed":"Failed") << std::endl;
        }
        {
            // test copy construction
            ossimByteStreamBuffer buf;
            std::iostream inout(&buf);
            inout << "ABCDEFG";
            
            ossimByteStreamBuffer buf2(buf);
            std::iostream inout2(&buf2);
            std::cout << "Test read after copy constructor? " << ((inout.get() == inout2.get())?"Passed":"Failed") << std::endl;
        }
    }
    catch (const ossimException& e)
    {
        ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
