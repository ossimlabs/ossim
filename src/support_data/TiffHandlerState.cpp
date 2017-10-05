#include <ossim/support_data/TiffHandlerState.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/support_data/ossimGeoTiff.h>
#include <ossim/base/ossimTieGptSet.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/projection/ossimUtmpt.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimBilinearProjection.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/base/ossimGeoTiffCoordTransformsLut.h>
#include <ossim/base/ossimGeoTiffDatumLut.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/support_data/TiffStreamAdaptor.h>
#include <xtiffio.h>
#include <geo_normalize.h>
#include <geotiff.h>
#include <geovalues.h>
#include <cstddef> //nullptr
#include <sstream>
static ossimTrace traceDebug("ossim::TiffHandlerState");

static const ossimGeoTiffCoordTransformsLut COORD_TRANS_LUT;
static const ossimGeoTiffDatumLut DATUM_LUT;

const ossimString ossim::TiffHandlerState::m_typeName = "ossim::TiffHandlerState";

ossim::TiffHandlerState::TiffHandlerState()
{

}
      
ossim::TiffHandlerState::~TiffHandlerState()
{

}

const ossimString& ossim::TiffHandlerState::getTypeName()const
{
   return m_typeName;
}

const ossimString& ossim::TiffHandlerState::getStaticTypeName()
{
   return m_typeName;
}

void ossim::TiffHandlerState::addValue(const ossimString& key, 
                                       const ossimString& value)
{
  m_tags.add(key.c_str(), value.c_str());
}

bool ossim::TiffHandlerState::getValue(ossimString& value,
                                       const ossim_uint32 directory, 
                                        const ossimString& key)const
{
  return getValue(value, 
                  "image"+ossimString::toString(directory)+"."+key);
}

bool ossim::TiffHandlerState::getValue(ossimString& value,
                                       const ossimString& key)const
{
  bool result = false;

  const char* v = m_tags.find(key.c_str());
  if(v)
  {
    result = true;
    value  = v;
  }

  return result;
}

bool ossim::TiffHandlerState::exists(ossim_uint32 directory, const ossimString& key)const
{
  return exists(ossimString("image")+
                ossimString::toString(directory)+
                "."+key);

}
bool ossim::TiffHandlerState::exists(const ossimString& key)const
{
  return (m_tags.find(key)!= 0);

}

bool ossim::TiffHandlerState::checkBool(ossim_uint32 directory, const ossimString& key)const
{
  return checkBool(ossimString("image")+directory+"."+key);
}

bool ossim::TiffHandlerState::checkBool(const ossimString& key)const
{
  bool result = false;

  const char* value = m_tags.find(key);
  if(value)
  {
    result = ossimString(value).toBool();
  }
  return result;
}

bool ossim::TiffHandlerState::loadDefaults(const ossimFilename& file)
{
  bool result = false;
  std::shared_ptr<std::istream> tiffStream = ossim::StreamFactoryRegistry::instance()->createIstream(file);
  if(tiffStream)
  {
     TIFF* tiffPtr = XTIFFClientOpen(file.c_str(), "rm", 
                                  (thandle_t)tiffStream.get(),
                                  ossim::TiffIStreamAdaptor::tiffRead, 
                                  ossim::TiffIStreamAdaptor::tiffWrite, 
                                  ossim::TiffIStreamAdaptor::tiffSeek, 
                                  ossim::TiffIStreamAdaptor::tiffClose, 
                                  ossim::TiffIStreamAdaptor::tiffSize,
                                  ossim::TiffIStreamAdaptor::tiffMap, 
                                  ossim::TiffIStreamAdaptor::tiffUnmap);

     if(tiffPtr)
     {
       loadDefaults(tiffPtr);
       result = true;
       XTIFFClose(tiffPtr);
       tiffPtr = 0;

     }
  }

  return result;
}

