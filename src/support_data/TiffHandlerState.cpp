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
#include <cpl_serv.h>
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
                  "dir"+ossimString::toString(directory)+"."+key);
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
  return exists("dir"+
                ossimString::toString(directory)+
                "."+key);

}
bool ossim::TiffHandlerState::exists(const ossimString& key)const
{
  return (m_tags.find(key)!= 0);

}

bool ossim::TiffHandlerState::checkBool(ossim_uint32 directory, const ossimString& key)const
{
  return checkBool("dir"+ossimString::toString(directory)+"."+key);
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

bool ossim::TiffHandlerState::loadDefaults(const ossimFilename& file, 
                                           ossim_uint32 entry)
{
  bool result = ImageHandlerState::loadDefaults(file, entry);
  
  if(result)
  {
    std::shared_ptr<std::istream> tiffStream = ossim::StreamFactoryRegistry::instance()->createIstream(file);
    if(tiffStream)
    {
       std::shared_ptr<ossim::TiffIStreamAdaptor> streamAdaptor = std::make_shared<ossim::TiffIStreamAdaptor>(tiffStream,
                                                                   file.c_str());
       TIFF* tiffPtr = XTIFFClientOpen(file.c_str(), "rm", 
                                    (thandle_t)streamAdaptor.get(),
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
  }

  return result;
}

void ossim::TiffHandlerState::loadDefaults(TIFF* tiffPtr)
{
  ossim_int64   currentDirectory = TIFFCurrentDirectory(tiffPtr);
  TIFFSetDirectory(tiffPtr, 0);
  ossim_int32   numberOfDirectories = TIFFNumberOfDirectories(tiffPtr);

  addValue("number_of_directories", ossimString::toString(numberOfDirectories));

  ossim_int32 idx=0;
  for(;idx < numberOfDirectories;++idx)
  {
    if (!TIFFSetDirectory(tiffPtr, idx))
    {
       break;
    }
    else
    {
      loadDirectory(tiffPtr, idx);
    }
  }
  TIFFSetDirectory(tiffPtr, currentDirectory);
}


void ossim::TiffHandlerState::loadCurrentDirectory(TIFF* tiffPtr)
{
  if(tiffPtr) 
  {
    loadDirectory(tiffPtr, TIFFCurrentDirectory(tiffPtr));
  }
}

void ossim::TiffHandlerState::loadDirectory(TIFF* tiffPtr, 
                                            ossim_uint32 directory)
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

  ossim_int32   numberOfDirectories = TIFFNumberOfDirectories(tiffPtr);

  addValue("number_of_directories", ossimString::toString(numberOfDirectories));

  if(TIFFCurrentDirectory(tiffPtr) != directory)
  {
    if(!TIFFSetDirectory(tiffPtr, directory))
    {
      return;
    }
  }
  ossimString dirPrefix = "dir"+ossimString::toString(directory)+".";

  if(TIFFGetField(tiffPtr, TIFFTAG_COMPRESSION, &compressionType))
  {
    addValue(dirPrefix+"tifftag.compression", ossimString::toString(compressionType));
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
  
  if (TIFFGetField(tiffPtr,
                   TIFFTAG_SUBFILETYPE ,
                   &subFileType ) )
  {
    addValue(dirPrefix+"tifftag.sub_file_type", ossimString::toString(subFileType));
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
    addValue(dirPrefix+"is_tiled", "true");
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

  loadGeotiffTags(tiffPtr, dirPrefix);    
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
  char citationStrPtr[CITATION_STRING_SIZE];
  char* buf = 0;
  ossim_uint16 doubleArraySize = 0;
  double tempDouble=0.0;
  double* doubleArray=0;
  ossimString doubleArrayStr;
  bool loadedGeotiff = false;
  if(!gtif)
  {
    addValue(dirPrefix+"is_geotiff", "false");
    return;
  } 

  // Note:  For some reason I am having troubles using
  // GTIFGetDefn so I have to query them directly.  I was getting
  // Core dumps in the JNI calls to the OSSIM core library.
  // Until this is resolved I will use the direct method of getting
  // the values I need.
  //
  ossim_uint16 tempUint16 = 0; 
  if(GTIFKeyGet(gtif, GTModelTypeGeoKey, &tempUint16, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.model_type", 
           ossimString::toString(tempUint16));
  }
  if(GTIFKeyGet(gtif, GeographicTypeGeoKey, &tempUint16, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.gcs_code", 
           ossimString::toString(tempUint16));
  }
  if(GTIFKeyGet(gtif, GeogGeodeticDatumGeoKey, &tempUint16, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.datum_code", 
           ossimString::toString(tempUint16));
  }
  if(GTIFKeyGet(gtif, GeogAngularUnitsGeoKey, &tempUint16, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.angular_units", 
           ossimString::toString(tempUint16));
  }
  if(GTIFKeyGet(gtif, GeogLinearUnitsGeoKey, &tempUint16, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.linear_units", 
           ossimString::toString(tempUint16));
  }

  if(GTIFKeyGet(gtif, ProjStdParallel1GeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.std_parallel_1", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjStdParallel2GeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.std_parallel_2", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjNatOriginLongGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.origin_lon", 
            ossimString::toString(tempDouble));
  }
  else if(GTIFKeyGet(gtif, ProjOriginLongGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.origin_lon", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjNatOriginLatGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.origin_lat", 
            ossimString::toString(tempDouble));
  }
  else if(GTIFKeyGet(gtif, ProjOriginLatGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.origin_lat", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjFalseEastingGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.false_easting", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjFalseNorthingGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.false_northing", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjCenterLongGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.center_lon", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjCenterLatGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.center_lat", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, ProjScaleAtNatOriginGeoKey, &tempDouble, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.scale_factor", 
            ossimString::toString(tempDouble));
  }
  if(GTIFKeyGet(gtif, GTRasterTypeGeoKey, &rasterType, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.raster_type", 
             ossimString::toString(rasterType));

  }
  if (GTIFKeyGet(gtif, ProjectedCSTypeGeoKey, &pcsCode, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.pcs_code", 
             ossimString::toString(pcsCode));
  }

  if ( GTIFKeyGet(gtif, GTCitationGeoKey, &citationStrPtr ,
                  0, CITATION_STRING_SIZE))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.citation", 
             ossimString(citationStrPtr));
  }
  if(GTIFKeyGet(gtif, PCSCitationGeoKey , &buf, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.pcs_citation", 
             buf);
  }
  if(GTIFKeyGet(gtif, ProjCoordTransGeoKey , &coordTransGeoCode, 0, 1))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.coord_trans_code", 
             ossimString::toString(coordTransGeoCode));
  }

  if(TIFFGetField(tiffPtr, TIFFTAG_GEOPIXELSCALE, &doubleArraySize, &doubleArray))
  {
    loadedGeotiff = true;
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_pixel_scale", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOTIEPOINTS,  &doubleArraySize, &doubleArray))
  {
    loadedGeotiff = true;
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_tie_points", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEODOUBLEPARAMS, &doubleArraySize, &doubleArray))
  {
    loadedGeotiff = true;
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_double_params", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOTRANSMATRIX, &doubleArraySize, &doubleArray))
  {
    loadedGeotiff = true;
    convertArrayToStringList(doubleArrayStr, doubleArray, doubleArraySize);
    if(!doubleArrayStr.empty())
    {
      addValue(dirPrefix+"tifftag.geo_trans_matrix", doubleArrayStr);
    }
  }
  if(TIFFGetField(tiffPtr, TIFFTAG_GEOASCIIPARAMS, &buf))
  {
    loadedGeotiff = true;
    addValue(dirPrefix+"tifftag.geo_ascii_params", buf);
  }

#if 1
  if(gtif)
  {
    GTIFDefn *defs = (GTIFDefn *)CPLCalloc(sizeof(GTIFDefn), 1); //GTIFAllocDefn();
    GTIFGetDefn(gtif, defs);
    if (!exists(dirPrefix + "tifftag.angular_units"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.angular_units",
               ossimString::toString(defs->UOMAngle));
    }
    if (!exists(dirPrefix + "tifftag.linear_units"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.linear_units",
               ossimString::toString(defs->UOMLength));
    }
    if (!exists(dirPrefix + "tifftag.datum_code"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.datum_code",
               ossimString::toString(defs->Datum));
    }
    if (!exists(dirPrefix + "tifftag.pcs_code"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.pcs_code",
               ossimString::toString(defs->PCS));
    }
    if (!exists(dirPrefix + "tifftag.gcs_code"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.gcs_code",
               ossimString::toString(defs->GCS));
    }
    if (!exists(dirPrefix + "tifftag.model_type"))
    {
      loadedGeotiff = true;
      addValue(dirPrefix + "tifftag.model_type",
               ossimString::toString(defs->Model));
    }
    VSIFree(defs);
    // GTIFFreeDefn(defs);
  }
#endif

  addValue(dirPrefix+"is_geotiff", ossimString::toString(loadedGeotiff));
  GTIFFree(gtif);
}

bool ossim::TiffHandlerState::isReduced(ossim_uint32 directory)const
{
  return getInt32Value("tifftag.sub_file_type", directory)&FILETYPE_REDUCEDIMAGE;
}

bool ossim::TiffHandlerState::isMask(ossim_uint32 directory)const
{
  return getInt32Value("tifftag.sub_file_type", directory)&FILETYPE_MASK;
}

bool ossim::TiffHandlerState::isPage(ossim_uint32 directory)const
{
  return getInt32Value("tifftag.sub_file_type", directory)&FILETYPE_PAGE;
}

bool ossim::TiffHandlerState::isTiled(ossim_uint32 directory)const
{
  return checkBool(directory, "is_tiled");
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

ossim_int32 ossim::TiffHandlerState::getModelType(ossim_int32 directory)const
{
  return getInt32Value("tifftag.model_type", directory);
}

ossim_int32 ossim::TiffHandlerState::getAngularUnits(ossim_int32 directory)const
{
  return getInt32Value("tifftag.angular_units", directory);
}

ossim_int32 ossim::TiffHandlerState::getLinearUnits(ossim_int32 directory)const
{
  return getInt32Value("tifftag.linear_units", directory);
}

ossim_int32 ossim::TiffHandlerState::getRasterType(ossim_int32 directory)const
{
  return getInt32Value("tifftag.raster_type", directory);
}

ossim_int32 ossim::TiffHandlerState::getDatumCode(ossim_int32 directory)const
{
  return getInt32Value("tifftag.datum_code", directory);
}

ossim_int32 ossim::TiffHandlerState::getPcsCode(ossim_int32 directory)const
{
  return getInt32Value("tifftag.pcs_code", directory);
}

ossim_int32 ossim::TiffHandlerState::getGcsCode(ossim_int32 directory)const
{

  return getInt32Value("tifftag.gcs_code", directory);
}

ossim_int32 ossim::TiffHandlerState::getInt32Value(const ossimString& key, 
                                                    ossim_int32 directory)const
{
  ossimString tempStr;
  
  if(getValue(tempStr, directory, key))
  {
    return tempStr.toDouble();
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

bool ossim::TiffHandlerState::load(const ossimKeywordlist& kwl,
                                   const ossimString& prefix)
{
  bool result = ossim::ImageHandlerState::load(kwl, prefix);
  m_tags.clear();

  kwl.extractKeysThatMatch(m_tags, "^("+prefix+"dir[0-9]+)");
  if(!prefix.empty())
  {
    m_tags.stripPrefixFromAll("^("+prefix+")");
  }
  ossimString numberOfDirectories = kwl.find(prefix, "number_of_directories");

  if(!numberOfDirectories.empty()) m_tags.add("number_of_directories", numberOfDirectories, true);

  return result;
}

bool ossim::TiffHandlerState::save(ossimKeywordlist& kwl,
                                   const ossimString& prefix)const
{
   bool result = ossim::ImageHandlerState::save(kwl, prefix);
   
   kwl.add(prefix.c_str(), m_tags);

   return result;
}
