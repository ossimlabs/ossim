#include <ossim/imaging/ImageHandlerState.h>
#include <ossim/base/ossimKeywordNames.h>

const ossimString ossim::ImageHandlerState::m_typeName = "ossim::ImageHandlerState";

ossim::ImageHandlerState::ImageHandlerState()
: State(),
  m_pixelType(OSSIM_PIXEL_IS_POINT)
{

}

ossim::ImageHandlerState::ImageHandlerState::~ImageHandlerState()
{

}

const ossimString& ossim::ImageHandlerState::getTypeName()const
{
   return m_typeName;
}

const ossimString& ossim::ImageHandlerState::getStaticTypeName()
{
   return m_typeName;
}

void ossim::ImageHandlerState::load(const ossimKeywordlist& kwl,
                                    const ossimString& prefix)
{
   ossim::State::load(kwl, prefix);
}

void ossim::ImageHandlerState::save(ossimKeywordlist& kwl,
                                    const ossimString& prefix)const
{
   ossim::State::save(kwl, prefix);
   // kwl.add(prefix, ossimKeywordNames::FILENAME_KW, m_connectionString.c_str());
   // kwl.add(prefix, HAS_LUT_KW, (m_lut.valid()?"true":"false"));
   // kwl.add(prefix, ossimKeywordNames::IMAGE_ID_KW, m_imageID);
   // kwl.add(prefix, ossimKeywordNames::OVERVIEW_FILE_KW, m_overviewFile.c_str());
   // kwl.add(prefix, SUPPLEMENTARY_DIRECTORY_KW, m_upplementaryDirectory.c_str());
   // kwl.add(prefix, START_RES_LEVEL_KW, m_startingResLevel);
   // ossimString pixelType = "point";
   // switch(m_pixelType)
   // {
   //    case OSSIM_PIXEL_IS_AREA:
   //    {
   //       pixelType = "area";
   //       break;
   //    }
   //    default:
   //    {
   //       break;
   //    }
   // }
   // kwl.add(prefix, ossimKeywordNames::PIXEL_TYPE_KW, pixelType);
}
