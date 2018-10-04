//*******************************************************************
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Class declaration for ossimQuickbirdMetaData.
// 
// This class parses a Space Imaging Quickbird meta data file.
//
//********************************************************************
// $Id: ossimQuickbirdMetaData.h 14412 2009-04-27 16:58:46Z dburken $

#ifndef ossimQuickbirdMetaData_HEADER
#define ossimQuickbirdMetaData_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordlist.h>
#include <iosfwd>
#include <memory>

class ossimFilename;
class ossimKeywordlist;

class OSSIMDLLEXPORT ossimQuickbirdMetaData : public ossimObject
{
public:
  enum QbParseTypes
  {
     QB_PARSE_TYPE_NONE = 0,
     QB_PARSE_TYPE_IMD = 1,
     QB_PARSE_TYPE_GEO = 2,
     QB_PARSE_TYPE_RPB = 4,
     QB_PARSE_TYPE_ATT = 8,
     QB_PARSE_TYPE_STE = 16,
     QB_PARSE_TYPE_EPH = 32,
     QB_PARSE_TYPE_TIL = 64,
     QB_PARSE_TYPE_ALL = QB_PARSE_TYPE_IMD | QB_PARSE_TYPE_GEO |
                         QB_PARSE_TYPE_RPB | QB_PARSE_TYPE_ATT |
                         QB_PARSE_TYPE_STE | QB_PARSE_TYPE_EPH |
                         QB_PARSE_TYPE_TIL

  };
  /** @brief default constructor */
  ossimQuickbirdMetaData();

  /** virtual destructor */
  virtual ~ossimQuickbirdMetaData();

  /**
    * @brief Open method that takes the image file.  By default we just 
    *        parse the imd, for this the most common.  
    *
    * @param imageFile Usually in the form of "po_2619900_pan_0000000.tif".
    * @param qbParseTypes The types of data to parse.  These are bitwise or 
    *                     together.   By default we will only do QB_PARSE_TYPE_IMD.
    *                     if you want more just QB_PARSE_TYPE_IMD | QB_PARSE_TYPE_GEO.
    * 
    * @return true on success, false on error.
    */
  bool open(const ossimFilename &imageFile,
            ossim_int32 qbParseTypes = QB_PARSE_TYPE_IMD);

  void clearFields();

  //---
  // Convenient method to print important image info:
  //---
  virtual std::ostream &print(std::ostream &out) const;

  /**
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
  virtual bool saveState(ossimKeywordlist &kwl,
                         const char *prefix = 0) const;

  /**
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
  virtual bool loadState(const ossimKeywordlist &kwl,
                         const char *prefix = 0);

  /**
    * @brief Method to parse Quickbird metadata file.
    *
    * @param metadata File name usually in the form of
    * ".IMD."
    *
    * @return true on success, false on error.
    */
  bool parseMetaData(const ossimFilename &metadata);

  /**
    * @brief Method to parse Quickbird rpc file.
    *
    * @param metadata File name usually in the form of
    * ".GEO.txt"
    *
    * @return true on success, false on error.
    *
    * @note Currently NOT implemented.
    */
  bool parseGEOData(const ossimFilename &data_file);

  bool parseATTData(const ossimFilename &data_file);

  bool parseEPHData(const ossimFilename &data_file);
  bool parseRPBData(const ossimFilename &data_file);
  bool parseSTEData(const ossimFilename &data_file);

  /** @return theSatID */
  ossimString getSatID() const;

  bool getEndOfLine(char *fileBuf,
                    ossimString lineBeginning,
                    const char *format,
                    ossimString &name);

  const ossimIpt &getImageSize() const;

  bool getMapProjectionKwl(const ossimFilename &imd_file,
                           ossimKeywordlist &kwl);

  const std::shared_ptr<ossimKeywordlist> getImdKwl() const;
  const std::shared_ptr<ossimKeywordlist> getAttKwl() const;
  const std::shared_ptr<ossimKeywordlist> getGeoKwl() const;
  const std::shared_ptr<ossimKeywordlist> getEphKwl() const;
  const std::shared_ptr<ossimKeywordlist> getRpbKwl() const;
  const std::shared_ptr<ossimKeywordlist> getSteKwl() const;
  const std::shared_ptr<ossimKeywordlist> getTilKwl() const;
  /*****************************************
*parseATTData EPH GEO IMD RPB TIL
*
****************************************/

private:
  ossimString theGenerationDate;
  ossimString theBandId;
  int theBitsPerPixel;
  ossimString theSatID;
  ossimString theTLCDate;
  ossim_float64 theSunAzimuth;
  ossim_float64 theSunElevation;
  ossim_float64 theSatAzimuth;
  ossim_float64 theSatElevation;
  int theTDILevel;
  std::vector<double> theAbsCalFactors;
  ossimString theBandNameList;
  ossimIpt theImageSize;
  ossim_float64 thePNiirs;
  ossim_float64 theCloudCoverage;

  std::shared_ptr<ossimKeywordlist> m_imdKwl;
  std::shared_ptr<ossimKeywordlist> m_attKwl;
  std::shared_ptr<ossimKeywordlist> m_geoKwl;
  std::shared_ptr<ossimKeywordlist> m_ephKwl;
  std::shared_ptr<ossimKeywordlist> m_rpbKwl;
  std::shared_ptr<ossimKeywordlist> m_steKwl;
  std::shared_ptr<ossimKeywordlist> m_tilKwl;

  bool parse(std::shared_ptr<ossimKeywordlist> &kwl,
             const ossimFilename &file);

  TYPE_DATA
};

#endif /* #ifndef ossimQuickbirdMetaData_HEADER */