void ossim::TiffHandlerState::loadDefaults(TIFF* tiffPtr)
{
  ossim_uint32  imageWidth=0;
  ossim_uint32  imageLength=0;
  ossim_int32   readMethod=0;
  ossim_uint16  planarConfig=0;
  ossim_uint16  photometric=0;
  ossim_uint32  rowsPerStrip=0;
  ossim_uint32  imageTileWidth=0;
  ossim_uint32  imageTileLength=0;
  ossim_uint32  imageDirectoryList=0;
  ossim_int32   compressionType=0;
  ossim_uint32  subFileType=0;
  ossim_uint16  bitsPerSample=0;
  ossim_uint16  samplesPerPixel=0;
  ossim_uint16  sampleFormatUnit=0;
  ossim_float64 sampleValue=0;
  ossim_uint16  sampleUintValue = 0;
  ossim_int64   currentDirectory = TIFFCurrentDirectory(tiffPtr);
  TIFFSetDirectory(tiffPtr, 0);

  ossim_int32   numberOfDirectories = TIFFNumberOfDirectories(tiffPtr);

  addValue("number_of_directories", ossimString::toString(numberOfDirectories));
  ossimString dirPrefix = "image"+ossimString::toString(TIFFCurrentDirectory(tiffPtr))+".";

  if(TIFFGetField(tiffPtr, TIFFTAG_COMPRESSION, &compressionType))
  {
    addValue(dirPrefix+"tifftag.compression", ossimString::toString(compressionType));
  } 

  if (TIFFGetField(tiffPtr,
                   TIFFTAG_SUBFILETYPE ,
                   &subFileType ) )
  {
    addValue(dirPrefix+"tifftag.sub_file_type", ossimString::toString(subFileType));
  }

  if( TIFFGetField(tiffPtr, TIFFTAG_BITSPERSAMPLE, &bitsPerSample) )
  {
    addValue(dirPrefix+"tifftag.bits_per_sample", ossimString::toString(bitsPerSample));
  }
  if( TIFFGetField(tiffPtr,
                   TIFFTAG_SAMPLESPERPIXEL,
                   &samplesPerPixel ) )
  {
    addValue(dirPrefix+"tifftag.samples_per_pixel", ossimString::toString(samplesPerPixel));
  }

  if ( TIFFGetField( tiffPtr,
                     TIFFTAG_SAMPLEFORMAT,
                     &sampleFormatUnit ) )
  {
    addValue(dirPrefix+"tifftag.sample_format", ossimString::toString(sampleFormatUnit));
  }

  if (TIFFGetField(tiffPtr,
                   TIFFTAG_SMAXSAMPLEVALUE,
                    &sampleValue ) )
  {
    addValue(dirPrefix+"tifftag.max_sample_value", ossimString::toString(sampleValue));
  }
  else
  {
    if(TIFFGetField( tiffPtr,
                      TIFFTAG_MAXSAMPLEVALUE,
                      &sampleUintValue))
    {
      addValue(dirPrefix+"tifftag.max_sample_value", ossimString::toString(sampleUintValue));
    }

  }

  if ( TIFFGetField( tiffPtr,
                     TIFFTAG_SMINSAMPLEVALUE,
                     &sampleValue ) )
  {
    addValue(dirPrefix+"tifftag.min_sample_value", ossimString::toString(sampleValue));
  }
  else
  {
    if(TIFFGetField( tiffPtr,
                      TIFFTAG_MINSAMPLEVALUE,
                      &sampleUintValue))
    {
      addValue(dirPrefix+"tifftag.min_sample_value", ossimString::toString(sampleUintValue));
    }
  }

  ossim_int32 idx=0;
  for(;idx < numberOfDirectories;++idx)
  {
    if (!TIFFSetDirectory(tiffPtr, idx))
    {
       break;
    }
    loadGeotiffTags(tiffPtr, dirPrefix);
    ossimString dirPrefix = "image"+ossimString::toString(idx)+".";

      
    // lines:
    if ( TIFFGetField( tiffPtr,
                        TIFFTAG_IMAGELENGTH,
                        &imageLength ) )
    {
      addValue(dirPrefix+"tifftag.image_length", 
               ossimString::toString(imageLength));
    }

    // samples:
    if ( TIFFGetField( tiffPtr,
                        TIFFTAG_IMAGEWIDTH,
                        &imageWidth ) )
    {
      addValue(dirPrefix+"tifftag.image_width", 
               ossimString::toString(imageWidth));
    }
    
    if ( TIFFGetField( tiffPtr,
                       TIFFTAG_SUBFILETYPE ,
                       &subFileType ) )
    {
      addValue(dirPrefix+"tifftag.sub_file_type", 
               ossimString::toString(subFileType));
    }

    if( TIFFGetField( tiffPtr, TIFFTAG_PLANARCONFIG,
                       &planarConfig ) )
    {
      addValue(dirPrefix+"tifftag.planar_config", 
               ossimString::toString(planarConfig));
    }
    
    if( TIFFGetField( tiffPtr, TIFFTAG_PHOTOMETRIC,
                       &photometric ) )
    {
      addValue(dirPrefix+"tifftag.photometric", 
               ossimString::toString(photometric));
    }
    // Check for palette.
    ossim_uint16* red;
    ossim_uint16* green;
    ossim_uint16* blue;
    if(TIFFGetField(tiffPtr, TIFFTAG_COLORMAP, &red, &green, &blue))
    {
      if(bitsPerSample)
      {
        saveColorMap(dirPrefix, red, green, blue, 1<<bitsPerSample);
      }
    }

    if( TIFFIsTiled(tiffPtr))
    {
      addValue(dirPrefix+"tiff_is_tiled", "true");
      if ( TIFFGetField( tiffPtr,
                         TIFFTAG_TILEWIDTH,
                         &imageTileWidth ) )
      {
        addValue(dirPrefix+"tifftag.tile_width", 
                 ossimString::toString(imageTileWidth));
      }
      if ( TIFFGetField( tiffPtr,
                         TIFFTAG_TILELENGTH,
                         &imageTileLength ) )
      {
        addValue(dirPrefix+"tifftag.tile_length", 
                 ossimString::toString(imageTileLength));
      }
    }
    else
    {
      addValue(dirPrefix+"tiff_is_tiled", "false");
      if( TIFFGetField( tiffPtr, TIFFTAG_ROWSPERSTRIP,
                        &rowsPerStrip ) )
      {
        addValue(dirPrefix+"tifftag.rows_per_strip", 
                 ossimString::toString(rowsPerStrip));
      }
    }
  }
  TIFFSetDirectory(tiffPtr, currentDirectory);
}

