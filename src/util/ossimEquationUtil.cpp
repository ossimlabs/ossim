//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: ossimEquationUtil.cpp 23434 2015-07-15 17:01:59Z gpotts $
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#include <ossim/util/ossimEquationUtil.h>
#include <ossim/imaging/ossimEquationCombiner.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimRtti.h>

static ossimTrace traceDebug(ossimString("mosaic:main"));

// Copied from ossimEquationCombiner.h:
static const char* EQ_SPEC = "\nEquation Specification:\n\
\n\
sin(x)                 takes the sine of the input  \n\
sind(x)                takes the sin of the input and assumes degree input \n\
cos(x)                 takes cosine of input \n\
cosd(x)                takes the cosine of input and assumes input in degrees \n\
sqrt(x)                takes square root of input \n\
log(x)                 takes the natural log of input \n\
log10(x)               takes the log base 10 of the input \n\
exp(x)                 takes the e raised to the passed in argument \n\
abs(x)                 takes the absolute value of the passed in value \n\
min(x1, x2, ... xn)    takes the min of all values in the list \n\
max(x1, x2, ... xn)    takes the max of all values in the list. \n\
\n\
clamp(image_data, min, max) \n\
                      will clamp all data to be between the min max values. \n\
                      will set anything less than min to min and anythin \n\
                      larger than max to max \n\
\n\
band(image_data, num)  returns a single band image object \n\
                      by selecting band num from input image i1.  Note \n\
                      the first argument must be an image \n\
                      and the second argument must be a number \n\
\n\
shift(index, num_x, num_y) \n\
                      currently, the first argument must be an image \n\
                      variable(i1, i2, ... in) and x, and y must b numbers \n\
                      indicating the delta in that direction to shift the \n\
                      input. \n\
\n\
blurr(index, rows, cols) \n\
                      Will blurr the input image i with a \n\
                      rows-by-cols kernel.  All values are equal \n\
                      weight.  Note the fist argument must by an image \n\
                      variable (ex: i1, i2,....in). \n\
\n\
conv(index, rows, cols, <row ordered list of values> ) \n\
                      this allows you to define an arbitrary matrix.  The \n\
                      <row ordered list of values> is a comma separated \n\
                      list of constant values. \n\
\n\
assign_band(image_data, num1, data2, num2) \n\
                      will take band num2 from image data2 and assign it to \n\
                      band num1 in data 1. \n\
\n\
assign_band(image_data, num1, data2) \n\
                      will take band 1 from image data2 and assign it to \n\
                      band num1 in data 1. \n\
\n\
assign_band(image_data, num1, num2) \n\
                      will assin to band num1 of data 1 the value of num2 \n\
\n\
x1 * x2                will multiply x1 and x2 \n\
x1 + x2                will add x1 and x2 \n\
x1 - x2                will subtract x1 and x2 \n\
x1 / x2                will divide x1 and x2 \n\
x1 ^ x2                will do a power, raises x1 to x2 \n\
x1 | x2                will do a bitwise or operation \n\
                      ( will do it in unisgned char precision) \n\
\n\
x1 & x2                will do a bitwise and operation \n\
                      ( will do it in unsigned char precision) \n\
\n\
~x1                    will do the ones complement of the input \n\
\n\
x1 xor x2              will do an xclusive or operation \n\
                      (will do it in unsigned char precision) \n\
\n\
- x1                   will negative of x1 \n\
\n\
Boolean ops: 1=true, 0=false \n\
x1 > x2 \n\
x1 >= x2 \n\
x1 == x2 \n\
x1 <= x2 \n\
x1 < x2 \n\
x1 <> x2 \n\
\n\
Note: \n\
\n\
Currently an image input is reference by the variable in[<I>] where \n\
<I> is the input image index beginning at 0.  So 1 referes to the second image \n\
in the input list. \n\
\n\
(in[0] + in[1])/2 \n\
Will take image 1 and add it to image 2 and average them. \n\
\n\
exp(sqrt(in[0])/4) \n\
Will take the root of the image and divide by 4 and then raise e to that \n\
amount. \n\
\n\
128 \n\
Will return a constant value of 128 for all input bands. \n\
\n\
min(1,in[2],in[3], max(in[1],in[0])) \n\
\n\
shift(0, 1, 1) - i1 \n\
Will shift input 0 by 1 pixel along the diagonal  and then subtract it \n\
from input 1. \n\
\n\
assign_band(in[0], 1, blurr(in[0], 5, 5), 2) \n\
Will assign to the first band of i1 the 2nd band of the 5x5 blurr of i1. \n\
\n\
conv(0, 3, 3, -1, -2, -1, 0, 0, 0, 1, 2, 1) \n\
Will convolve the first input connection with a 3x3 matrix. \n\
The args are row ordered: \n\
                        -1, -2, -1 \n\
                         0,  0,  0 \n\
                         1,  2,  1 \n\
\n\
NDVI: \n\
N=(in[0]-in[1])/(in[0]+in[1]) \n\
\n\
For indexed-type values,like NDVI, (with limited values) it is better \n\
to rescale between 0.0 and 1.0 and use type NormalizedFloat. \n\
\n\
Rescaled NDVI between 0 and 1: \n\
(N+1)/2 = in[0]/(in[0]+in[1]) \n\
\n";

