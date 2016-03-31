//---
//
// License:  MIT
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
class ossimIpt;
class ossimIrect;
class ossimKeywordlist;
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
    * @param rect Output rectangle (view rect).
    */
   bool initialize( const ossimImageGeometry* geom,
                    const ossimIrect& rect );

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

   void getGeoOrigin( const ossimImageGeometry* geom,
                      const ossimIpt& ul,
                      ossimString& originString,
                      ossimString& offsetVector1String, 
                      ossimString& offsetVector2String ) const;

   void getMapOrigin( const ossimImageGeometry* geom,
                      const ossimIpt& ul,
                      ossimString& originString,
                      ossimString& offsetVector1String, 
                      ossimString& offsetVector2String ) const;

   void getLimits( const ossimIrect& rect,
                   ossimString& gridHighString,
                   ossimString& gridLowString ) const;

   void getGeoBounds( const ossimImageGeometry* geom,
                      const ossimIrect& rect,
                      ossimString& upperCornerString,
                      ossimString& lowerCornerString ) const;
#if 0
   bool configureGmljp2V1( ossimRefPtr<ossimXmlNode> node,
                           const ossimImageGeometry* geom,
                           const ossimIrect& rect );
#endif
   
   bool configureGmljp2V2( ossimRefPtr<ossimXmlNode> node,
                           const ossimImageGeometry* geom,
                           const ossimIrect& rect );
   
   bool configureBounds  ( ossimRefPtr<ossimXmlNode> node,
                           const ossimImageGeometry* geom,
                           const ossimIrect& rect );

   /**
    * @brief For sensor model data:
    * @param geomKwl Initialized by this.
    */   
   bool getImageGeometryFromSeonsorModel( ossimKeywordlist& geomKwl ) const;

   /**
    * @brief For map projected data:
    * @param geomKwl Initialized by this.
    */   
   bool getImageGeometryFromRectifiedGrid( ossimKeywordlist& geomKwl ) const;
   
   /**
    * @brief Adds line and sample to keyword list.
    * @param xpath0
    * @param useGmlPrefix If true tack on "gml:" to paths.
    * @param geomKwl Initialized by this.
    */
   bool addLineSamps( const ossimString& xpath0,
                      bool useGmlPrefix,
                      ossimKeywordlist& geomKwl ) const;

   /**
    * @brief Adds tie point to keyword list.
    * @param xpath0
    * @param useGmlPrefix If true tack on "gml:" to paths.
    * @param geomKwl Initialized by this.
    */
   bool addTie( const ossimString& xpath0,
                bool useGmlPrefix,
                ossim_uint32 pcsCode,
                ossimKeywordlist& geomKwl ) const;

   /**
    * @brief Adds tie and scale to keyword list.
    * @param xpath0
    * @param useGmlPrefix If true tack on "gml:" to paths.
    * @param geomKwl Initialized by this.
    */
   bool addScale( const ossimString& xpath0,
                  bool useGmlPrefix,
                  ossim_uint32 pcsCode,
                  ossimKeywordlist& geomKwl ) const;
                  
   ossimRefPtr<ossimXmlDocument> m_xmlDocument;

   // bool         m_use_gmljp2_version2;
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
