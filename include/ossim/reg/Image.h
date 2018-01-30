//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef Image_HEADER
#define Image_HEADER 1

#include <map>
#include <memory>
#include <string>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/matrix/newmat.h>
#include <ossim/base/JsonInterface.h>
#include <ossim/projection/ossimSensorModel.h>
#include <string>

namespace ossim
{

/**
 * Class representing an Image as used by ossim-msp services.
 */
class OSSIM_DLL Image : public ossim::JsonInterface,
                        public std::enable_shared_from_this<Image>
{
public:
   static unsigned int UNASSIGNED_PHOTOBLOCK_INDEX;

   Image(const std::string& imageId,
         const std::string& filename,
         const std::string& modelName="",
         unsigned int entryIndex=0,
         unsigned int band=1);

   Image(const Json::Value& image_json_node);

   ~Image();

   std::string getImageId() const { return m_imageId; }
   std::string getFilename() const { return m_filename; }
   std::string getModelName() const { return m_modelName; }
   unsigned int getEntryIndex() const { return m_entryIndex; }
   unsigned int getActiveBand() const { return m_activeBand; }

   void setImageId(const std::string& id) { m_imageId = id; }
   void setFilename(const std::string& f) { m_filename = f; }
   void setEntryIndex(unsigned int i) { m_entryIndex = i; }

   /**
    * Returns all available sensor model plugins and model names for this image:
    * @param availableModels List of <plugin-name, model-name> pairs.
    */
    virtual void getAvailableModels(std::vector< pair<std::string, std::string> >& availableModels) const;

    /**
    * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
    * for JSON format used.
    */
    virtual void loadJSON(const Json::Value& json);

    /**
    * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
    * for JSON format used.
    */
    virtual void saveJSON(Json::Value& json) const;

protected:
   std::string m_imageId;
   ossimFilename m_filename;
   unsigned int m_entryIndex;
   unsigned int m_activeBand;
   std::string m_modelName;
   std::vector< pair<std::string, std::string> > m_availableModel;
   ossimRefPtr<ossimSensorModel> m_sensorModel;
};

typedef std::vector< std::shared_ptr<Image> > ImageList;

} // End namespace ATP

#endif
