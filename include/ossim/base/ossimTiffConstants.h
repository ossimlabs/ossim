//---
//
// License: MIT
//
// Description: TIFF constants. Here for stand alone code; i.e.,
// code that does not use libtiff.
// 
//---
// $Id$

#ifndef ossimTiffConstants_HEADER
#define ossimTiffConstants_HEADER 1

namespace ossim
{
   /**
    * Anonymous enumerations.  Note prefixed with 'O' to avoid conflict with
    * libtiff.
    */
   enum
   {
      OPIXEL_IS_AREA = 1,
      OPIXEL_IS_POINT = 2,
      OTIFFTAG_SUBFILETYPE = 254,
      OTIFFTAG_IMAGEWIDTH = 256,
      OTIFFTAG_IMAGELENGTH = 257,
      OTIFFTAG_BITSPERSAMPLE = 258,
      OTIFFTAG_COMPRESSION = 259,
      OTIFFTAG_PHOTOMETRIC = 262,
      OTIFFTAG_FILLORDER = 266,
      OTIFFTAG_IMAGEDESCRIPTION = 270,
      OTIFFTAG_MAKE = 271,
      OTIFFTAG_MODEL = 272,
      OTIFFTAG_STRIPOFFSETS = 273,
      OTIFFTAG_ORIENTATION = 274,
      OTIFFTAG_SAMPLESPERPIXEL = 277,
      OTIFFTAG_ROWSPERSTRIP = 278,
      OTIFFTAG_STRIPBYTECOUNTS = 279,
      OTIFFTAG_MINSAMPLEVALUE = 280,
      OTIFFTAG_MAXSAMPLEVALUE = 281,
      OTIFFTAG_XRESOLUTION = 282,
      OTIFFTAG_YRESOLUTION = 283,

      OTIFFTAG_PLANARCONFIG = 284,
      OTIFFTAG_PLANARCONFIG_CONTIG = 1,
      OTIFFTAG_PLANARCONFIG_SEPARATE = 2,

      OTIFFTAG_RESOLUTIONUNIT = 296,
      OTIFFTAG_PAGENUMBER = 297,
      OTIFFTAG_SOFTWARE = 305,

      // FORMAT is YYYY:MM:DD HH:MM:SS
      OTIFFTAG_DATETIME = 306,
      OTIFFTAG_ARTIST = 315,
      OTIFFTAG_PREDICTOR = 317,
      OTIFFTAG_COLORMAP = 320,
      OTIFFTAG_SUBIFD = 330,
      OTIFFTAG_TILEWIDTH = 322,
      OTIFFTAG_TILELENGTH = 323,
      OTIFFTAG_TILEOFFSETS = 324,
      OTIFFTAG_TILEBYTECOUNTS = 325,
      OTIFFTAG_EXTRASAMPLES = 338,
      OTIFFTAG_SAMPLEFORMAT = 339,
      OTIFFTAG_SMINSAMPLEVALUE = 340,
      OTIFFTAG_SMAXSAMPLEVALUE = 341,
      OTIFFTAG_YCBCRSUBSAMPLING = 530,
      OTIFFTAG_YCBCRPOSITIONING = 531,
      OTIFFTAG_REFERENCEBLACKWHITE = 532,
      OTIFFTAG_XMLPACKET = 700,
      OSAMPLEFORMAT_UINT = 1,
      OSAMPLEFORMAT_INT = 2,
      OSAMPLEFORMAT_IEEEFP = 3,
      OSAMPLEFORMAT_VOID = 4,
      OSAMPLEFORMAT_COMPLEXINT = 5,
      OSAMPLEFORMAT_COMPLEXIEEEFP = 6,

