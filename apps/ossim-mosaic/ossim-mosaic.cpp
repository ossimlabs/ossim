//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: mosaic.cpp 13312 2008-07-27 01:26:52Z gpotts $
#include <iostream>
#include <fstream>
using namespace std;

#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageMosaic.h>
#include <ossim/imaging/ossimFeatherMosaic.h>
#include <ossim/imaging/ossimBlendMosaic.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageSourceFactory.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageWriter.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/imaging/ossimTiffWriter.h>
#include <ossim/init/ossimInit.h>
#include <ossim/projection/ossimMapProjectionInfo.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimRtti.h>

static ossimTrace traceDebug(ossimString("mosaic:main"));

void outputTemplateKeywordlist(const ossimFilename &templateFilename)
{
   ofstream out(templateFilename.c_str());

   out << "file1.filename: <full path and file name>" << endl
       << "file2.filename: <full path and file name>" << endl
       << "// :\n"
       << "// :\n"
       << "// fileN: <full path and file name to the Nth file in the list>" << endl
       << "\n// currently this option has been tested\n"
       << "// with ossimTiffWriter and ossimJpegWriter\n"
       << "// writer.type: ossimTiffWriter"            << endl
       << "// writer.filename: <full path to output file>"  << endl
       << "\n// Currently, the mosaic application supports\n"
       << "// SIMPLE mosaics (ie. no blending algorithms)\n"
       << "// BLEND  for maps or layers that you want to blend together\n"
       << "// FEATHER for applying a spatial feaher along overlapped regions\n"
       << "// mosaic.type: SIMPLE"                     << endl
       << "\n// product type and projection information" << endl
       << "// is optional.  It will use the first images"<<endl
       << "// geometry information instead." << endl
       << "// product.type: "        << endl
       << "// product.meters_per_pixel_y: "       << endl
       << "// product.meters_per_pixel_x: "       << endl
       << "// product.central_meridian:   " << endl
       << "// product.origin_latitude:"    << endl;

   ossimNotify(ossimNotifyLevel_NOTICE)
      << "Wrote file: " << templateFilename << std::endl;
}

ossimMapProjection* buildProductProjection(const ossimKeywordlist& kwl,
                                           ossimConnectableObject::ConnectableObjectList& inputSources)
{
   const char* productType = kwl.find("product.type");
   const char* productGeom = kwl.find("product.geom_file");
   ossimRefPtr<ossimMapProjection> result = 0;

   // if we don't have a prduct output specified
   // we will just use the first image.
   if(!productType&&!productGeom)
   {
      if(inputSources.size() < 1)
      {
         return NULL;
      }
      ossimKeywordlist geom;
      ossimImageChain* imageChain = PTR_CAST(ossimImageChain, inputSources[0].get());
      ossimConnectableObject* source = NULL;
      
      if(!imageChain)
      {
         source = PTR_CAST(ossimImageHandler, inputSources[0].get());
      }
      else
      {
         source = imageChain->findFirstObjectOfType(STATIC_TYPE_INFO(ossimImageHandler));
      }

      if(source)
      {
         ossimImageHandler* handler = PTR_CAST(ossimImageHandler, source);
         if(handler)
         {
            ossimRefPtr<ossimImageGeometry> geom = handler->getImageGeometry();
            ossimRefPtr<ossimProjection>   temp    = geom->getProjection();
            result = PTR_CAST(ossimMapProjection, temp.get());
         }
      }
      else
      {
         return NULL;
      }
   }
   else
   {
      ossimRefPtr<ossimProjection>   temp    = ossimProjectionFactoryRegistry::instance()->createProjection(kwl,
                                                                                                "product.");
      result = PTR_CAST(ossimMapProjection, temp.get());
   }

   return result.release();
}

