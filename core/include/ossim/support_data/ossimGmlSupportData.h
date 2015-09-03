//---
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: GML support data object.
// 
//---
// $Id$
#ifndef ossimGmlSupportData_HEADER
#define ossimGmlSupportData_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <iosfwd>

class ossimImageGeometry;
class ossimKeywordlist;
class ossimMapProjection;
class ossimXmlDocument;
class ossimXmlNode;

/** @brief GML support data class. */
class OSSIM_DLL ossimGmlSupportData
{
public:
   
   /** default constructor */
   ossimGmlSupportData();

   /** destructor */
   ~ossimGmlSupportData();

   /**
    * @brief Initializes gml block from geometry file.
    * @param geom
    */
   bool initialize( const ossimImageGeometry* geom );

   /**
    * @brief Initializes from stream assumed to be a gml block.
    * @param in
    */   
   bool initialize( std::istream& in );

   /** @brief Write xml doc to stream. */
   bool write( std::ostream& os );

   /** @return Reference to xml doc. */
   ossimRefPtr<ossimXmlDocument> getXmlDoc() const;

   /**
    *  @brief Extracts geometry info to keyword list.
    *  
    *  Populates the keyword list with image geometry information.  This
    *  method is used to relay projection/model information to users.
    *
    *  @param geomKwl Keyword list that will be initialized with geometry info.
    *
    *  @return true if geometry info is present, false if not.
    */
   bool getImageGeometry( ossimKeywordlist& geomKwl ) const;
  

private:
   
   ossimRefPtr<ossimXmlNode> getGmljp2V1RootNode() const;
   ossimRefPtr<ossimXmlNode> getGmljp2V2RootNode() const;

   void getOrigin( const ossimImageGeometry* geom,
                   ossimString& originString,
                   ossimString& offsetVector1String, 
                   ossimString& offsetVector2String ) const;

   void getLimits( const ossimImageGeometry* geom,
                   ossimString& gridHighString,
                   ossimString& gridLowString ) const;

   void getGeoBounds( const ossimImageGeometry* geom,
                      ossimString& upperCornerString,
                      ossimString& lowerCornerString ) const;

   void getGeoOrigin( const ossimImageGeometry* geom,
                      ossimString& originString,
                      ossimString& offsetVector1String, 
                      ossimString& offsetVector2String ) const;

   void getMapOrigin( const ossimImageGeometry* geom,
                      ossimString& originString,
                      ossimString& offsetVector1String, 
                      ossimString& offsetVector2String ) const;

   bool configureGmljp2V1( ossimRefPtr<ossimXmlNode> node, const ossimImageGeometry* geom );
   bool configureGmljp2V2( ossimRefPtr<ossimXmlNode> node, const ossimImageGeometry* geom );
   bool configureBounds  ( ossimRefPtr<ossimXmlNode> node, const ossimImageGeometry* geom );
                  
   ossimRefPtr<ossimXmlDocument>         m_xmlDocument;
   ossimRefPtr<const ossimMapProjection> m_mapProj;

   bool         m_use_gmljp2_version2;
   ossim_uint32 m_pcsCodeMap;
   ossim_uint32 m_pcsCodeGeo;
   ossimString  m_srsNameStringMap;
   ossimString  m_srsNameStringGeo;
   ossimString  m_srsDimensionString;
   ossimString  m_axisLabelsStringMap;
   ossimString  m_axisLabelsStringGeo;
   ossimString  m_uomLabelsStringMap;
   ossimString  m_uomLabelsStringGeo;

};

#endif /* End of "#ifndef ossimGmlSupportData_HEADER" */
