//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// File: ossim-foo.cpp
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "svn revert foo.cpp" if you want to
//        keep your working repository up to snuff.
//
// $Id$
//----------------------------------------------------------------------------

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
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimVisitor.h>

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

#include <ossim/init/ossimInit.h>

#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <ossim/support_data/ossimSrcRecord.h>
#include <ossim/support_data/ossimWkt.h>
#include <ossim/base/ossimUrl.h>
//#include <ossim/base/ossimStreamFactoryRegistry.h>

// Put your includes here:

// System includes:
#include <cmath>
#include <sstream>
#include <iostream>
using namespace std;

#if 0
class char_array_buffer : public std::streambuf {
public:
    char_array_buffer(const char *data, unsigned int len);
 
private:
    int_type underflow();
    int_type uflow();
    int_type pbackfail(int_type ch);
    std::streamsize showmanyc();
 
    const char * const begin_;
    const char * const end_;
    const char * current_;
};
 
char_array_buffer::char_array_buffer(const char *data, unsigned int len)
: begin_(data), end_(data + len), current_(data) { }
 
char_array_buffer::int_type char_array_buffer::underflow() {
    if (current_ == end_) {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*current_);     // HERE!
}
 
char_array_buffer::int_type char_array_buffer::uflow() {
    if (current_ == end_) {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*current_++);   // HERE!
}
 
char_array_buffer::int_type char_array_buffer::pbackfail(int_type ch) {
    if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1])) {
        return traits_type::eof();
    }
    return traits_type::to_int_type(*--current_);   // HERE!
}
 
std::streamsize char_array_buffer::showmanyc() {
    return end_ - current_;
}
#endif
int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);



//   char* data="Hello!";
//   int len = 7;
//char_array_buffer buf((char*)data, len);     // no copy here!!!
// std::istream is(&buf);  

//     char * buffer = new char [len];
//     is.read(buffer,len);

// std::cout << "len == " << buffer << std::endl;
    try
   {
//    std::shared_ptr<ossim::istream> inStream = 
//        ossim::StreamFactoryRegistry::instance()->createIstream("s3://ossimlabs/dependencies/jai/jai_codec-1.1.3.jar",
//                std::ios::in);
      // ossimUrl url("/data/test/test1.tif");

      // std::cout << url.toString() << std::endl;
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