      OGT_MODEL_TYPE_GEO_KEY = 1024,
      OGT_RASTER_TYPE_GEO_KEY = 1025,
      OGT_CITATION_GEO_KEY = 1026,
      OGEOGRAPHIC_TYPE_GEO_KEY = 2048,
      OGEOG_CITATION_GEO_KEY = 2049,
      OGEOG_GEODETIC_DATUM_GEO_KEY = 2050,
      OGEOG_PRIME_MERIDIAN_GEOKEY = 2051,
      OGEOG_LINEAR_UNITS_GEO_KEY = 2052,
      OGEOG_ANGULAR_UNITS_GEO_KEY = 2054,
      OGEOG_ANGULAR_UNIT_SIZE_GEO_KEY = 2055,
      OGEOG_ELLIPSOID_GEO_KEY = 2056,
      OGEOG_SEMI_MAJOR_AXIS = 2057,
      OGEOG_SEMI_MINOR_AXIS = 2058,
      OGEOG_INV_FLATTENING_GEO_KEY = 2059,
      OGEOG_PRIME_MERIDIAN_LONG_GEO_KEY = 2061,
      OPROJECTED_CS_TYPE_GEO_KEY = 3072,
      OPCS_CITATION_GEO_KEY = 3073,
      OPROJECTION_GEO_KEY = 3074,
      OPROJ_COORD_TRANS_GEO_KEY = 3075,
      OPROJ_LINEAR_UNITS_GEO_KEY = 3076,
      OPROJ_LINEAR_UNIT_SIZE_GEO_KEY = 3077,
      OPROJ_STD_PARALLEL1_GEO_KEY = 3078,
      OPROJ_STD_PARALLEL2_GEO_KEY = 3079,
      OPROJ_NAT_ORIGIN_LONG_GEO_KEY = 3080,
      OPROJ_NAT_ORIGIN_LAT_GEO_KEY = 3081,
      OPROJ_FALSE_EASTING_GEO_KEY = 3082,
      OPROJ_FALSE_NORTHING_GEO_KEY = 3083,
      OPROJ_FALSE_ORIGIN_LONG_GEO_KEY = 3084,
      OPROJ_FALSE_ORIGIN_LAT_GEO_KEY = 3085,
      OPROJ_FALSE_ORIGIN_EASTING_GEO_KEY = 3086,
      OPROJ_FALSE_ORIGIN_NORTHING_GEO_KEY = 3087,
      OPROJ_CENTER_LONG_GEO_KEY = 3088,
      OPROJ_CENTER_LAT_GEO_KEY = 3089,
      OPROJ_SCALE_AT_NAT_ORIGIN_GEO_KEY = 3092,
      OPROJ_SCALE_AT_CENTER_GEO_KEY = 3093,
      OVERTICAL_CS_TYPE_GEO_KEY = 4096,
      OVERTICAL_CITATION_GEO_KEY = 4097,
      OVERTICAL_DATUM_GEO_KEY = 4098,
      OVERTICAL_UNITS_GEO_KEY = 4099,
      OLINEAR_METER = 9001,
      OLINEAR_FOOT = 9002,
      OLINEAR_FOOT_US_SURVEY = 9003,
      OANGULAR_DEGREE = 9102,
      OANGULAR_ARC_MINUTE = 9103,
      OANGULAR_ARC_SECOND = 9104,
      OANGULAR_GRAD = 9105,
      OANGULAR_GON = 9106,
      OANGULAR_DMS = 9107,
      OANGULAR_DMS_HEMISPHERE = 9108,
      OPCS_BRITISH_NATIONAL_GRID = 27700,
      OUSER_DEFINED = 32767,
      OTIFFTAG_COPYRIGHT = 33432,
      OMODEL_PIXEL_SCALE_TAG = 33550,
      OMODEL_TIE_POINT_TAG = 33922,
      OMODEL_TRANSFORM_TAG = 34264,
      OTIFFTAG_PHOTOSHOP = 34377,
      OGEO_KEY_DIRECTORY_TAG = 34735,
      OGEO_DOUBLE_PARAMS_TAG = 34736,
      OGEO_ASCII_PARAMS_TAG = 34737,
      OGDAL_METADATA_TAG = 42112,
      OGDAL_NODATA = 42113,
      ORPC_COEFFICIENT_TAG = 50844
   };

   enum CompressType
   {
      COMPRESSION_NONE = 1
   };

   enum PhotoInterpretation
   {
      OPHOTO_MINISWHITE = 0, // min value is white
      OPHOTO_MINISBLACK = 1, // min value is black
      OPHOTO_RGB = 2,        // RGB color model
      OPHOTO_PALETTE = 3,    // color map indexed
      OPHOTO_MASK = 4,       // $holdout mask
      OPHOTO_SEPARATED = 5,  // !color separations
      OPHOTO_YCBCR = 6,      // !CCIR 601
      OPHOTO_CIELAB = 7,     // !1976 CIE L*a*b*
      OPHOTO_ICCLAB = 9,
      OPHOTO_ITULAB = 10,
      OPHOTO_LAST = 11, // this is here for range checking

      OPHOTO_LOGL = 32844,
      OPHOTO_LOGLUV = 32845
   };

   enum ModelType
   {
      UNKNOWN = 0,
      OMODEL_TYPE_PROJECTED = 1,  // Projection Coordinate System
      OMODEL_TYPE_GEOGRAPHIC = 2, // Geographic latitude-longitude System
      OMODEL_TYPE_GEOCENTRIC = 3
   };

   enum
   {
      OTIFF_NOTYPE = 0,     /* placeholder */
      OTIFF_BYTE = 1,       /* 8-bit unsigned integer */
      OTIFF_ASCII = 2,      /* 8-bit bytes w/ last byte null */
      OTIFF_SHORT = 3,      /* 16-bit unsigned integer */
      OTIFF_LONG = 4,       /* 32-bit unsigned integer */
      OTIFF_RATIONAL = 5,   /* 64-bit unsigned fraction */
      OTIFF_SBYTE = 6,      /* !8-bit signed integer */
      OTIFF_UNDEFINED = 7,  /* !8-bit untyped data */
      OTIFF_SSHORT = 8,     /* !16-bit signed integer */
      OTIFF_SLONG = 9,      /* !32-bit signed integer */
      OTIFF_SRATIONAL = 10, /* !64-bit signed fraction */
      OTIFF_FLOAT = 11,     /* !32-bit IEEE floating point */
      OTIFF_DOUBLE = 12,    /* !64-bit IEEE floating point */
      OTIFF_IFD = 13,       /* %32-bit unsigned integer (offset) */
      OTIFF_LONG8 = 16,     /* BigTIFF 64-bit unsigned integer */
      OTIFF_SLONG8 = 17,    /* BigTIFF 64-bit signed integer */
      OTIFF_IFD8 = 18       /* BigTIFF 64-bit unsigned integer (offset) */
   };
   
} // End: namespace ossim

#endif /* End of "#ifndef ossimTiffConstants_HEADER" */
