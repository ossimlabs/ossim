//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*****************************************************************************
#ifndef ossimTiffHandlerState_HEADER
#define ossimTiffHandlerState_HEADER 1
#include <ossim/support_data/ImageHandlerState.h>
#include <ossim/projection/ossimProjection.h>

typedef struct tiff TIFF;  
class ossimTieGptSet;
namespace ossim
{
  class OSSIM_DLL TiffHandlerState : public ossim::ImageHandlerState
  {
  public:
    TiffHandlerState();
    virtual ~TiffHandlerState();
    virtual const ossimString& getTypeName()const override;
    static const ossimString& getStaticTypeName();

    ossimKeywordlist& getTags(){return m_tags;}
    const ossimKeywordlist& getTags()const{return m_tags;}
    void addValue(const ossimString& key, const ossimString& value);
    bool getValue(ossimString& value, const ossimString& key)const;
    bool getValue(ossimString& value,
                  const ossim_uint32 directory, 
                  const ossimString& key)const;
    bool exists(ossim_uint32 directory, const ossimString& key)const;
    bool exists(const ossimString& key)const;
    bool checkBool(const ossimString& key)const;
    bool checkBool(ossim_uint32 directory, const ossimString& key)const;
    
    /**
    * Will load everything
    */
    bool loadDefaults(const ossimFilename& file);

    /**
    * Will load everything
    */
    void loadDefaults(TIFF* tiffPtr);


    void loadCurrentDirectory(TIFF* tiffPtr);
    void loadDirectory(TIFF* tiffPtr, ossim_uint32 directory);

    bool isReduced(ossim_uint32 directory)const;
    bool isMask(ossim_uint32 directory)const;
    bool isPage(ossim_uint32 directory)const;
    bool isTiled(ossim_uint32 directory)const;

    ossim_int32 getInt32Value(const ossimString& key, ossim_int32 directory=0)const;
    ossim_float64 getDoubleValue(const ossimString& key, ossim_int32 directory=0)const;
    bool getDoubleArray(std::vector<ossim_float64>& result, 
                        ossim_int32 directory,
                        const ossimString& key)const;

    ossim_int32 getRasterType(ossim_int32 directory=0)const;
    ossim_int32 getPcsCode(ossim_int32 directory=0)const;
    ossim_int32 getDatumCode(ossim_int32 directory=0)const;
    ossim_int32 getGcsCode(ossim_int32 directory=0)const;
    ossim_int32 getModelType(ossim_int32 directory=0)const;
    ossim_int32 getAngularUnits(ossim_int32 directory=0)const;
    ossim_int32 getLinearUnits(ossim_int32 directory=0)const;
    ossim_int64 getImageLength(ossim_int32 directory=0)const;
    ossim_int64 getImageWidth(ossim_int32 directory=0)const;
    ossim_float64 getOriginLat(ossim_int32 directory=0)const;
    ossim_float64 getOriginLon(ossim_int32 directory=0)const;
    ossim_float64 getStandardParallel1(ossim_int32 directory=0)const;
    ossim_float64 getStandardParallel2(ossim_int32 directory=0)const;
    ossim_float64 getFalseEasting(ossim_int32 directory=0)const;
    ossim_float64 getFalseNorthing(ossim_int32 directory=0)const;
    ossim_float64 getScaleFactor(ossim_int32 directory=0)const;
    bool getGeoDoubleParams(std::vector<ossim_float64>& result,
                            ossim_int32 directory)const;

    bool getGeoPixelScale(std::vector<ossim_float64>& result,
                          ossim_int32 directory)const;

    bool getGeoTiePoints(std::vector<ossim_float64>& result,
                         ossim_int32 directory)const;

    bool getGeoTransMatrix(std::vector<ossim_float64>& result,
                           ossim_int32 directory)const;

    virtual void load(const ossimKeywordlist& kwl,
                           const ossimString& prefix="") override;
    virtual void save(ossimKeywordlist& kwl,
                           const ossimString& prefix="")const override;
  private:
    static const ossimString m_typeName;

    void saveColorMap(const ossimString& dirPrefix,
                      const ossim_uint16* red, 
                      const ossim_uint16* green,
                      const ossim_uint16* blue,
                      ossim_uint32 numberOfEntries);
    void loadGeotiffTags(TIFF* tiffPtr, const ossimString& dirPrefix);
    void convertArrayToStringList(ossimString& result, double* doubleArray, ossim_int32 doubleArraSize)const;
    void initializeWorkingVariables();
    // void getTieSet(ossimTieGptSet& tieSet,
    //                ossim_uint32 directory,
    //                const std::vector<ossim_float64>& tiePoints) const;
    // bool hasOneBasedTiePoints(ossim_uint32 directory,
    //                           const std::vector<ossim_float64>& tiePoints) const;
    // bool getOssimProjectionName(ossimString& projName, ossim_uint32 directory) const;
    // bool getOssimDatumName(ossimString& datumName, ossim_uint32 directory) const;
   /**
    *  Converts double passed in to meters if needed.  The conversion is
    *  base on "theLiniarUnitsCode".
    */
//   double convert2meters(ossim_int32 linearUnits, double d) const;

    ossimKeywordlist  m_tags; 
    // these will be working variables
  };

}
#endif
