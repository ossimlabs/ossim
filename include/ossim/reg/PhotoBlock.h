//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef PhotoBlock_HEADER
#define PhotoBlock_HEADER 1

#include <string>
#include <vector>
#include <memory>
#include <ossim/base/ossimConstants.h>
#include <ossim/reg/GroundControlPoint.h>
#include <ossim/reg/Image.h>
#include <ossim/reg/TiePoint.h>
#include <ossim/base/JsonInterface.h>

namespace ossim
{

/**
 * Class for representing MSP PhotoBlock.
 */
class PhotoBlock : public JsonInterface,
                   public std::enable_shared_from_this<PhotoBlock>
{
public:
   /**
    * Initialize the photoblock from a prior saved session. If none found, assumes a new, blank
    * session is being started.
    */
   PhotoBlock();
   PhotoBlock(const Json::Value& pb_json_node);
   PhotoBlock(const PhotoBlock& copyThis);

   ~PhotoBlock();

   PhotoBlock& operator=(const PhotoBlock& copythis);

   std::shared_ptr<Image> getImage(const std::string& imageId);
   std::shared_ptr<TiePoint> getTiePoint(unsigned int tpId);
   std::shared_ptr<GroundControlPoint> getGroundPoint(const std::string& gpId);

   ImageList&              getImageList()       { return m_imageList; }
   TiePointList&           getTiePointList()    { return m_tiePointList; }
   std::vector<std::shared_ptr<GroundControlPoint> >& getGroundPointList() { return m_gcpList; }

   // TODO: Add of individual components not valid until proper management of the JCM can be
   //       provided

   /**
    * Adds the image to the photoblock at last position. Sets the image's pb index member.
    * @param images To be added to PB
    * @return The PB index of the image in the photoblock.
    */
   unsigned int addImage(std::shared_ptr<Image> image);

   /**
    * Adds the image to the photoblock at last position. Sets the image's pb index member.
    * @param images To be added to PB
    * @return The PB index of the image in the photoblock.
    */
   unsigned int addGroundPoint(std::shared_ptr<GroundControlPoint> groundPoint);

   /**
    * Adds the tiepoint to the PB
    * @param tiepoint To be added to PB
    * @return The index of the TP in the photoblock (should be same as TP ID)
    */
   unsigned int addTiePoint(std::shared_ptr<TiePoint> tiepoint);

   /**
    * Adds the list of tiepoints to the PB
    * @param tiepointList To be added to PB
    */
  void addTiePoints(TiePointList& tiepointList);

   /*
   * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
   * for JSON format used.
   */
   virtual void loadJSON(const Json::Value& json);

   /*
   * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
   * for JSON format used.
   */
   virtual void saveJSON(Json::Value& json) const;

protected:
   std::vector<std::shared_ptr<Image> > m_imageList;
   std::vector<std::shared_ptr<TiePoint> > m_tiePointList;
   std::vector<std::shared_ptr<GroundControlPoint> > m_gcpList;
};

} // End namespace ISA

#endif