ossimRefPtr<ossimProjection> ossim::TiffHandlerState::createProjection(ossim_int32 directory)const
{
  ossimRefPtr<ossimProjection> result;
  ossimKeywordlist kwl;
  ossimString tempStr;
  std::vector<ossim_float64> geoPixelScale;
  std::vector<ossim_float64> geoDoubleParams;
  std::vector<ossim_float64> geoTiePoints;
  std::vector<ossim_float64> geoTransMatrix;
  bool usingModelTransform = false;

  getGeoPixelScale(geoPixelScale, directory);
  getGeoTiePoints(geoTiePoints, directory);
  getGeoDoubleParams(geoDoubleParams, directory);
  getGeoTransMatrix(geoTransMatrix, directory);

  usingModelTransform = ((geoTransMatrix.size() == 16)&&
                        (geoPixelScale.empty())&&
                        (geoTiePoints.size()<24)); // 4 tie points <x,y,z,lat,lon,height>

  if ( (!usingModelTransform) &&
       (geoPixelScale.size() < 2) && // no scale
       ( geoTiePoints.size() < 24) )//need at least 3 ties if no scale.
  {
    return result;
  }
  double xTiePoint = 0.0;
  double yTiePoint = 0.0;
  ossim_uint32 tieCount = (ossim_uint32)geoTiePoints.size()/6;

  if( (geoPixelScale.size() == 3) && (tieCount == 1))
  {
    //---
    // Shift the tie point to the (0, 0) position if it's not already.
    //
    // Note:
    // Some geotiff writers like ERDAS IMAGINE set the "GTRasterTypeGeoKey"
    // key to RasterPixelIsArea, then set the tie point to (0.5, 0.5).
    // This really means "RasterPixelIsPoint" with a tie point of (0.0, 0.0).
    // Anyway we will check for this blunder and attempt to do the right
    // thing...
    //---
    xTiePoint = geoTiePoints[3] - geoTiePoints[0]*geoPixelScale[0];
    yTiePoint = geoTiePoints[4] + geoTiePoints[1]*geoPixelScale[1];
  }
  else if(tieCount > 1)
  {
    //---
    // Should we check the model type??? (drb)
    // if (theModelType == ModelTypeGeographic)
    //---
    if(tieCount >= 4)
    {
       ossimTieGptSet tieSet;
       getTieSet(tieSet, directory, geoTiePoints);

       if(tieCount > 4)
       {
          // create a cubic polynomial model
          //ossimRefPtr<ossimPolynomProjection> proj = new ossimPolynomProjection;
          //proj->setupOptimizer("1 x y x2 xy y2 x3 y3 xy2 x2y z xz yz");
          ossimRefPtr<ossimBilinearProjection> proj = new ossimBilinearProjection;
          proj->optimizeFit(tieSet);
          result = proj.get();
          if(traceDebug())
          {
             ossimNotify(ossimNotifyLevel_DEBUG)
                << "ossimGeoTiff::addImageGeometry: "
                << "Creating a Cubic polynomial projection" << std::endl;
          }
          return result;            
       }
       else if(tieCount == 4)
       {
          // create a bilinear model
          // Should we check the model type (drb)
          // if (theModelType == ModelTypeGeographic)
          
          ossimRefPtr<ossimBilinearProjection> proj = new ossimBilinearProjection;
          proj->optimizeFit(tieSet);
          //proj->saveState(kwl, prefix);
          result = proj.get();
          if(traceDebug())
          {
             ossimNotify(ossimNotifyLevel_DEBUG)
                << "ossimGeoTiff::addImageGeometry: "
                << "Creating a bilinear projection" << std::endl;
          }
         return result;
       }
       else
       {
          ossimNotify(ossimNotifyLevel_WARN)
             << "ossimGeoTiff::addImageGeometry: "
             << "Not enough tie points to create a interpolation model"
             <<std::endl;
       }
       result = 0;
       return result;
    }
  }
  else if (usingModelTransform)
  {
    if(traceDebug())
    {
       ossimNotify(ossimNotifyLevel_WARN)
          << "ossimGeoTiff::addImageGeometry: Do not support rotated "
          << "map models yet.  You should provide the image as a sample "
          << "and we will fix it" << std::endl;
    }
  }
  ossim_uint32 rasterType = getRasterType(directory);
  if (rasterType == ossimGeoTiff::PIXEL_IS_AREA)
  {
    // Since the internal pixel representation is "point", shift the
    // tie point to be relative to the center of the pixel.
    if (geoPixelScale.size() > 1)
    {
       xTiePoint += (geoPixelScale[0])/2.0;
       yTiePoint -= (geoPixelScale[1])/2.0;
    }
  }
  ossim_uint32 pcsCode = getPcsCode(directory);

  if( pcsCode && (pcsCode != ossimGeoTiff::USER_DEFINED) )
  {
    ossimString epsg_spec ("EPSG:");
    epsg_spec += ossimString::toString(pcsCode);
    ossimRefPtr<ossimProjection> tempProj = ossimEpsgProjectionFactory::instance()->createProjection(epsg_spec);
    if(tempProj)
    {
      tempProj->saveState(kwl);      
    }
  }
  ossimString projName;
  ossimString datumName;
  if(getOssimProjectionName(projName, directory))
  {
    kwl.add(ossimKeywordNames::TYPE_KW, projName);
    if(getOssimDatumName(datumName, directory))
    {
      kwl.add(ossimKeywordNames::DATUM_KW, datumName);
    }
  }
  ossim_int32 modelType    = getModelType(directory);
  ossim_int32 angularUnits = getAngularUnits(directory);

  ossim_float64 originLat = getOriginLat(directory);
  ossim_float64 originLon = getOriginLon(directory);

  if (modelType == ossimGeoTiff::MODEL_TYPE_GEOGRAPHIC)
  {
    if (angularUnits != ossimGeoTiff::ANGULAR_DEGREE)
    {
       ossimNotify(ossimNotifyLevel_WARN)
          << "WARNING ossim::TiffHandlerState::createProjection:"
          << "\nNot coded yet for unit type:  "
          << angularUnits << endl;
       result = 0;

       return result;
    }        

    //---
    // Tiepoint
    // Have data with tie points -180.001389 so use ossimGpt::wrap() to handle:
    //---
    ossimGpt tieGpt(xTiePoint, yTiePoint, 0.0);
    tieGpt.wrap();
    ossimDpt tiepoint(tieGpt);
    kwl.add(ossimKeywordNames::TIE_POINT_XY_KW, tiepoint.toString(), true);
    kwl.add(ossimKeywordNames::TIE_POINT_UNITS_KW, "degrees", true);
    ossim_uint64 h=getImageLength(directory);
    ossim_uint64 w=getImageWidth(directory);
    // scale or gsd
    if (geoPixelScale.size() > 1)
    {
      ossimDpt scale (geoPixelScale[0], geoPixelScale[1]);
      kwl.add(ossimKeywordNames::PIXEL_SCALE_XY_KW, scale.toString(), true);
      kwl.add(ossimKeywordNames::PIXEL_SCALE_UNITS_KW, "degrees", true);
      if(ossim::isnan(originLat))
      {
        double centerY = h/2.0;
        originLat = tieGpt.lat - geoPixelScale[1]*centerY;

      }
      if (  ossim::isnan(originLon) )
      {
        originLon = 0.0;
      }
      if (!(ossim::isnan(originLat) || ossim::isnan(originLon)))
      {
         kwl.add(ossimKeywordNames::ORIGIN_LATITUDE_KW,  originLat, true);
         kwl.add(ossimKeywordNames::CENTRAL_MERIDIAN_KW, originLon, true);
      }
    }
  }
  else // Projected
  {
    ossim_int32 linearUnits = getLinearUnits(directory);
    if(linearUnits != ossimGeoTiff::UNDEFINED)
    {
      linearUnits = tempStr.toInt32();
      ossimDpt tiepoint (convert2meters(linearUnits, xTiePoint),
                         convert2meters(linearUnits, yTiePoint));
      kwl.add(ossimKeywordNames::TIE_POINT_XY_KW, tiepoint.toString(), true);
      kwl.add(ossimKeywordNames::TIE_POINT_UNITS_KW, "meters", true);

    }
    else
    {
      result = 0;

      return result;        
    }
    if (geoPixelScale.size() > 1)
    {
       ossimDpt scale (convert2meters(linearUnits, geoPixelScale[0]), convert2meters(linearUnits, geoPixelScale[1]));
       kwl.add(ossimKeywordNames::PIXEL_SCALE_XY_KW, scale.toString(), true);
       kwl.add(ossimKeywordNames::PIXEL_SCALE_UNITS_KW, "meters", true);
    }
    // origin
    if(!ossim::isnan(originLat))
    {
       kwl.add(ossimKeywordNames::ORIGIN_LATITUDE_KW, originLat);        
    }
    if(!ossim::isnan(originLon))
    {
      kwl.add(ossimKeywordNames::CENTRAL_MERIDIAN_KW, originLon);
    }
    kwl.add(ossimKeywordNames::STD_PARALLEL_1_KW, getStandardParallel1(directory));
    kwl.add(ossimKeywordNames::STD_PARALLEL_2_KW, getStandardParallel2(directory));
    
    // false easting and northing.
    kwl.add(ossimKeywordNames::FALSE_EASTING_KW, convert2meters(linearUnits, getFalseEasting(directory)));
    kwl.add(ossimKeywordNames::FALSE_NORTHING_KW, convert2meters(linearUnits, getFalseNorthing(directory)));

    ossimUtmpt utmPt(ossimGpt(originLat, originLon));
    // Based on projection type, override/add the appropriate info.
    if (projName == "ossimUtmProjection")
    {
       // Check the zone before adding...
       kwl.add(ossimKeywordNames::ZONE_KW, ossimString::toString(utmPt.zone()), true);
       kwl.add(ossimKeywordNames::HEMISPHERE_KW, utmPt.hemisphere(), true);

       //---
       // Must set the central meridian even though the zone should do it.
       // (in decimal degrees)
       //---
       double centralMeridian = ( 6.0 * abs(utmPt.zone()) ) - 183.0;
       kwl.add(ossimKeywordNames::CENTRAL_MERIDIAN_KW, centralMeridian, true);
       kwl.add(ossimKeywordNames::ORIGIN_LATITUDE_KW, 0.0, true);

    } // End of "if (UTM)"
    else if (projName == "ossimTransMercatorProjection")
    {
      ossim_float64 scaleFactor = getScaleFactor(directory);
      if(!ossim::isnan(scaleFactor))
       kwl.add(ossimKeywordNames::SCALE_FACTOR_KW, scaleFactor, true); 
    }
  }

  if(kwl.getSize())
  {
    result = ossimProjectionFactoryRegistry::instance()->createProjection(kwl);
  }
  return result;
}

