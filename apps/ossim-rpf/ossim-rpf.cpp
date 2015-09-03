//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Authors:  Eric Hirschorn, David Burken
//
// Description:
//
// Command line application "ossim-rpf" to:
// 1) Print raster product format(rpf) frames list from a.toc file.
// 2) Generate an a.toc file from a list of rpf frames.
//
//----------------------------------------------------------------------------
// $Id$

#include <cstdlib> /* for exit */
#include <iomanip>
#include <iostream>

using namespace std;

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/init/ossimInit.h>

#include <ossim/imaging/ossimCibCadrgTileSource.h>
#include <ossim/support_data/ossimNitfFile.h>
#include <ossim/support_data/ossimNitfFileHeader.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_X.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_0.h>
#include <ossim/support_data/ossimNitfFileHeaderV2_1.h>
#include <ossim/support_data/ossimNitfImageHeader.h>
#include <ossim/support_data/ossimNitfTagInformation.h>
#include <ossim/support_data/ossimRpfAttributes.h>
#include <ossim/support_data/ossimRpfFrame.h>
#include <ossim/support_data/ossimRpfHeader.h>
#include <ossim/support_data/ossimRpfReplaceUpdateTable.h>
#include <ossim/support_data/ossimRpfToc.h>
#include <ossim/support_data/ossimRpfTocEntry.h>

#include <ossim/util/ossimRpfUtil.h>

static ossimTrace traceDebug = ossimTrace("ossim-rpf:debug");

static const ossimString A_DOT_TOC_FILE = "a.toc";

/**
 * @brief Prints out the list of frames referred to in the given
 * input RPF file. Will work even if all the frames files are 
 * missing. Deprecated function which is also available in the
 * ossim-img2vrr application.
 */
void printFrameList( const ossimFilename& imageFile,
                     std::ostream& out, 
                     const ossimString& prefix );

void framesToToc( const ossimFilename& dotRpfFile,
                  const ossimFilename& outputDir );

void aDotTocToDotRpf( const ossimFilename& aDotTocFile,
                      const ossimFilename& outputDir );

int main(int argc, char* argv[])
{
   //---
   // Get the arg count so we can tell if an arg was consumed by
   // ossimInit::instance()->initialize
   //---
   int originalArgCount = argc;
   
   ossimString tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser ap(&argc, argv);
   ossimInit::instance()->addOptions(ap);
   ossimInit::instance()->initialize(ap);

   if ( (ap.argc() == 1) && (originalArgCount > 1) )
   {
      exit(0); // ossimInit consumed all options.
   }
   
   ossimApplicationUsage* appuse = ap.getApplicationUsage();

   appuse->setApplicationName(ap.getApplicationName());
   
   appuse->setDescription(ap.getApplicationName()+" Utility application for a.toc files.");
   
   appuse->addCommandLineOption("-h or --help", "Shows help");

   appuse->addCommandLineOption("--list-frames"," <a.toc> Lists RPF formatted files of the a.toc passed to this option."); 
   
   appuse->addCommandLineOption("--create-toc", "<dot_rpf_file> <output_directory>\nWrites a toc file out from dot rpf file.");

   appuse->addCommandLineOption("--create-dot-rpf", "<a.toc> <output_directory>\nWrites a dot rpf file(s) to output directory from an a.toc file.");
   
   
   //---
   // Extract optional arguments.
   //---
   bool needUsage   = true;
   bool writeToc    = false;
   bool writeDotRpf = false;
   ossimFilename inputTocFile  = ossimFilename::NIL;
   ossimFilename outputTocFile = ossimFilename::NIL;
   
   if( ap.read("--list-frames", stringParam) )
   {
      inputTocFile = tempString;
      needUsage = false;
   }
   
   if( ap.read("--create-toc") )
   {
      writeToc  = true;
      needUsage = false;
   }

   if( ap.read("--create-dot-rpf") )
   {
      writeDotRpf = true;
      needUsage   = false;
   }

   if(ap.read("-h") || ap.read("--help") || needUsage )
   {
      appuse->write(std::cout);
      exit(0);
   }
   
   ap.reportRemainingOptionsAsUnrecognized();
   if (ap.errors())
   {
      ap.writeErrorMessages(std::cout);
      exit(1);
   }
   
   if ( inputTocFile.size() && inputTocFile.exists() )
   {
      ossimString prefix = "toc.";
      printFrameList( inputTocFile, std::cout, prefix );
   }
   
   if (writeToc)
   {
      if (argc != 3)
      {
         cout << argv[0]
              << " --create-toc <dot_rpf_file> <output_directory>"
              << std::endl;
         exit(0);
      }

      ossimFilename dotRpfFile        = argv[1];
      ossimFilename outputDir         = argv[2];

      framesToToc( dotRpfFile, outputDir );
   }

   if (writeDotRpf)
   {
      if (argc != 3)
      {
         cout << argv[0]
              << " --create-dot-rpf <a.toc> <output_directory>"
              << std::endl;
         exit(0);
      }
      
      ossimFilename aDotTocFile = argv[1];
      ossimFilename outputDir   = argv[2];

      aDotTocToDotRpf( aDotTocFile, outputDir );
   }
   
   exit(0);
   
} // End of main...