ossimEquationUtil::ossimEquationUtil(ossimArgumentParser& ap)
:  m_argumentParser(new ossimArgumentParser(ap))
{
   m_usage.setApplicationName(ap.getApplicationName());
   m_usage.setDescription(ap.getApplicationName() +
                        " Takes a list of images and performs the specified equation. The inputs"
                        "must be in the same projection as the operations are done at the pixel"
                        "level with no renderer in th chain. The output file will contain the same"
                        "geospatial header info as the first image in the input list.");
   m_usage.setCommandLineUsage(ap.getApplicationName()+
                              " \"<equation spec>\" <input_file1> <input_file2> <input_file...> <output_file>");
   m_usage.addCommandLineOption("-h or --help","Display this information");
   m_usage.addCommandLineOption("-k <filename>", "keyword list to load from");
   m_usage.addCommandLineOption("-w <type>", "Writer type (tiff_strip, jpeg, etc... see ossim-info --writers) (default=tiff_strip)");
   m_usage.addCommandLineOption("--writer-prop <string>", "Adds a property to send to the writer. format is name=value");
   m_usage.addCommandLineOption("-t <filename>", "ouput a keyword list template");
}

bool ossimEquationUtil::execute()
{
   // Allocate some stuff up front for use later
   ossimString writerType = "tiff_strip";
   ossimString equationSpec;
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   std::map<ossimString, ossimString, ossimStringLtstr> writerPropertyMap;
   vector<ossimFilename> infiles;
   ossimFilename outfile;

   // Display Help
   if (m_argumentParser->read("-h") || m_argumentParser->read("--help") || (m_argumentParser->argc() < 4))
   {
      m_usage.write(ossimNotify(ossimNotifyLevel_INFO));
      ossimNotify(ossimNotifyLevel_INFO)<<EQ_SPEC<<endl;
      return false;
   }

   // Output KWL template
   if (m_argumentParser->read("-t", stringParam))
   {
      ossimFilename templateFilename = tempString.c_str();
      outputTemplateKeywordlist(templateFilename);
      return false;
   }

   // Keyword list to load from
   if (m_argumentParser->read("-k", stringParam))
   {
      if (!parseKwl(tempString.c_str(), infiles, equationSpec, outfile, writerType))
         return false;
   }
   else
   {
      // Everything on command line:
      // user input writer props, should use those
      while(m_argumentParser->read("--writer-prop", stringParam))
      {
         std::vector<ossimString> nameValue;
         ossimString(tempString).split(nameValue, "=");
         if(nameValue.size() == 2)
            writerPropertyMap.insert(std::make_pair(nameValue[0], nameValue[1]));
      }

      // User input a writer type
      if (m_argumentParser->read("-w", stringParam))
         writerType = tempString;

      int argCount = m_argumentParser->argc();
      if (argCount < 4)
      {
         m_usage.write(ossimNotify(ossimNotifyLevel_INFO));
         ossimNotify(ossimNotifyLevel_INFO)<<EQ_SPEC<<endl;
         return false;
      }

      // First the equation spec:
      equationSpec = m_argumentParser->argv()[1];

      // Get the input files.
      for (int i=2; i< (argCount-1); ++i)
         infiles.push_back(ossimFilename(m_argumentParser->argv()[i]));

      // Get the output file.
      outfile = m_argumentParser->argv()[argCount-1];
   }

   if (!initInputSources(infiles))
      return false;

   // Create combiner object
   ossimRefPtr<ossimEquationCombiner> combiner = new ossimEquationCombiner(m_inputSources);
   combiner->setEquation(equationSpec);

   // Create writer:
   ossimRefPtr<ossimImageFileWriter> writer;
   writer = ossimImageWriterFactoryRegistry::instance()->createWriter(writerType);
   if(!writer.valid())
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<"Could not create writer of type <"<<writerType<<">"<<endl;
      return false;
   }
   writer->connectMyInputTo(combiner.get());
   writer->setFilename(outfile);
   ossimIrect bounding_irect;
   m_prodGeometry->getBoundingRect(bounding_irect);
   writer->setAreaOfInterest(bounding_irect);
   writer->initialize();

   if ( writerPropertyMap.size() )
   {
      ossimPropertyInterface* propInterface = (ossimPropertyInterface*) writer.get();
      std::map<ossimString, ossimString, ossimStringLtstr>::iterator iter = writerPropertyMap.begin();
      while(iter!=writerPropertyMap.end())
      {
         propInterface->setProperty(iter->first, iter->second);
         ++iter;
      }
   }

   // Execute:
   ossimStdOutProgress listener;
   writer->addListener(&listener);
   if (!writer->execute())
   {
      ossimNotify(ossimNotifyLevel_WARN)<< "Error encountered writing file..."<<endl;
      return false;
   }

   writer->close();
   return true;
}