void ossim::TiffHandlerState::saveColorMap(const ossimString& dirPrefix,
                                            const ossim_uint16* red, 
                                            const ossim_uint16* green,
                                            const ossim_uint16* blue,
                                            ossim_uint32 numberOfEntries)
{
  ossim_uint32 idx = 0;
  std::ostringstream redStream;
  std::ostringstream greenStream;
  std::ostringstream blueStream;
  redStream << "(";
  greenStream << "(";
  blueStream << "(";  
  for(;idx < (numberOfEntries-1);++idx,++red,++green,++blue)
  {
    redStream   << *red << ",";
    greenStream << *green << ",";
    blueStream  << *blue << ",";
    if((idx%16)==0)
    {
      redStream   << "\n";
      greenStream << "\n";
      blueStream  << "\n";      
    }
  }  
  redStream   << *red << ")";
  greenStream << *green << ")";
  blueStream  << *blue << ")";
  
  addValue(dirPrefix+"tifftag.colormap.red", 
           redStream.str());
  addValue(dirPrefix+"tifftag.colormap.green", 
           greenStream.str());
  addValue(dirPrefix+"tifftag.colormap.blue", 
           blueStream.str());
}

void ossim::TiffHandlerState::loadGeotiffTags(TIFF* tiffPtr,
                                              const ossimString& dirPrefix )
{
  const int CITATION_STRING_SIZE = 512;
  ossim_uint16 rasterType = 0;
  ossim_uint16 pcsCode = 0;
  ossim_uint16 coordTransGeoCode = 0;
  ossim_uint32 idx = 0;
  GTIF* gtif = GTIFNew(tiffPtr);
  std::vector<char> citationStr(CITATION_STRING_SIZE);
  char* citationStrPtr = &citationStr.front();
  char* buf = 0;
  ossim_uint16 doubleArraySize = 0;
  double* doubleArray=0;
  ossimString doubleArrayStr;

  if(!gtif) return;

  // need to add tru for geotiff flag

  // now load tags
  //
  std::shared_ptr<GTIFDefn> defs = std::make_shared<GTIFDefn>();

  GTIFGetDefn(gtif, defs.get());

  addValue(dirPrefix+"tifftag.model_type", 
           ossimString::toString(defs->Model));
  addValue(dirPrefix+"tifftag.gcs_code", 
           ossimString::toString(defs->GCS));
  addValue(dirPrefix+"tifftag.pcs_code", 
           ossimString::toString(defs->PCS));
  addValue(dirPrefix+"tifftag.datum_code", 
           ossimString::toString(defs->Datum));
  addValue(dirPrefix+"tifftag.angular_units", 
           ossimString::toString(defs->UOMAngle));
  addValue(dirPrefix+"tifftag.linear_units", 
           ossimString::toString(defs->UOMLength));
  
  if(GTIFKeyGet(gtif, GTRasterTypeGeoKey, &rasterType, 0, 1))
  {
    addValue(dirPrefix+"tifftag.raster_type", 
             ossimString::toString(rasterType));

  }
  if (GTIFKeyGet(gtif, ProjectedCSTypeGeoKey, &pcsCode, 0, 1))
  {
    addValue(dirPrefix+"tifftag.pcs_code", 
             ossimString::toString(pcsCode));
  }

  if ( GTIFKeyGet(gtif, GTCitationGeoKey, &citationStrPtr ,
                  0, CITATION_STRING_SIZE))
  {
    addValue(dirPrefix+"tifftag.citation", 
             citationStrPtr);
  }
  if(GTIFKeyGet(gtif, PCSCitationGeoKey , &buf, 0, 1))
  {
    addValue(dirPrefix+"tifftag.pcs_citation", 
             buf);
  }
  if(GTIFKeyGet(gtif, ProjCoordTransGeoKey , &coordTransGeoCode, 0, 1))
  {
    addValue(dirPrefix+"tifftag.coord_trans_code", 
             ossimString::toString(coordTransGeoCode));
  }

  for(idx = 0; idx < (ossim_uint32)(defs->nParms); ++idx)
  {
    switch(defs->ProjParmId[idx])
    {
      case ProjStdParallel1GeoKey:
      {
        addValue(dirPrefix+"tifftag.std_parallel_1", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjStdParallel2GeoKey:
      {
        addValue(dirPrefix+"tifftag.std_parallel_2", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjOriginLongGeoKey:
      {
        addValue(dirPrefix+"tifftag.origin_lon", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjOriginLatGeoKey:
      {
        addValue(dirPrefix+"tifftag.origin_lat", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjFalseEastingGeoKey:
      {
        addValue(dirPrefix+"tifftag.false_easting", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjFalseNorthingGeoKey:
      {
        addValue(dirPrefix+"tifftag.false_northing", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjCenterLongGeoKey:
      {
        addValue(dirPrefix+"tifftag.center_lon", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjCenterLatGeoKey:
      {
        addValue(dirPrefix+"tifftag.center_lat", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjFalseOriginLatGeoKey:
      {
        addValue(dirPrefix+"tifftag.origin_lat", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjFalseOriginLongGeoKey:
      case ProjStraightVertPoleLongGeoKey:
      {
        addValue(dirPrefix+"tifftag.origin_lon", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
      case ProjScaleAtNatOriginGeoKey:
      {
        addValue(dirPrefix+"tifftag.scale_factor", 
                ossimString::toString(defs->ProjParm[idx]));
        break;
      }
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOPIXELSCALE, &doubleArraySize, &doubleArray))
  {
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_pixel_scale", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOTIEPOINTS,  &doubleArraySize, &doubleArray))
  {
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_tie_points", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEODOUBLEPARAMS, &doubleArraySize, &doubleArray))
  {
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_double_params", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOTRANSMATRIX, &doubleArraySize, &doubleArray))
  {
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_trans_matrix", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOASCIIPARAMS, &buf))
  {
    addValue(dirPrefix+"tifftag.geo_ascii_params", buf);
  }
  GTIFFree(gtif);
}

void ossim::TiffHandlerState::convertArrayToStringList(ossimString& result, double* doubleArray, ossim_int32 doubleArraySize)const
{
  ossim_int32 idx = 0;
  result = "";
  if(doubleArray && (doubleArraySize > 0))
  {
    ossim_int32 precision = 20;
    std::ostringstream doubleArrayStream;
    doubleArrayStream << "(";
    for(idx = 0; idx < doubleArraySize-1;++idx)
    {
      doubleArrayStream << ossimString::toString(doubleArray[idx],20) << ",";
    }
    doubleArrayStream << ossimString::toString(doubleArray[idx],20) << ")";
    result = doubleArrayStream.str();
  }
}
void ossim::TiffHandlerState::getTieSet(ossimTieGptSet& tieSet,
                                        ossim_uint32 directory,
                                        const std::vector<ossim_float64>& tiePoints) const
{
   ossim_uint32 idx = 0;
   ossim_uint32 tieCount = (ossim_uint32)tiePoints.size()/6;
   const double* tiePointsPtr = &tiePoints.front();
   double offset = 0;
   if (hasOneBasedTiePoints(directory, tiePoints))
   {
      offset = -1.0;
   }
   
   for(idx = 0; idx < tieCount; ++idx)
   {
      ossimDpt xyPixel(tiePointsPtr[0]+offset, tiePointsPtr[1]+offset);
      // tie[3] = x, tie[4]
      ossimGpt gpt(tiePointsPtr[4], tiePointsPtr[3], tiePointsPtr[5]);
      
      tieSet.addTiePoint(new ossimTieGpt(gpt, xyPixel, .5));
      tiePointsPtr+=6;
   }
}

bool ossim::TiffHandlerState::hasOneBasedTiePoints(ossim_uint32 directory,
                                                   const std::vector<ossim_float64>& tiePoints) const
{
   bool result = false;
   
   // Assuming ties of (x,y,z,lat,lon,hgt) so size should be divide by 3.
   if (tiePoints.size()%6)
   {
      return result;
   }
   
   ossim_float64 minX = 999999.0;
   ossim_float64 minY = 999999.0;
   ossim_float64 maxX = 0.0;
   ossim_float64 maxY = 0.0;

   const ossim_uint32 SIZE = (ossim_uint32)tiePoints.size();
   ossim_uint32 tieIndex = 0;

   while (tieIndex < SIZE)
   {
      if ( tiePoints[tieIndex]   < minX ) minX = tiePoints[tieIndex];
      if ( tiePoints[tieIndex]   > maxX ) maxX = tiePoints[tieIndex];
      if ( tiePoints[tieIndex+1] < minY ) minY = tiePoints[tieIndex+1];
      if ( tiePoints[tieIndex+1] > maxY ) maxY = tiePoints[tieIndex+1];
      tieIndex += 6;
   }
   ossimString tempStr;
   ossim_uint64 w = 0;
   ossim_uint64 h = 0;
   if(getValue(tempStr, directory, "tifftag.image_width"))
   {
      w = tempStr.toUInt64();
   }
   if(getValue(tempStr, directory, "tifftag.image_height"))
   {
      h = tempStr.toUInt64();
   }
   if ( (minX == 1) && (maxX == w) &&
        (minY == 1) && (maxY == h) )
   {
      result = true;
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossim::TiffHandlerState DEBUG:"
         << "\nminX:       " << minX
         << "\nmaxX:       " << maxX
         << "\nminY:       " << minY
         << "\nmaxY:       " << maxY
         << "\ntheWidth:   " << w
         << "\ntheLength:  " << h
         << "\none based:  " << (result?"true":"false")
         << std::endl;
   }
   
   return result;
}

bool ossim::TiffHandlerState::getOssimProjectionName(ossimString& projName, ossim_uint32 directory) const
{
  bool result = false;
  ossimString tempStr;
  projName="unknown";
  if(getValue(tempStr, directory, "tifftag.coord_trans_code"))
  {
    ossim_uint16 coordTransGeoCode = tempStr.toUInt16();
    //---
    // The "parsePcsCode" method can also set the projection name.  So check
    // it prior to looking in the lookup table.
    //---
    projName =  COORD_TRANS_LUT.getEntryString(coordTransGeoCode);

  }

   // If still unknown check for the model type.
   if (projName == "unknown")
   {
      ossim_int32 modelType;
      if(getValue(tempStr, directory, "tifftag.model_type"))
      {
        modelType = tempStr.toInt32();
        if(modelType == ModelTypeGeographic)
        {
           projName = "ossimEquDistCylProjection";
           result = true;
        }

      }
   }
   else
   {
    result = true;
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "DEBUG ossim::TiffHandlerState::getOssimProjectionName: "
         << "projName:  "
         << projName
         << std::endl;
   }

   return result;
}

bool ossim::TiffHandlerState::getOssimDatumName(ossimString& datumName, ossim_uint32 directory) const
{
  bool result = false;
  ossimString tempStr;
  datumName="unknown";
  if(getValue(tempStr, directory, "tifftag.datum_code"))
  {
    //---
    // The "parsePcsCode" method can also set the projection name.  So check
    // it prior to looking in the lookup table.
    //---
    datumName =  COORD_TRANS_LUT.getEntryString(tempStr.toInt32());

  }
  if(datumName.empty()||(datumName == "unknown"))
  {
    if(getValue(tempStr, directory, "tifftag.gcs_code"))
    {
         datumName = DATUM_LUT.getEntryString(tempStr.toInt32());
    }
  }

  result = (!datumName.empty())&&(datumName!="unknown");
  if (traceDebug())
  {
    ossimNotify(ossimNotifyLevel_DEBUG)
       << "DEBUG ossim::TiffHandlerState::getOssimDatumName: "
       << "datumName:  "
       << datumName
       << "\n";
  }

   return result;
}

double ossim::TiffHandlerState::convert2meters(ossim_int32 units,
                                              double d) const
{
   switch(units)
   {
   case ossimGeoTiff::LINEAR_METER:
      return d;
   case ossimGeoTiff::LINEAR_FOOT:
      return ossim::ft2mtrs(d);
   case ossimGeoTiff::LINEAR_FOOT_US_SURVEY:
      return ossim::usft2mtrs(d);
   default:
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "DEBUG ossim::TiffHandlerState::convert2meters: " 
            << "Linear units code was undefined!\n"
            << "No conversion was performed." << std::endl;
      }
      break;
   }
   
   return d;
}


ossim_int32 ossim::TiffHandlerState::getModelType(ossim_int32 directory)const
{
  ossimString tempStr;
  if(getValue(tempStr, directory, "tifftag.model_type"))
  {
    return tempStr.toInt32();
  }

  return ossimGeoTiff::UNDEFINED;    
}

ossim_int32 ossim::TiffHandlerState::getAngularUnits(ossim_int32 directory)const
{
  ossimString tempStr;
  if(getValue(tempStr, directory, "tifftag.angular_units"))
  {
    return tempStr.toInt32();
  }

  return ossimGeoTiff::UNDEFINED;    
}
ossim_int32 ossim::TiffHandlerState::getLinearUnits(ossim_int32 directory)const
{
  ossimString tempStr;
  if(getValue(tempStr, directory, "tifftag.linear_units"))
  {
    return tempStr.toInt32();
  }

  return ossimGeoTiff::UNDEFINED;    
}
ossim_int32 ossim::TiffHandlerState::getRasterType(ossim_int32 directory)const
{
  ossimString tempStr;
  if(getValue(tempStr, directory, "tifftag.raster_type"))
  {
    return tempStr.toInt32();
  }

  return ossimGeoTiff::UNDEFINED;    
}
ossim_int32 ossim::TiffHandlerState::getPcsCode(ossim_int32 directory)const
{
  ossimString tempStr;
  if(getValue(tempStr, directory, "tifftag.pcs_code"))
  {
    return tempStr.toInt32();
  }

  return ossimGeoTiff::UNDEFINED;    
}

ossim_float64 ossim::TiffHandlerState::getDoubleValue(const ossimString& key, 
                                                      ossim_int32 directory)const
{
  ossimString tempStr;
  
  if(getValue(tempStr, directory, key))
  {
    return tempStr.toDouble();
  }
  
  return ossim::nan();  
}

ossim_float64 ossim::TiffHandlerState::getOriginLat(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.origin_lat", directory);  
}

ossim_float64 ossim::TiffHandlerState::getOriginLon(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.origin_lon", directory);  
}

ossim_float64 ossim::TiffHandlerState::getStandardParallel1(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.std_parallel_1", directory);  
}

ossim_float64 ossim::TiffHandlerState::getStandardParallel2(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.std_parallel_2", directory);  
}

ossim_float64 ossim::TiffHandlerState::getFalseEasting(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.false_easting", directory);  
}

ossim_float64 ossim::TiffHandlerState::getFalseNorthing(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.false_northing", directory);  

}

ossim_float64 ossim::TiffHandlerState::getScaleFactor(ossim_int32 directory)const
{
  return getDoubleValue("tifftag.scale_factor", directory);  
}

ossim_int64 ossim::TiffHandlerState::getImageLength(ossim_int32 directory)const
{
  ossimString tempStr;

  if(getValue(tempStr, directory, "tifftag.image_length"))
  {
    return tempStr.toInt64();
  }

  return 0;
}

ossim_int64 ossim::TiffHandlerState::getImageWidth(ossim_int32 directory)const
{
  ossimString tempStr;

  if(getValue(tempStr, directory, "tifftag.image_width"))
  {
    return tempStr.toInt64();
  }

  return 0;
}

bool ossim::TiffHandlerState::getDoubleArray(std::vector<ossim_float64>& result, 
                                              ossim_int32 directory,
                                              const ossimString& key)const
{
  ossimString tempStr;

  if(getValue(tempStr, directory, key))
  {
    if(!tempStr.empty())
    {
      if(!toSimpleVector(result, tempStr))
      {
        result.clear();
      }
      tempStr="";
    }
  }

  return !result.empty();

}

bool ossim::TiffHandlerState::getGeoDoubleParams(std::vector<ossim_float64>& result,
                                                 ossim_int32 directory)const
{
  return getDoubleArray(result, directory, "tifftag.geo_double_params");
}

bool ossim::TiffHandlerState::getGeoPixelScale(std::vector<ossim_float64>& result,
                                                 ossim_int32 directory)const
{
  return getDoubleArray(result, directory, "tifftag.geo_pixel_scale");
}

bool ossim::TiffHandlerState::getGeoTiePoints(std::vector<ossim_float64>& result,
                                              ossim_int32 directory)const
{
  return getDoubleArray(result, directory, "tifftag.geo_tie_points");
}

bool ossim::TiffHandlerState::getGeoTransMatrix(std::vector<ossim_float64>& result,
                                              ossim_int32 directory)const
{
  return getDoubleArray(result, directory, "tifftag.geo_trans_matrix");
}

void ossim::TiffHandlerState::load(const ossimKeywordlist& kwl,
                                   const ossimString& prefix)
{
  ossim::ImageHandlerState::load(kwl, prefix);
  m_tags.clear();

  kwl.extractKeysThatMatch(m_tags, "^("+prefix+"image[0-9]+)");
  if(!prefix.empty())
  {
    m_tags.stripPrefixFromAll("^("+prefix+")");
  }
  ossimString numberOfDirectories = kwl.find(prefix, "number_of_directories");

  if(!numberOfDirectories.empty()) m_tags.add("number_of_directories", numberOfDirectories, true);
}

void ossim::TiffHandlerState::save(ossimKeywordlist& kwl,
                                   const ossimString& prefix)const
{
   ossim::ImageHandlerState::save(kwl, prefix);
   kwl.add(prefix.c_str(), m_tags);
}