bool buildRenderers(const ossimKeywordlist& specFile,
                    ossimConnectableObject::ConnectableObjectList& imageSources,
                    ossimMapProjection* productProjection)
{
   ossim_int32 index = 0;

   // add the renderer to each chain.  First, look for
   // the ossimImageHandler and get the image
   // geometry for this chain.  The add the ossimImageRenderer
   // to each chain and then connect it up
   //
   for(index = 0; index < (ossim_int32)imageSources.size(); ++index)
   {
      ossimImageChain* imageChain = PTR_CAST(ossimImageChain, imageSources[index].get());
      if(imageChain)
      {
         ossimConnectableObject* source = imageChain->findFirstObjectOfType(STATIC_TYPE_INFO(ossimImageHandler));

         if(source)
         {
            ossimImageHandler* imageHandler = PTR_CAST(ossimImageHandler,
                                                       source);


            ossimRefPtr<ossimImageGeometry> geom = imageHandler->getImageGeometry();
            // now add the image/view transform to the
            // renderer.
            //
            if(geom.valid())
            {
               ossimImageSource* last = imageChain->getFirstSource();

               
               ossimImageRenderer* renderer = new ossimImageRenderer;
               renderer->connectMyInputTo(0, PTR_CAST(ossimConnectableObject, last));
               imageChain->add(renderer);
               ossimImageViewProjectionTransform* transform = new ossimImageViewProjectionTransform;
               transform->setImageGeometry(geom.get());
               ossimRefPtr<ossimImageGeometry> viewGeom = new ossimImageGeometry();
               viewGeom->setProjection(productProjection);
               // Make a copy of the view projection for
               // each chain by passing in the object and not
               // the pointer.
               transform->setViewGeometry(viewGeom.get());
               
               renderer->setImageViewTransform(transform);
               imageChain->initialize();
            }
            else
            {
               return false;
            }
         }
      }
   }

   return true;
}

bool buildChains(std::vector<ossimFilename>& fileList,
                 ossimConnectableObject::ConnectableObjectList& chains)
{
   bool result = true;
   chains.clear();

   for(ossim_int32 index = 0; index < (ossim_int32)fileList.size();++index)
   {
      ossimRefPtr<ossimImageSource> handler = ossimImageHandlerRegistry::instance()->open(fileList[index]);

      if(handler.valid())
      {
         ossimImageChain* imageChain = new ossimImageChain;
         imageChain->add(handler.get());
         chains.push_back(imageChain);
      }
      else
      {
         cerr << "Error: Unable to load image " << fileList[index] << endl;
         result = false;
      }
   }

   return result;
}

bool buildChains(const ossimKeywordlist& kwl,
                 ossimConnectableObject::ConnectableObjectList& chains)
{
   ossim_int32 index = 0;
   ossim_int32 result = kwl.getNumberOfSubstringKeys("file[0-9]+\\.filename");
   const char* lookup = NULL;
   ossim_int32 numberOfMatches = 0;
   vector<ossimFilename> fileList;
   
   while(numberOfMatches < result)
   {
      ossimString searchValue = "file" + ossimString::toString(index);
      
      ossimString filename = searchValue + ".filename";
      lookup = kwl.find(filename.c_str());
      if(lookup)
      {
         fileList.push_back(ossimFilename(lookup));
         ++numberOfMatches;
      }
      ++index;
   }
      
   return buildChains(fileList,
                      chains);
}