void ossimEquationUtil::outputTemplateKeywordlist(const ossimFilename &templateFilename)
{
   ofstream out(templateFilename.c_str());

   out << "file1.filename: <full path and file name>" << endl
       << "file2.filename: <full path and file name>" << endl
       << "// :\n"
       << "// :\n"
       << "// fileN.filename:: <full path and file name>" << endl
       << "\n// currently this option has been tested\n"
       << "// with ossimTiffWriter and ossimJpegWriter\n"
       << "writer.type: tiff_strip"            << endl
       << "writer.filename: <full path to output file>"  << endl
       << "\n// Equation specification:\n"
       << "equation: <equation spec string>"<<endl;

   ossimNotify(ossimNotifyLevel_NOTICE) << "Wrote file: " << templateFilename << std::endl;
}

bool ossimEquationUtil::parseKwl(const ossimFilename& kwl_file,
              vector<ossimFilename>& inputs,
              ossimString& equationSpec,
              ossimFilename& output,
              ossimString& writerType)
{
   ossimKeywordlist kwl;
   if (!kwl.addFile(kwl_file))
      return false;

   ossim_int32 index = 0;
   ossim_int32 result = kwl.getNumberOfSubstringKeys("file[0-9]+\\.filename");
   const char* lookup = NULL;
   ossim_int32 numberOfMatches = 0;

   while(numberOfMatches < result)
   {
      ossimString searchValue = "file" + ossimString::toString(index);

      ossimString filename = searchValue + ".filename";
      lookup = kwl.find(filename.c_str());
      if(lookup)
      {
         inputs.push_back(ossimFilename(lookup));
         ++numberOfMatches;
      }
      ++index;
   }

   equationSpec = kwl.find("equation");
   if (equationSpec.empty())
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<"No equation specified in KWL"<<endl;
      return false;
   }

   output = kwl.find("writer.filename");
   if (output.empty())
   {
      ossimNotify(ossimNotifyLevel_FATAL)<<"No output filename specified in KWL"<<endl;
      return false;
   }

   writerType = kwl.find("writer.type");
   if (writerType.empty())
      writerType = "tiff_strip";

   return true;
}

bool ossimEquationUtil::initInputSources(vector<ossimFilename>& fileList)
{
   bool result = true;
   m_inputSources.clear();
   ossimRefPtr<ossimImageGeometry> input_geom;

   for(ossim_int32 index = 0; index < (ossim_int32)fileList.size();++index)
   {
      ossimRefPtr<ossimImageHandler> handler =
            ossimImageHandlerRegistry::instance()->open(fileList[index]);
      if(!handler.valid())
      {
         cerr << "Error: Unable to load image " << fileList[index] << endl;
         result = false;
         break;
      }

      input_geom = handler->getImageGeometry();

      if (index == 0)
      {
         m_prodGeometry = (ossimImageGeometry*) input_geom->dup();
         ossimGpt origin;
         m_prodGeometry->getTiePoint(origin, false);
         ossimDpt gsd (m_prodGeometry->getMetersPerPixel());

         if (m_prodGeometry->isAffectedByElevation())
         {
            // Need to ortho to UTM, so define output geometry shared by all input chains:
            ossimMapProjection* prod_proj = new ossimUtmProjection;
            prod_proj->setOrigin(origin);
            prod_proj->setMetersPerPixel(m_prodGeometry->getMetersPerPixel());
            m_prodGeometry->setProjection(prod_proj);
         }
      }

      // Set up the IVT for this input's renderer:
      ossimRefPtr<ossimImageViewProjectionTransform> transform = new ossimImageViewProjectionTransform;
      transform->setImageGeometry(input_geom.get());
      transform->setViewGeometry(m_prodGeometry.get());

      ossimRefPtr<ossimImageRenderer> renderer = new ossimImageRenderer;
      renderer->connectMyInputTo(0, handler.get());
      renderer->setImageViewTransform(transform.get());
      renderer->initialize();

      m_inputSources.push_back(renderer.get());
   }

   return result;
}