void printFrameList( const ossimFilename& imageFile,
                     std::ostream& out, 
                     const ossimString& prefix )
{
   // Tell the instance to skip the empty check and open 
   // the file no matter if the frame images are there or not.
   ossimRefPtr<ossimCibCadrgTileSource> pRpf = 
      new ossimCibCadrgTileSource();
   if ( pRpf != 0 )
   {
      if( pRpf->isOpen() )
      {
         pRpf->close();
      }
      
      pRpf->setSkipEmptyCheck( true );
      pRpf->setFilename(imageFile);
      
      if( pRpf->open() )
      {
         int nTotalFramesFound = 0;
         
         ossim_uint32 nRows = -1;
         ossim_uint32 nCols = -1;
         
         const ossimRpfToc* pRpfToc = pRpf->getToc();
         
         ossimFilename rootDir;
         pRpfToc->getRootDirectory( rootDir );
         
         // Number of directories of images
         unsigned long nEntries = pRpfToc->getNumberOfEntries();
         ossim_uint32 iE;
         for ( iE=0; iE<nEntries; ++iE )
         {
            ossimString entryPrefix = prefix + 
               ossimString( "entry" ) + 
               ossimString::toString( iE ) + ".";
            
            int nEntryFramesFound = 0;
            ossimRefPtr<ossimImageGeometry> geom = 0;
            const ossimRpfTocEntry* pEntry = pRpfToc->getTocEntry(iE);
            if ( pEntry != 0 )
            {
               const ossimRpfBoundaryRectRecord& rpfBRR =
                  pEntry->getBoundaryInformation();
               const ossimRpfCoverageSection& rpfCC = rpfBRR.getCoverage();
               
               ossim_float64 ul_lat = (ossim_float64)rpfCC.getUlLat();
               ossim_float64 ul_lon = (ossim_float64)rpfCC.getUlLon();
               ossim_float64 lr_lat = (ossim_float64)rpfCC.getLrLat();
               ossim_float64 lr_lon = (ossim_float64)rpfCC.getLrLon();
               
               // Try to print out ul and lr just as ossim-info outputs them.
               pRpf->setCurrentEntry(iE);
               ossimDrect outputRect = pRpf->getBoundingRect();

               geom = pRpf->getImageGeometry();
               if(geom.valid())
               {
                  ossimGpt ulg;
                  ossimGpt lrg;
                  geom->localToWorld(outputRect.ul(), ulg);
                  geom->localToWorld(outputRect.lr(), lrg);

                  ul_lat = ulg.latd();
                  ul_lon = ulg.lond();
                  lr_lat = lrg.latd();
                  lr_lon = lrg.lond();
               }

               // *** HACK *** OLK 06/2010 (same hack as in ossim-orthoigen.cpp)
               // Encountered CADRG RPF imagery where the left edge was longitude -180 and right 
               // edge +180. The projection code above reasonably maps all -180 to +180. This 
               // however breaks the image footprint since it would appear that the left and 
               // right edges were coincident instead of 360 degrees apart, i.e., a line segment 
               // instead of a rect. So added check here for 
               // coincident left and right edges and remapping left edge to -180.
               if ((ul_lon == 180.0) && (lr_lon == 180.0))  
               {
                  ul_lon = -180.0;
               }

               // Make sure we use the same number of digits precision as 
               // ossim-info (15), so we can do a per digit comparison.
               int prec = 15;
               ossimString ul_lat_str = ossimString::toString( ul_lat, prec );
               ossimString ul_lon_str = ossimString::toString( ul_lon, prec );
               ossimString lr_lat_str = ossimString::toString( lr_lat, prec );
               ossimString lr_lon_str = ossimString::toString( lr_lon, prec );

               ossim_uint32 bands = pRpf->getNumberOfOutputBands();
               
               out << setiosflags(std::ios::left)
                   << entryPrefix << std::setw(31) << "ul_lat:"
                   << ul_lat_str.c_str() << "\n"
                   << entryPrefix << std::setw(31) << "ul_lon:"
                   << ul_lon_str.c_str() << "\n"
                   << entryPrefix << std::setw(31) << "lr_lat:"
                   << lr_lat_str.c_str() << "\n"    
                   << entryPrefix << std::setw(31) << "lr_lon:"
                   << lr_lon_str.c_str() << "\n"
                   << entryPrefix << std::setw(31) << "bands:"
                   << ossimString::toString(bands) << "\n";
            } // if ( pEntry != 0 )
               
            nRows = pRpfToc->getNumberOfFramesVertical(iE);
            nCols = pRpfToc->getNumberOfFramesHorizontal(iE);
               
            ossim_uint32 iV,iH;
            ossim_uint32 rowIndex = nRows;
            for ( iV=0; iV<nRows; ++iV )
            {
               for ( iH=0; iH<nCols; ++iH )
               {
                  ossimFilename framePath =
                     pRpfToc->getRelativeFramePath(iE,iV,iH);
                     
                  if ( framePath.length() > 0 )
                  {
                     ossimString framePrefix = entryPrefix + 
                        ossimString( "frame" ) + 
                        ossimString::toString( nEntryFramesFound++ ) + ".";
                        
                     out << setiosflags(std::ios::left)
                         << framePrefix << std::setw(24) << "row:"
                         << iV << "\n"
                         << framePrefix << std::setw(24) << "col:"
                         << iH << "\n"
                         << framePrefix << std::setw(24) << "path:"
                         << framePath.c_str() << "\n";
                        
                     ++nTotalFramesFound;
                        
                     ossimString fullExt = framePath.ext();
                     ossimString seriesFromExt("");
                     if ( fullExt.length() > 2 )
                     {
                        seriesFromExt = fullExt.beforePos(2);
                     }
                        
                     const ossimRpfAttributes* pRpfAttr = 0;
                     ossimRpfFrame rpfFrame;
                     ossimFilename fullPath = rootDir.dirCat(framePath);
                     const char* pFileDateTime = 0;
                     ossimString acqDate("");
                     ossimGpt ulg;
                     ossimGpt llg;
                     ossimGpt lrg;
                     ossimGpt urg;
                     ossimRefPtr<ossimRpfReplaceUpdateTable> replaceUpdateTable = 0;
                     
                     if ( rpfFrame.parseFile(fullPath,true) == ossimErrorCodes::OSSIM_OK )
                     {
                        replaceUpdateTable = rpfFrame.getRpfReplaceUpdateTable();
                        if ( replaceUpdateTable.valid() )
                        {
                           replaceUpdateTable->print(out, framePrefix);
                        }
                        
                        pRpfAttr = rpfFrame.getAttributes();
                           
                        const ossimNitfFile* pNitfFile = rpfFrame.getNitfFile();
                        const ossimNitfFileHeader* pNitfFileHeader =
                           pNitfFile!=0 ? pNitfFile->getHeader() : 0;
                        pFileDateTime = pNitfFileHeader!=0 ? 
                           pNitfFileHeader->getDateTime() : 0;
                           
                        ossimNitfImageHeader* pNitfImageHeader = pNitfFile!=0 ? 
                           pNitfFile->getNewImageHeader(0) : 0;
                        if ( pNitfImageHeader != 0 )
                        {
                           acqDate = pNitfImageHeader->getImageDateAndTime();
                           ossim_uint32 cols = pNitfImageHeader->getNumberOfCols();
                           ossim_uint32 rows = pNitfImageHeader->getNumberOfRows();

                           ossimIrect imageRect = ossimIrect(cols*iH, rows*rowIndex-rows-1,
                                                             cols*iH+cols-1, rows*rowIndex);
                           if ( traceDebug() )
                           {
                              ossimNotify(ossimNotifyLevel_DEBUG)
                                 << "frame image rect: " << imageRect << "\n";
                           }

                           if (geom.valid())
                           {
                             geom->localToWorld(imageRect.ul(), ulg);
                             geom->localToWorld(imageRect.ll(), llg);
                             geom->localToWorld(imageRect.lr(), lrg);
                             geom->localToWorld(imageRect.ur(), urg);
                           }

                           // *** HACK *** OLK 06/2010 (same hack as in ossim-orthoigen.cpp)
                           // Encountered CADRG RPF imagery where the left edge was longitude
                           // -180 and right edge +180. The projection code above reasonably maps
                           // all -180 to +180. This however breaks the 
                           // image footprint since it would appear that the left and right edges
                           // were coincident instead of 360 degrees apart, i.e., a line segment
                           // instead of a rect. So added check here for coincident left and
                           // right edges and remapping left edge to -180.
                           if ((ulg.lon == 180.0) && (lrg.lon == 180.0))  
                           {
                              ulg.lon = -180.0;
                              llg.lon = -180.0;
                           }
                        }
                     }
                     else if ( traceDebug() )
                     {
                        ossimNotify(ossimNotifyLevel_WARN)
                           << "ERROR: Failed to parse frame image: "
                           << fullPath.c_str()
                           << std::endl;
                     }
                        
                     if ( pRpfAttr != 0 )
                     {
                        int prec = 15;
                        ossimString ul_lat_str = ossimString::toString(ulg.latd(), prec);
                        ossimString ul_lon_str = ossimString::toString(ulg.lond(), prec);
                        ossimString ll_lat_str = ossimString::toString(llg.latd(), prec);
                        ossimString ll_lon_str = ossimString::toString(llg.lond(), prec);
                        ossimString lr_lat_str = ossimString::toString(lrg.latd(), prec);
                        ossimString lr_lon_str = ossimString::toString(lrg.lond(), prec);
                        ossimString ur_lat_str = ossimString::toString(urg.latd(), prec);
                        ossimString ur_lon_str = ossimString::toString(urg.lond(), prec);
                        
                        out << setiosflags(std::ios::left)
                            << framePrefix << std::setw(24) << "ul_lat:"
                            << ul_lat_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ul_lon:"
                            << ul_lon_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ll_lat:"
                            << ll_lat_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ll_lon:"
                            << ll_lon_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "lr_lat:"
                            << lr_lat_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "lr_lon:"
                            << lr_lon_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ur_lat:"
                            << ur_lat_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ur_lon:"
                            << ur_lon_str.c_str() << "\n"
                            << framePrefix << std::setw(24) << "present:"
                            << "true" << "\n"
                            << framePrefix << std::setw(24) << "CurrencyDate:"
                            << pRpfAttr->theCurrencyDate << "\n"
                            << framePrefix << std::setw(24) << "ProductionDate:"
                            << pRpfAttr->theProductionDate << "\n"
                            << framePrefix << std::setw(24) << "SignificantDate:"
                            << pRpfAttr->theSignificantDate << "\n"
                            << framePrefix << std::setw(24) << "FileDateTime:"
                            << pFileDateTime << "\n"
                            << framePrefix << std::setw(24) << "AcquisitionDate:"
                            << acqDate << "\n"
                            << framePrefix << std::setw(24) << "SeriesCode:"
                            << seriesFromExt.c_str() << "\n"
                            << framePrefix << std::setw(24) << "ChartSeriesCode:"
                            << pRpfAttr->theChartSeriesCode << "\n"
                            << framePrefix << std::setw(24) << "MapDesignationCode:"
                            << pRpfAttr->theMapDesignationCode << "\n"
                            << framePrefix << std::setw(24) << "Edition:"
                            << pRpfAttr->theEdition << "\n"
                            << framePrefix << std::setw(24) << "DataSource:"
                            << pRpfAttr->theDataSource << "\n";
                     }
                     else
                     {
                        out << setiosflags(std::ios::left)
                            << framePrefix << std::setw(24) << "present:"
                            << "false" << "\n";
                     }
                  }
                  
               } // for ( iH=0; iH<nCols; ++iH )

               --rowIndex;
               
            } // for ( iV=0; iV<nRows; ++iV )
            
         } // for ( iE=0; iE<nEntries; ++iE )
            
         if ( nTotalFramesFound == 0 && traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimCibCadrgTileSource::printFrameList ERROR: "
               << "Found no frame files in input RPF image ("
               << imageFile << ")." 
               << std::endl;               
         }
      }
      else // the open failed
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "ossim-img2rr --list-frames ERROR: "
            << "Input image (" << imageFile << ") cannot be opened." 
            << std::endl;
      }
   }
      
   // Delete the image handler instance
   pRpf = 0;
   
} // End fo printFrameList


void framesToToc( const ossimFilename& dotRpfFile,
                  const ossimFilename& outputDir )
{
   const char MODULE[] = "ossim-rpf::framesToToc";

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\ndot_rpf_file:      " << dotRpfFile
         << "\noutput_directory:  " << outputDir
         << std::endl;
   }
   
   ossimRefPtr<ossimRpfToc> toc = new ossimRpfToc();

   try
   {
      toc->createTocAndCopyFrames( dotRpfFile, outputDir );
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
   }
}

void aDotTocToDotRpf( const ossimFilename& aDotTocFile,
                      const ossimFilename& outputDir )
{
   const char MODULE[] = "ossim-rpf::aDotTocToDotRpf";

   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " entered..."
         << "\na.toc file:       " << aDotTocFile
         << "\noutput directory: " << outputDir
         << std::endl;
   }
   
   ossimRefPtr<ossimRpfUtil> toc = new ossimRpfUtil();

   try
   {
      toc->writeDotRpfFiles( aDotTocFile, outputDir );
   }
   catch (const ossimException& e)
   {
      ossimNotify(ossimNotifyLevel_WARN) << e.what() << std::endl;
   }
}