ossimRefPtr<ossimImageCombiner> createMosaic(const ossimKeywordlist& kwl,
                               ossimConnectableObject::ConnectableObjectList& inputSources)
{
   const char* mosaicType = kwl.find("mosaic.type");
   //ossimImageCombiner* mosaic = NULL;
   ossimRefPtr<ossimImageCombiner> mosaic = 0;
   if(mosaicType)
   {
      if(ossimString(mosaicType).upcase() == "FEATHER")
      {
         mosaic  = new ossimFeatherMosaic;
      }
      else if(ossimString(mosaicType).upcase() == "BLEND")
      {
         mosaic  = new ossimBlendMosaic;
      }
      else
      {
		 mosaic = PTR_CAST(ossimImageCombiner, ossimObjectFactoryRegistry::instance()->createObject(ossimString(mosaicType)));
      }
   }
   else
   {
      mosaic = new ossimImageMosaic;
   }
   if (!mosaic) cerr << "Could not create Mosaic!";

   for(ossim_int32 index = 0; index < (ossim_int32)inputSources.size(); ++index)
   {
      mosaic->connectMyInputTo(inputSources[index].get());
   }

   mosaic->initialize();

   return mosaic;
}
ossimString massageQuotedValue(const ossimString& value)
{
   char quote = '\0';
   if(*value.begin() == '"')
   {
      quote = '"';
   }
   else if(*value.begin() == '\'')
   {
      quote = '\'';
   }
   
   if(quote == '\0')
   {
      return value;
   }
   std::vector<ossimString> splitString;
   value.split(splitString, quote);
   if(splitString.size() == 3)
   {
      return splitString[1];
   }
   return value;
}
int main(int argc, char *argv[])
{
	ossimArgumentParser argumentParser(&argc, argv);
	ossimInit::instance()->addOptions(argumentParser);
	ossimInit::instance()->initialize(argumentParser);
	argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
	argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName() +" Takes a list of images and outputs to mosaic.");
	argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options] <input_file1> <input_file2> <input_file...> <output_file>");
	argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
	argumentParser.getApplicationUsage()->addCommandLineOption("-k", "keyword list to load from");
	argumentParser.getApplicationUsage()->addCommandLineOption("-m", "Mosiac type (SIMPLE, BLEND, FEATHER, or any MosaicCombinerType [ossimClosestToCenter, ossimFeatherMosaic, etc..]) (default=SIMPLE");
	argumentParser.getApplicationUsage()->addCommandLineOption("-w", "Writer type (tiff_strip, jpeg, etc... see ossim-info --writers) (default=tiff_strip)");
	argumentParser.getApplicationUsage()->addCommandLineOption("--writer-prop <string>", "Adds a property to send to the writer. format is name=value");
	argumentParser.getApplicationUsage()->addCommandLineOption("-t", "ouput a keyword list template");

	// Allocate some stuff up front for use later
	ossimKeywordlist kwl;
	ossimString theWriterType = "tiff_strip";
	bool keywordlistSupplied = false;
	bool optionGiven = false;
	ossimFilename outfile = "";
	std::string tempString;
	ossimArgumentParser::ossimParameter stringParam(tempString);
	std::map<ossimString, ossimString, ossimStringLtstr> writerPropertyMap;

	// user input writer props, should use those
	while(argumentParser.read("--writer-prop", stringParam))
   {
      std::vector<ossimString> nameValue;
      ossimString(tempString).split(nameValue, "=");
      if(nameValue.size() == 2)
      {
         writerPropertyMap.insert(std::make_pair(nameValue[0], massageQuotedValue(nameValue[1])));
      }
   }

	// User input a writer type
	while (argumentParser.read("-w", stringParam))
   {
      theWriterType = tempString;
   }

   // Keyword list to load from
   while (argumentParser.read("-k", stringParam))
   {
      kwl.addFile(tempString.c_str());
      keywordlistSupplied = true;
      optionGiven         = true;
   }

   // Mosiac type
   while (argumentParser.read("-m", stringParam))
   {
      ossimString opt = tempString;
      opt = opt.upcase();
      opt = opt.trim();
      if( (opt != "SIMPLE")&&
          (opt != "BLEND")&&
          (opt != "FEATHER"))
      {
         opt = tempString;
      }
      kwl.add("mosaic.type",
              opt.c_str(),
              true);
   }
   
   // Output KWL template
   while (argumentParser.read("-t", stringParam))
   {
      ossimFilename templateFilename = tempString.c_str();
      outputTemplateKeywordlist(templateFilename);
      exit(0);
   }
   
   // Display Help - changed 3 to 2, because you can just supply a kwl and an output
   while (argumentParser.read("-h") ||
       argumentParser.read("--help") ||
       (argumentParser.argc() < 2 ))
   {
      argumentParser.getApplicationUsage()->write(
         ossimNotify(ossimNotifyLevel_INFO));
      exit(0);
   }

   int argCount = argumentParser.argc();

   // Get the input files.
   for (int i=1; i< (argCount-1); ++i)
   {
      ossimFilename f = argv[i];
      if(f.exists())
      {
         kwl.add((ossimString("file")+ossimString::toString(i)+".").c_str(),
                 "filename",
                 f.c_str(),
                 true);
      }
      keywordlistSupplied = true;
      optionGiven         = true;
   }

   // Get the output file.
   outfile = argv[argCount-1];

   if(outfile.exists())
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << "mosaic ERROR:"
         << "\nOuput file " << outfile << " exits and will not be overwritten!"
         << "\nExiting..."
         << std::endl;
      exit(1);
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "kwl:\n" << kwl
         << "output file: " << outfile << std::endl;
   }

   if(keywordlistSupplied)
   {
	   // NOTE: Got rid of the separate method writeMosaic and moved the code down here so i didn't have to pass a ton of variables to it.
		ossimConnectableObject::ConnectableObjectList inputSources;

		if(outfile == "")
		{
		   outfile = "./output.tif";
		}
		// build the image and a remapper for it.
		// if we were successful then we will add the image
		// renderer that will transform it it some output
		// product projection.
		//
		if(buildChains(kwl, inputSources))
		{
		   ossimMapProjection* productProjection = buildProductProjection(kwl,
		                                                                  inputSources);
		   
		   if(!productProjection)
		   {
		      cerr << "unable to create product projection" << endl;
		      return false;
		   }
		   
		   // now let's build up the renderers
		   //
		   if(buildRenderers(kwl,
		                     inputSources,
		                     productProjection))
		   {
				
		      // Create mosaic object
		      ossimRefPtr<ossimImageCombiner> mosaic = createMosaic(kwl,
		                                                inputSources);
			  // Create writer from kwl if supplied
		      ossimRefPtr<ossimImageFileWriter> writer = ossimImageWriterFactoryRegistry::instance()->createWriter(kwl,"writer.");

		      if(!writer)
		      {
				  // If no kwl was supplied, then create a writer from the 'theWriterType' string
				 writer = ossimImageWriterFactoryRegistry::instance()->createWriter(theWriterType);
		         //writer = new ossimTiffWriter; // the default 'theWriterType' is not 'tiff_strip' which should be this class
		         writer->open(outfile);
		      }
		      
		      if(writer.valid())
		      {
		         ossimStdOutProgress listener;
		         if(mosaic.valid())
		         {
		            writer->addListener(&listener);
		            writer->connectMyInputTo(0, mosaic.get());
		            writer->initialize();
		            
					if ( writerPropertyMap.size() )
					{
					   ossimPropertyInterface* propInterface = (ossimPropertyInterface*)writer.get();
					   std::map<ossimString, ossimString, ossimStringLtstr>::iterator iter = writerPropertyMap.begin();
					   while(iter!=writerPropertyMap.end())
					   {
					      propInterface->setProperty(iter->first, iter->second);
					      ++iter;
					   }
					}
					
					// NOTE: This stuff is copied in anyhow not sure why it was being done in the first place... 
					// still here because maybe it is required! should be more generic.

					//ossimMapProjectionInfo mapInfo(productProjection,
		            //                               mosaic->getBoundingRect());
		            //ossimTiffWriter* tempTiffPtr = PTR_CAST(ossimTiffWriter, writer.get());
		            //
		            //if(tempTiffPtr)
		            //{
		            //   tempTiffPtr->setProjectionInfo(mapInfo);
		            //   tempTiffPtr->execute();
		            //}
		            //else
		            //{
					// Do the hard work here..
		            if (!writer->execute())
					{
						ossimNotify(ossimNotifyLevel_WARN)
							<< "mosaic ERROR:"
							<< "\nCould not execute writer!"
							<< "\nExiting..."
							<< std::endl;
						return 1;
					}
		            //}
		            writer->removeListener(&listener);
		         }
		         writer->disconnect();
		         mosaic->disconnect();
		         mosaic = 0;
		         writer = 0;
		      }
		   }
		   else
		   {
		      return 1;
		   }
		}
   }

   if(!optionGiven)
   {
      argumentParser.getApplicationUsage()->write(
         ossimNotify(ossimNotifyLevel_INFO));
   }

   return 0;
}
