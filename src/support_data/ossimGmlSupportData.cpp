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

#include <ossim/support_data/ossimGmlSupportData.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimGrect.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimXmlAttribute.h>
#include <ossim/base/ossimXmlDocument.h>
#include <ossim/base/ossimXmlNode.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimEpsgProjectionFactory.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/projection/ossimSensorModel.h>

#include <iomanip>
#include <sstream>

static ossimTrace traceDebug("ossimGmlSupportData:debug");

#define UNKNOWN_PCSCODE 32767

ossimGmlSupportData::ossimGmlSupportData()
   :
   m_xmlDocument(0),
   // m_use_gmljp2_version2(true), 
   m_pcsCodeMap(UNKNOWN_PCSCODE),
   m_pcsCodeGeo(UNKNOWN_PCSCODE),
   m_srsNameStringMap("http://www.opengis.net/def/crs/EPSG/0/32767"),
   m_srsNameStringGeo("http://www.opengis.net/def/crs/EPSG/0/32767"),
   m_srsDimensionString("2"),
   m_axisLabelsStringMap("X Y"),
   m_axisLabelsStringGeo("Lat Long"),
   m_uomLabelsStringMap("m m"),
   m_uomLabelsStringGeo("deg deg")
{
}

ossimGmlSupportData::~ossimGmlSupportData()
{
}

bool ossimGmlSupportData::initialize( std::istream& in )
{
   bool status = false;
   
   if ( in.good() )
   {
      m_xmlDocument = new ossimXmlDocument();
      m_xmlDocument->read( in );
      status = true;
   }
   else
   {
      m_xmlDocument = 0;
   }
   
   return status;
}

bool ossimGmlSupportData::initialize( const ossimImageGeometry* geom,
                                      const ossimIrect& rect )
{
   static const char MODULE[] = "ossimGmlSupportData::initialize(geom)";
   
   bool status = false;
   
   if ( geom )
   {
      // ossim_uint32 code = UNKNOWN_PCSCODE; // unknown code
       
      ossimRefPtr<const ossimMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() )
      {
         // Get the PCS code:
         m_pcsCodeMap = mapProj->getPcsCode();
         m_pcsCodeGeo = mapProj->getPcsCode();
         
         // Create an SRS Name for the map projection
         std::ostringstream os;
         os << "http://www.opengis.net/def/crs/EPSG/0/" << m_pcsCodeMap;
         m_srsNameStringMap = os.str();
         
         // Create an SRS Name for the projection datum
         std::ostringstream os2;
         os2 << "http://www.opengis.net/def/crs/EPSG/0/" << m_pcsCodeGeo;
         m_srsNameStringGeo = os2.str();
         
         m_xmlDocument = new ossimXmlDocument(ossimFilename::NIL);
         
         // if ( m_use_gmljp2_version2 == true )
         // {

         ossimRefPtr<ossimXmlNode> rootNode = getGmljp2V2RootNode();
         m_xmlDocument->initRoot( rootNode );
         status = configureGmljp2V2( rootNode, geom, rect );
         // }
         
#if 0
         else
         {
            ossimRefPtr<ossimXmlNode> rootNode = getGmljp2V1RootNode();
            m_xmlDocument->initRoot( rootNode );
            status = configureGmljp2V1( rootNode, geom, rect );
         }
#endif
         
         // cout << "gmljp2Node: " << *(gmljp2Node.get()) << endl;
         
         // cout << "xmlDoc: " << *(m_xmlDocument.get()) << endl;
      }
   }

   if ( status == false )
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " DEBUG Entered...\n";
      
   }

   return status;
   
} // End: ossimGmlSupportData::initialize( geom, mapProj )

#if 0 
bool ossimGmlSupportData::configureGmljp2V1( ossimRefPtr<ossimXmlNode> node0,
                                             const ossimImageGeometry* geom )
{
   bool success = true;
   const ossimString BLANK = "";

   ossimString gridHighString;
   ossimString gridLowString;
   getLimits( geom, gridHighString, gridLowString );

   configureBounds( node0, geom );

   ossimString path = "rectifiedGridDomain";
   ossimRefPtr<ossimXmlNode> node2 = node0->addChildNode( path, BLANK );

   path = "RectifiedGrid";
   ossimRefPtr<ossimXmlNode> node2a = node2->addChildNode( path, BLANK );

   ossimRefPtr<ossimXmlAttribute> attr1 = new ossimXmlAttribute();
   ossimString name = "dimension";
   ossimString value = "2";
   attr1->setNameValue( name, value );
   node2a->addAttribute( attr1 );

   path = "limit";
   ossimRefPtr<ossimXmlNode> node2a1 = node2a->addChildNode( path, BLANK );

   path = "GridEnvelope"; 
   ossimRefPtr<ossimXmlNode> node2a1a = node2a1->addChildNode( path, BLANK );

   path = "low";
   ossimRefPtr<ossimXmlNode> node2a1a1 = node2a1a->addChildNode( path, gridLowString );

   path = "high";
   ossimRefPtr<ossimXmlNode> node2a1a2 = node2a1a->addChildNode( path, gridHighString );

   return success;
}
#endif

bool ossimGmlSupportData::configureGmljp2V2( ossimRefPtr<ossimXmlNode> node0,
                                             const ossimImageGeometry* geom,
                                             const ossimIrect& rect )
{
   bool success = false;

   ossimRefPtr<const ossimMapProjection> mapProj = geom->getAsMapProjection();
   if ( mapProj.valid() )
   {
      bool isGeographic = mapProj->isGeographic();
      
      const ossimString BLANK = "";
      
      ossimRefPtr<ossimXmlAttribute> attr(0);
      ossimString name;
      ossimString value;
      
      ossimString gridHighString;
      ossimString gridLowString;
      getLimits( rect, gridHighString, gridLowString );
      
      configureBounds( node0, geom, rect );
      
      ossimString path = "domainSet";
      ossimRefPtr<ossimXmlNode> node2 =
         node0->addChildNode( path, BLANK );
      
      path = "rangeSet";
      ossimRefPtr<ossimXmlNode> node3 =
         node0->addChildNode( path, BLANK );
      
      path = "File";
      ossimRefPtr<ossimXmlNode> node3a =
         node3->addChildNode( path, BLANK );
      
      path = "rangeParameters";
      ossimRefPtr<ossimXmlNode> node3a1 =
         node3a->addChildNode( path, BLANK );
      
      path = "fileName";
      ossimRefPtr<ossimXmlNode> node3a2 =
         node3a->addChildNode( path, "gmljp2://codestream" );
      
      path = "fileStructure";
      ossimRefPtr<ossimXmlNode> node3a3 =
         node3a->addChildNode( path, "inapplicable" );
      
      path = "gmlcov:rangeType";
      ossimRefPtr<ossimXmlNode> node4 =
         node0->addChildNode( path, BLANK );
      
      path = "gmljp2:featureMember";
      ossimRefPtr<ossimXmlNode> node5 =
         node0->addChildNode( path, BLANK );
      
      path = "gmljp2:GMLJP2RectifiedGridCoverage";
      ossimRefPtr<ossimXmlNode> node5a =
         node5->addChildNode( path, BLANK );

      attr  = new ossimXmlAttribute();
      name  = "gml:id";
      value = "CodeStream_0";
      attr->setNameValue( name, value );
      node5a->addAttribute( attr );

      path = "domainSet";
      ossimRefPtr<ossimXmlNode> node5a1 =
         node5a->addChildNode( path, BLANK );

      path = "RectifiedGrid";
      ossimRefPtr<ossimXmlNode> node5a1a =
         node5a1->addChildNode( path, BLANK );

      attr  = new ossimXmlAttribute();
      name  = "gml:id";
      value = "RG0001";
      attr->setNameValue( name, value );
      node5a1a->addAttribute( attr );

      attr  = new ossimXmlAttribute();
      name  = "dimension";
      value = "2";
      attr->setNameValue( name, value );
      node5a1a->addAttribute( attr );

      attr  = new ossimXmlAttribute();
      name  = "srsName";
      attr->setNameValue( name, m_srsNameStringMap );
      node5a1a->addAttribute( attr );

      path = "limits";
      ossimRefPtr<ossimXmlNode> node5a1a1 =
         node5a1a->addChildNode( path, BLANK );

      path = "GridEnvelope";
      ossimRefPtr<ossimXmlNode> node5a1a1a =
         node5a1a1->addChildNode( path, BLANK );

      path = "low";
      ossimRefPtr<ossimXmlNode> node5a1a1a1 =
         node5a1a1a->addChildNode( path, gridLowString );

      path = "high";
      ossimRefPtr<ossimXmlNode> node5a1a1a2 =
         node5a1a1a->addChildNode( path, gridHighString );

      path = "axisLabels";
      ossimRefPtr<ossimXmlNode> node5a1a2 =
         node5a1a->addChildNode( path, 
                                 mapProj->isGeographic() ? 
                                 m_axisLabelsStringGeo : m_axisLabelsStringMap );

      path = "origin";
      ossimRefPtr<ossimXmlNode> node5a1a3 =
         node5a1a->addChildNode( path, BLANK );

      path = "Point";
      ossimRefPtr<ossimXmlNode> node5a1a3a =
         node5a1a3->addChildNode( path, BLANK );

      attr  = new ossimXmlAttribute();
      name  = "gml:id";
      value = "P0001";
      attr->setNameValue( name, value );
      node5a1a3a->addAttribute( attr );

      attr  = new ossimXmlAttribute();
      name  = "srsName";
      attr->setNameValue( name, m_srsNameStringMap );
      node5a1a3a->addAttribute( attr );

      ossimString originString;
      ossimString offsetVector1String;
      ossimString offsetVector2String;
      if ( isGeographic )
      {
         getGeoOrigin( geom, rect.ul(), originString,
                       offsetVector1String, offsetVector2String );
      }
      else
      {
         getMapOrigin( geom, rect.ul(), originString,
                       offsetVector1String, offsetVector2String );
      }

      path = "pos";
      ossimRefPtr<ossimXmlNode> node5a1a3a1 =
         node5a1a3a->addChildNode( path, originString );

      path = "offsetVector";
      ossimRefPtr<ossimXmlNode> node5a1a4 =
         node5a1a->addChildNode( path, offsetVector1String );

      attr  = new ossimXmlAttribute();
      name  = "srsName";
      attr->setNameValue( name, 
                          (isGeographic ? m_srsNameStringGeo : m_srsNameStringMap ) );
      node5a1a4->addAttribute( attr );

      path = "offsetVector";
      ossimRefPtr<ossimXmlNode> node5a1a5 =
         node5a1a->addChildNode( path, offsetVector2String );

      attr  = new ossimXmlAttribute();
      name  = "srsName";
      attr->setNameValue( name, (isGeographic ? m_srsNameStringGeo : m_srsNameStringMap) );
      node5a1a5->addAttribute( attr );

      path = "rangeSet";
      ossimRefPtr<ossimXmlNode> node5a2 =
         node5a->addChildNode( path, BLANK );

      path = "File";
      ossimRefPtr<ossimXmlNode> node5a2a =
         node5a2->addChildNode( path, BLANK );

      path = "rangeParameters";
      ossimRefPtr<ossimXmlNode> node5a2a1 =
         node5a2a->addChildNode( path, BLANK );

      path = "fileName";
      ossimRefPtr<ossimXmlNode> node5a2a2 =
         node5a2a->addChildNode( path, "gmljp2://codestream" );

      path = "fileStructure";
      ossimRefPtr<ossimXmlNode> node5a2a3 =
         node5a2a->addChildNode( path, "inapplicable" );

      success = true;
   }
   
   return success;
}

bool ossimGmlSupportData::configureBounds(
   ossimRefPtr<ossimXmlNode> node0,
   const ossimImageGeometry* geom,
   const ossimIrect& rect)
{
   bool success = true;
   const ossimString BLANK = "";

   ossimString upperCornerString;         
   ossimString lowerCornerString;
   getGeoBounds( geom, rect, upperCornerString, lowerCornerString );

   ossimString path = "boundedBy";
   ossimRefPtr<ossimXmlNode> node1 = node0->addChildNode( path, BLANK );

   path = "Envelope";
   ossimRefPtr<ossimXmlNode> node1a = node1->addChildNode( path, BLANK );

   ossimRefPtr<ossimXmlAttribute> attr( 0 );
   ossimString name;

   attr = new ossimXmlAttribute();
   name = "srsName";
   attr->setNameValue( name, m_srsNameStringGeo );
   node1a->addAttribute( attr );

   attr = new ossimXmlAttribute();
   name = "axisLabels";
   attr->setNameValue( name, m_axisLabelsStringGeo );
   node1a->addAttribute( attr );

   attr = new ossimXmlAttribute();
   name = "uomLabels";
   attr->setNameValue( name, m_uomLabelsStringGeo );
   node1a->addAttribute( attr );

   attr = new ossimXmlAttribute();
   name = "srsDimension";
   attr->setNameValue( name, m_srsDimensionString );
   node1a->addAttribute( attr );

   path = "lowerCorner";
   ossimRefPtr<ossimXmlNode> node1a1 =
      node1a->addChildNode( path, lowerCornerString );

   path = "upperCorner";
   ossimRefPtr<ossimXmlNode> node1a2 =
      node1a->addChildNode( path, upperCornerString );

   return success;
}

ossimRefPtr<ossimXmlNode> ossimGmlSupportData::getGmljp2V2RootNode() const
{
   ossimRefPtr<ossimXmlNode> node = new ossimXmlNode();

   ossimString os = "gmljp2:GMLJP2CoverageCollection";
   node->setTag( os );

   ossimRefPtr<ossimXmlAttribute> attr( 0 );
   ossimString name;
   ossimString value;

   attr = new ossimXmlAttribute();
   name = "gml:id";
   value = "JPEG2000_0";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();
   name = "xmlns";
   value = "http://www.opengis.net/gml/3.2";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();   
   name = "xmlns:gml";
   value = "http://www.opengis.net/gml/3.2";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();   
   name = "xmlns:gmlcov";
   value = "http://www.opengis.net/gmlcov/1.0";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();   
   name = "xmlns:gmljp2";
   value = "http://www.opengis.net/gmljp2/2.0";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();     
   name = "xmlns:xsi";
   value = "http://www.w3.org/2001/XMLSchema-instance";
   attr->setNameValue( name, value );
   node->addAttribute( attr );
      
   attr = new ossimXmlAttribute();   
   name = "xsi:schemaLocation";
   value = "http://www.opengis.net/gmljp2/2.0 http://schemas.opengis.net/gmljp2/2.0/gmljp2.xsd";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   return node;
   
} // ossimGmlSupportData::getGmljp2V2Node()

ossimRefPtr<ossimXmlNode> ossimGmlSupportData::getGmljp2V1RootNode() const
{
   ossimRefPtr<ossimXmlNode> node = new ossimXmlNode();

   ossimString os = "gml:FeatureCollection";
   node->setTag( os );

   ossimRefPtr<ossimXmlAttribute> attr(0);
   ossimString name;
   ossimString value;
   
   attr = new ossimXmlAttribute();
   name = "xmlns";
   value = "http://www.opengis.net/gml";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();   
   name = "xmlns:gml";
   value = "http://www.opengis.net/gml";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   attr = new ossimXmlAttribute();     
   name = "xmlns:xsi";
   value = "http://www.w3.org/2001/XMLSchema-instance";
   attr->setNameValue( name, value );
   node->addAttribute( attr );
      
   attr = new ossimXmlAttribute();   
   name = "xsi:schemaLocation";
   value = "http://www.opengis.net/gml gmlJP2Profile.xsd";
   attr->setNameValue( name, value );
   node->addAttribute( attr );

   return node;
   
} // ossimGmlSupportData::getGmljp2V1Node()

bool ossimGmlSupportData::write(std::ostream& os)
{
   bool status = false;
   
   if ( m_xmlDocument.valid() )
   {
      os << *(m_xmlDocument.get());
      
      status = true;
   }

   return status;
}

ossimRefPtr<ossimXmlDocument> ossimGmlSupportData::getXmlDoc() const
{
   return m_xmlDocument;
}

bool ossimGmlSupportData::getImageGeometry( ossimKeywordlist& geomKwl ) const
{
   // Try for map projected geometry first...
   bool success = getImageGeometryFromRectifiedGrid( geomKwl );
   if ( !success )
   {
      // Look for sensor model block.
      getImageGeometryFromSeonsorModel( geomKwl );
   }

   return success;
}

#if 0 /* Please leave.  (drb) */

bool ossimGmlSupportData::getImageGeometry( ossimKeywordlist& geomKwl ) const
{   
   if ( m_xmlDocument.valid() )
   {
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;
      bool gotSensorImage    = false;
      bool gotRectifiedImage = false;
      ossim_uint32 pcsCodeGrid = 32767; // only applies to rectified

      // Check the GMLJP2CoverageCollection attributes for the default namespace.
      ossimString defaultNamespaceStr( "" );
      ossimString xpath_root = "/gmljp2:GMLJP2CoverageCollection";
      xml_nodes.clear();
      m_xmlDocument->findNodes( xpath_root, xml_nodes );
      if ( xml_nodes.size() == 0 ) 
      {
         // check if the default namespace is gmljp2
         xpath_root = "/GMLJP2CoverageCollection";
         m_xmlDocument->findNodes( xpath_root, xml_nodes );
      }
      if ( xml_nodes.size() >= 1 )
      {
         const ossimString defaultNamespaceIdentifierStr( "xmlns" );
         ossimString defaultNamespacePrependStr = defaultNamespaceIdentifierStr + ":";

         const ossimRefPtr<ossimXmlAttribute> defaultNamespaceAttribute = xml_nodes[0]->findAttribute( defaultNamespaceIdentifierStr );
         ossimString defaultNamespaceSettingStr = defaultNamespaceAttribute->getValue();

         // search for the attribute value in the other attributes
         const ossimXmlNode::AttributeListType& attributeList = xml_nodes[0]->getAttributes();
         size_t nAttributes = attributeList.size();
         for ( size_t i=0; i<nAttributes; ++i )
         {
            const ossimRefPtr<ossimXmlAttribute> attribute = attributeList[i];
         
            const ossimString& attribute_name  = attribute->getName();
            const ossimString& attribute_value = attribute->getValue();

            if ( attribute_name  != defaultNamespaceIdentifierStr && 
                 attribute_value == defaultNamespaceSettingStr )
            {
               defaultNamespaceStr = attribute_name.after( defaultNamespacePrependStr );
               defaultNamespaceStr += ":";
            }
         }
      }
       
      // Check for a sensor image
      ossimString xpath0 = "/gmljp2:GMLJP2CoverageCollection/gmljp2:featureMember/gmljp2:GMLJP2ReferenceableGridCoverage/gml:domainSet/gmlcov:ReferenceableGridBySensorModel";
      xpath0 = xpath0.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
      xml_nodes.clear();
      m_xmlDocument->findNodes( xpath0, xml_nodes );
      if ( xml_nodes.size() >= 1 )
      {
         // we've got a sensor model image
         gotSensorImage = true;
      }
      else
      {
         const ossimString srsNameStr( "srsName" );
         ossimString pcsCodeDefinitionStr( "http://www.opengis.net/def/crs/EPSG/0/" );

         xpath0 = "/gmljp2:GMLJP2CoverageCollection/gmljp2:featureMember/gmljp2:GMLJP2RectifiedGridCoverage/gml:domainSet/gml:RectifiedGrid";
         xpath0 = xpath0.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath0, xml_nodes );
         if ( xml_nodes.size() >= 1 )
         {
            // we've got a rectified image
            gotRectifiedImage = true;

            const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[0]->findAttribute( srsNameStr );
            const ossimString& originSrsName = hrefAttribute->getValue();
            ossimString pcsCodeGridStr = originSrsName.after( pcsCodeDefinitionStr.string() );
            pcsCodeGrid = pcsCodeGridStr.toUInt32();
            if ( pcsCodeGrid != 32767 )
            {
               //---
               // The ossimEpsgProjectionFactory will not pick up the origin latitude if code is
               // 4326 (geographic) so we use the projection name; else, the origin_latitude will
               // always be 0.  This is so the gsd comes out correct for scale.
               //---
               if ( pcsCodeGrid != 4326 ) // map projection
               {
                  // Add the pcs code.
                  geomKwl.add( ossimKeywordNames::PCS_CODE_KW,
                               pcsCodeGridStr.c_str() );
               }
               else // geographic
               {
                  geomKwl.add( ossimKeywordNames::TYPE_KW, 
                               ossimString( "ossimEquDistCylProjection" ) );
               }
            }
         }
      }

      // Number of lines & samples, for either sensor or rectified imagery

      ossimString xpath_limits_low  = "/gml:limits/gml:GridEnvelope/gml:low";
      ossimString xpath_limits_high = "/gml:limits/gml:GridEnvelope/gml:high";

      xpath_limits_low  = xpath_limits_low.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
      xpath_limits_high = xpath_limits_high.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );

      bool gotLow = false;
      ossim_int32 lowX, lowY;
      ossimString xpath = xpath0 + xpath_limits_low;
      xml_nodes.clear();
      m_xmlDocument->findNodes( xpath, xml_nodes );
      if ( xml_nodes.size() == 1 )
      {
         const ossimString& lowerCorner = xml_nodes[0]->getText();
         size_t spacePos = lowerCorner.find( ' ' );
         ossimString lowerXString = lowerCorner.beforePos( spacePos );
         ossimString lowerYString = lowerCorner.afterPos ( spacePos );
         lowX = lowerXString.toInt32();
         lowY = lowerYString.toInt32();
         gotLow = true;
      }

      bool gotHigh = false;
      ossim_int32 highX = 0;
      ossim_int32 highY = 0;
      xpath = xpath0 + xpath_limits_high;
      xml_nodes.clear();
      m_xmlDocument->findNodes( xpath, xml_nodes );
      if ( xml_nodes.size() == 1 )
      {
         const ossimString& higherCorner = xml_nodes[0]->getText();
         size_t spacePos = higherCorner.find( ' ' );
         ossimString higherXString = higherCorner.beforePos( spacePos );
         ossimString higherYString = higherCorner.afterPos ( spacePos );
         highX = higherXString.toInt32();
         highY = higherYString.toInt32();
         gotHigh = true;
      }

      if ( gotHigh && gotLow )
      {
         geomKwl.add( ossimKeywordNames::NUMBER_LINES_KW,   highY - lowY + 1 );
         geomKwl.add( ossimKeywordNames::NUMBER_SAMPLES_KW, highX - lowX + 1 );
      }

      if ( gotSensorImage )
      {
         const ossimString hrefStr( "xlink:href" );
         const ossimString codeSpaceStr( "codeSpace" );

         ossimString sensorModelHref( "" );
         ossimString xpath_sensor_model = "/gmlcov:sensorModel";
         xpath_sensor_model = xpath_sensor_model.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xpath = xpath0 + xpath_sensor_model;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[0]->findAttribute( hrefStr );
            sensorModelHref = hrefAttribute->getValue();
         }

         ossimString sensorInstanceHref( "" );
         ossimString xpath_sensor_typeOf = "/gmlcov:sensorInstance/sml:SimpleProcess/sml:typeOf";
         xpath_sensor_typeOf = xpath_sensor_typeOf.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xpath = xpath0 + xpath_sensor_typeOf;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[0]->findAttribute( hrefStr );
            sensorInstanceHref = hrefAttribute->getValue();
         }

         ossimRefPtr<ossimSensorModel> sensor_model = 0;
         ossimString xpath_sensor_name = "/gmlcov:sensorInstance/sml:SimpleProcess/gml:name";
         xpath_sensor_name = xpath_sensor_name.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xpath = xpath0 + xpath_sensor_name;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         int nSensorNames = (int)xml_nodes.size();
         for ( int i=0; i<nSensorNames; ++i )
         {
            const ossimString& sensorName = xml_nodes[i]->getText();

            ossimProjectionFactoryRegistry* registry = ossimProjectionFactoryRegistry::instance();
            ossimProjection* proj = registry->createProjection( sensorName );

            // Is it a sensor model ?
            sensor_model = dynamic_cast<ossimSensorModel*>( proj );
            if ( sensor_model.valid() )
            {
               geomKwl.add( ossimKeywordNames::TYPE_KW, sensorName.c_str() );
               break;
            }
         }
         
         if ( !sensor_model.valid() )
         {
            // Add debug message
            return false;
         }

         // Check if the sensor instance is typeOf the sensor model
         if ( sensorModelHref == sensorInstanceHref )
         {
            const ossimString refStr( "ref" );

            // sml:setValue
            ossimString xpath_setValue = "/gmlcov:sensorInstance/sml:SimpleProcess/sml:configuration/sml:Settings/sml:setValue";
            xpath_setValue = xpath_setValue.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
            xpath = xpath0 + xpath_setValue;
            xml_nodes.clear();
            m_xmlDocument->findNodes( xpath, xml_nodes );
            size_t nXmlNodes = xml_nodes.size();
            for( size_t i=0; i<nXmlNodes; ++i )
            {
               const ossimString& elementValue = xml_nodes[i]->getText();

               const ossimRefPtr<ossimXmlAttribute> refAttribute = xml_nodes[i]->findAttribute( refStr );
               const ossimString& settingsRef = refAttribute->getValue();

               bool successSetValue = sensor_model->getImageGeometry( settingsRef, elementValue, geomKwl );
               success &= successSetValue;
               if ( !successSetValue )
               {
                  // Add debug message
               }
            }

            /* sml:setArrayValues */
            ossimString xpath_setArrayValues = "/gmlcov:sensorInstance/sml:SimpleProcess/sml:configuration/sml:Settings/sml:setArrayValues";
            xpath_setArrayValues = xpath_setArrayValues.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
            xpath = xpath0 + xpath_setArrayValues;
            xml_nodes.clear();
            m_xmlDocument->findNodes( xpath, xml_nodes );
            nXmlNodes = xml_nodes.size();
            for( size_t i=0; i<nXmlNodes; ++i )
            {
               ossimString elementValue( "" );

               const ossimRefPtr<ossimXmlAttribute> refAttribute = xml_nodes[i]->findAttribute( refStr );
               const ossimString& settingsRef = refAttribute->getValue();

               const ossimXmlNode::ChildListType& children = xml_nodes[i]->getChildNodes();
               if ( children.size() > 0 )
               {
                  const ossimXmlNode::ChildListType& grandchildren = children[0]->getChildNodes();
                   
                  if ( (grandchildren.size() > 1) && (grandchildren[1]->getTag() == ossimString( "sml:value")) )
                  {
                     elementValue = grandchildren[1]->getText();
                  }
               }

               bool successSetArrayValues = sensor_model->getImageGeometry( settingsRef, elementValue, geomKwl );
               success &= successSetArrayValues;
               if ( !successSetArrayValues )
               {
                  // Add debug message
               }
            }
         }
      }
      else if ( gotRectifiedImage )
      {
         const ossimString srsNameStr( "srsName" );
         ossimString pcsCodeDefinitionStr( "http://www.opengis.net/def/crs/EPSG/0/" );

         /* axis labels for rectified imagery */

         ossimString xpath_axisLabels = "/gml:axisLabels";
         xpath_axisLabels = xpath_axisLabels.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         ossimString firstAxisLabelString( "" );
         ossimString secondAxisLabelString( "" );
         ossimString xpath = xpath0 + xpath_axisLabels;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            ossimString axisLabelsString = xml_nodes[0]->getText();
            size_t spacePos = axisLabelsString.find( ' ' );
            firstAxisLabelString  = axisLabelsString.beforePos( spacePos );
            secondAxisLabelString = axisLabelsString.afterPos ( spacePos );
         }

         success = addTieAndScale( geomKwl );
         
         //---
         // origin:
         // Note: In GML the origin is the tie point, NOT the projection origin.
         //---

         ossim_uint32 pcsCodeOrigin = 32767;
         ossimString xpath_originPoint = "/gml:origin/gml:Point";
         xpath_originPoint = xpath_originPoint.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xpath = xpath0 + xpath_originPoint;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            const ossimString& originString = xml_nodes[0]->getChildTextValue( ossimString( "pos" ) );
            size_t spacePos = originString.find( ' ' );
            ossimString firstOriginString  = originString.beforePos( spacePos );
            ossimString secondOriginString = originString.afterPos ( spacePos );
            const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[0]->findAttribute( srsNameStr );
            const ossimString& originSrsName = hrefAttribute->getValue();
            ossimString pcsCodeOriginStr = originSrsName.after( pcsCodeDefinitionStr.string() );
            pcsCodeOrigin = pcsCodeOriginStr.toUInt32();

            if ( pcsCodeOrigin != 32767 )
            {
               std::string tie_point_xy;
               std::string tie_point_units;

               if ( pcsCodeOrigin == 4326 ) // map projection
               {
                  // Longitude first, e.g. (lon,lat)
                  tie_point_units = "degrees";
               }
               else
               {
                  tie_point_units = "meters";
               }
               
               if ( ( tie_point_units == "degrees" ) &&
                    ( firstAxisLabelString == "Lat" ) )
               {
                  tie_point_xy = "(";
                  tie_point_xy += secondOriginString.string();
                  tie_point_xy += ",";
                  tie_point_xy += firstOriginString.string();
                  tie_point_xy += ")";
               }
               else
               {
                  tie_point_xy = "(";
                  tie_point_xy += firstOriginString.string();
                  tie_point_xy += ",";
                  tie_point_xy += secondOriginString.string();
                  tie_point_xy += ")";
               }
                  
               geomKwl.add( ossimKeywordNames::TIE_POINT_XY_KW, tie_point_xy.c_str() );
               geomKwl.add( ossimKeywordNames::TIE_POINT_UNITS_KW, tie_point_units.c_str() );
            }
         }

         //---
         // offset vector
         // Note this is the scale:
         
         ossimString xpath_offsetVector = "/gml:offsetVector";
         xpath_offsetVector = xpath_offsetVector.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
         xpath = xpath0 + xpath_offsetVector;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );

         if ( xml_nodes.size() )
         {
            const ossimString& offsetVectorString = xml_nodes[0]->getText();
            size_t spacePos = offsetVectorString.find( ' ' );
            ossimString firstOffsetVectorString  = offsetVectorString.beforePos( spacePos );
            ossimString secondOffsetVectorString = offsetVectorString.afterPos ( spacePos );
            

            const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[0]->findAttribute( srsNameStr );
            const ossimString& offsetVectorSrsName = hrefAttribute->getValue();
            ossimString pcsCodeOffsetVectorStr = offsetVectorSrsName.after( pcsCodeDefinitionStr.string() );
            ossim_uint32 pcsCodeOffsetVector = pcsCodeOffsetVectorStr.toUInt32();

            if ( pcsCodeOffsetVector != 32767 )
            {
               std::string scale_xy;
               std::string scale_units;
               if ( pcsCodeOffsetVector == 4326 )
               {
                  scale_units = "degrees";
               }
               else
               {
                  scale_units = "meters";
               }
               
               if ( ( scale_units == "degrees" ) && ( firstAxisLabelString == "Lat" ) )
               {
                  scale_xy = "(";
                  scale_xy += secondOffsetVectorString.c_str();
                  scale_xy += ",";
                  scale_xy += firstOffsetVectorString.c_str();
                  scale_xy += ")";
               }
               else
               {
                  scale_xy = "(";
                  scale_xy += firstOffsetVectorString.c_str();
                  scale_xy += ",";
                  scale_xy += secondOffsetVectorString.c_str();
                  scale_xy += ")";
               }
               
               geomKwl.add( ossimKeywordNames::PIXEL_SCALE_XY_KW, scale_xy.c_str() );
               geomKwl.add( ossimKeywordNames::PIXEL_SCALE_UNITS_KW, scale_units.c_str() ); 
            }
         }
      }
   }

   return success;
   
} // End: ossimGmlSupportData::getImageGeometry( geomKwl )
#endif

void ossimGmlSupportData::getGeoOrigin( 
   const ossimImageGeometry* geom,
   const ossimIpt& ul,
   ossimString& originString,
   ossimString& offsetVector1String, 
   ossimString& offsetVector2String ) const
{
   if ( geom )
   {
      // Get the gsd:
      ossimDpt gsd;
      geom->getDegreesPerPixel( gsd );

      // Get tie point:
      ossimGpt tie;
      geom->localToWorld( ul, tie );

      std::ostringstream os;
      os.precision(15);
      os << tie.lat << " " << tie.lon;
      originString = os.str();

      std::ostringstream os2;
      os2.precision(15);
      os2 << -gsd.y << " " << "0.0";
      offsetVector1String = os2.str();

      std::ostringstream os3;
      os3.precision(15);
      os3 << "0.0" << " " << gsd.x;
      offsetVector2String = os3.str();
   }

} // End: ossimGmlSupportData::getGeoOrigin


void ossimGmlSupportData::getMapOrigin( 
   const ossimImageGeometry* geom,
   const ossimIpt& ul,
   ossimString& originString,
   ossimString& offsetVector1String, 
   ossimString& offsetVector2String ) const
{
   if ( geom )
   {
      ossimRefPtr<const ossimMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() != 0 )
      {
         // Get the tie point:
         ossimGpt gpt;
         geom->localToWorld( ul, gpt );

         ossimDpt tie = mapProj->forward( gpt );

         std::ostringstream os;
         os.precision(15);
         os << tie.x << " " << tie.y;
         originString = os.str();   
      }

      // Get the projected CS gsd:
      ossimDpt gsdMap;
      geom->getMetersPerPixel( gsdMap );

      std::ostringstream os2;
      os2.precision(15);
      os2 << gsdMap.x << " " << "0.0";
      offsetVector1String = os2.str();

      std::ostringstream os3;
      os3.precision(15);
      os3 << "0.0" << " " << -gsdMap.y;
      offsetVector2String = os3.str();
   }

} // End: ossimGmlSupportData::getMapBounds

void ossimGmlSupportData::getGeoBounds( const ossimImageGeometry* geom,
                                        const ossimIrect& rect,
                                        ossimString& upperCornerString,
                                        ossimString& lowerCornerString ) const
{
   if ( geom )
   {
      ossimRefPtr<const ossimMapProjection> mapProj = geom->getAsMapProjection();
      if ( mapProj.valid() )
      {
         // Get the bounding rect.  This assumes North up.
         ossimGpt ulGpt;
         ossimGpt lrGpt;
         geom->localToWorld( rect.ul(), ulGpt );
         geom->localToWorld( rect.lr(), lrGpt );
         
         std::ostringstream os;
         os.precision(15);
         os << ulGpt.lat << " " << ulGpt.lon;
         upperCornerString = os.str();
         
         std::ostringstream os2;
         os2.precision(15);
         os2 << lrGpt.lat << " " << lrGpt.lon;
         lowerCornerString = os2.str();
      }
   }
   
} // End: ossimGmlSupportData::getGeoBounds

void ossimGmlSupportData::getLimits( const ossimIrect& rect,
                                     ossimString& gridHighString,
                                     ossimString& gridLowString ) const
{
   if ( rect.hasNans() == false )
   {
      // Zero based image rect.
      gridLowString = "0 0";
         
      ossim_uint32 w = rect.width();
      ossim_uint32 h = rect.height();
      
      std::ostringstream os;
      os << (w-1) << " " << (h-1);
      gridHighString = os.str();
   }
}

bool ossimGmlSupportData::getImageGeometryFromSeonsorModel( ossimKeywordlist& /* geomKwl */) const
{
   bool status = false;

   if ( m_xmlDocument.valid() )
   {
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;
      ossimString xpath0 =  "/gmljp2:GMLJP2CoverageCollection/gmljp2:featureMember/gmljp2:GMLJP2ReferenceableGridCoverage/gml:domainSet/gmlcov:ReferenceableGridBySensorModel";

      m_xmlDocument->findNodes( xpath0, xml_nodes );
      if ( xml_nodes.size() >= 1 )
      {
         // Put sensor model code here...

         status = true;
      }
   }

   return status;
}

bool ossimGmlSupportData::getImageGeometryFromRectifiedGrid( ossimKeywordlist& geomKwl ) const
{
   bool status = false;

   if ( m_xmlDocument.valid() )
   {
      bool useGmlPrefix = false;
      
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;
      
      ossimString xpath0 = "/gmljp2:GMLJP2CoverageCollection/gmljp2:featureMember/gmljp2:GMLJP2RectifiedGridCoverage/domainSet/RectifiedGrid";
      m_xmlDocument->findNodes( xpath0, xml_nodes );
      if ( !xml_nodes.size() )
      {
         xpath0 = "/gmljp2:GMLJP2CoverageCollection/gmljp2:featureMember/gmljp2:GMLJP2RectifiedGridCoverage/gml:domainSet/gml:RectifiedGrid";
        m_xmlDocument->findNodes( xpath0, xml_nodes );
        if (  xml_nodes.size() )
        {
           useGmlPrefix = true;
        }
      }
         
      if ( xml_nodes.size() )
      {
         const ossimString SRS_NAME( "srsName" );
         const ossimString PCS_CODE_DEFINITION_STR( "http://www.opengis.net/def/crs/EPSG/0/" );
         
         const ossimRefPtr<ossimXmlAttribute> hrefAttribute =
            xml_nodes[0]->findAttribute( SRS_NAME );
         const ossimString& originSrsName = hrefAttribute->getValue();

         ossimString pcsCodeStr = originSrsName.after( PCS_CODE_DEFINITION_STR.string() );
         ossim_uint32 pcsCode = pcsCodeStr.toUInt32();

         if ( pcsCode != 32767 )
         {
            // Add the pcs code.
            geomKwl.add( ossimKeywordNames::PCS_CODE_KW, pcsCodeStr.c_str() );
            if ( pcsCode == 4326 )
            {
               ossimString srsName = "EPSG:";
               srsName += pcsCodeStr;
               geomKwl.add( ossimKeywordNames::SRS_NAME_KW, srsName.c_str() );
               geomKwl.add( ossimKeywordNames::TYPE_KW, "ossimEquDistCylProjection");
            }

            if ( addLineSamps( xpath0, useGmlPrefix, geomKwl ) )
            {
               if ( addTie( xpath0, useGmlPrefix, pcsCode, geomKwl ) )
               {
                  status = addScale( xpath0, useGmlPrefix, pcsCode, geomKwl );
               }
            }
         }
      }
   }

   return status;  
}

bool ossimGmlSupportData::addLineSamps( const ossimString& xpath0,
                                        bool useGmlPrefix,
                                        ossimKeywordlist& geomKwl ) const
{
   bool status = false;
   
   if ( m_xmlDocument.valid() )
   {
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;

      // Number of lines & samples, for either sensor or rectified imagery:
      ossimString xpath_limits_low;
      if (useGmlPrefix)
      {
         xpath_limits_low = "/gml:limits/gml:GridEnvelope/gml:low";
      }
      else
      {
         xpath_limits_low = "/limits/GridEnvelope/low";
      }
      ossimString xpath = xpath0 + xpath_limits_low;
       m_xmlDocument->findNodes( xpath, xml_nodes );
      if ( xml_nodes.size() == 1 )
      {
         const ossimString& lowerCorner = xml_nodes[0]->getText();
         size_t spacePos = lowerCorner.find( ' ' );
         ossimString lowerXString = lowerCorner.beforePos( spacePos );
         ossimString lowerYString = lowerCorner.afterPos ( spacePos );
         ossim_uint32 lowX = lowerXString.toInt32();
         ossim_uint32 lowY = lowerYString.toInt32();

         ossimString xpath_limits_high;
         if ( useGmlPrefix )
         {
            xpath_limits_high = "/gml:limits/gml:GridEnvelope/gml:high";
         }
         else
         {
            xpath_limits_high = "/limits/GridEnvelope/high";
         }

         xpath = xpath0 + xpath_limits_high;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            const ossimString& higherCorner = xml_nodes[0]->getText();
            size_t spacePos = higherCorner.find( ' ' );
            ossimString higherXString = higherCorner.beforePos( spacePos );
            ossimString higherYString = higherCorner.afterPos ( spacePos );
            ossim_uint32 highX = higherXString.toInt32();
            ossim_uint32 highY = higherYString.toInt32();
            
            geomKwl.add( ossimKeywordNames::NUMBER_LINES_KW,   highY - lowY + 1 );
            geomKwl.add( ossimKeywordNames::NUMBER_SAMPLES_KW, highX - lowX + 1 );
            
            status = true;
         }
      }
   }

   return status;
   
} // ossimGmlSupportData::addLineSamps

bool ossimGmlSupportData::addTie( const ossimString& xpath0,
                                  bool useGmlPrefix,
                                  ossim_uint32 pcsCode,
                                  ossimKeywordlist& geomKwl ) const
{
   bool status = false;

   if ( m_xmlDocument.valid() && (pcsCode != 32767 ) )
   {
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;

      //---
      // origin:
      // Note: In GML the origin is the tie point, NOT the projection origin.
      //---

      // axis labels for rectified imagery:
      ossimString xpath_axisLabels;
      if ( useGmlPrefix )
      {
          xpath_axisLabels = "/gml:axisLabels";
      }
      else
      {
         xpath_axisLabels = "/axisLabels";
      }

      ossimString xpath = xpath0 + xpath_axisLabels;
      m_xmlDocument->findNodes( xpath, xml_nodes );
      if ( xml_nodes.size() == 1 )
      {
         ossimString axisLabelsString = xml_nodes[0]->getText();

         ossimString xpath_originPoint;

         if ( useGmlPrefix )
         {
            xpath_originPoint = "/gml:origin/gml:Point";
         }
         else
         {
            xpath_originPoint = "/origin/Point";
         }

         xpath = xpath0 + xpath_originPoint;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 1 )
         {
            const ossimString& originString = xml_nodes[0]->getChildTextValue( ossimString( "pos" ) );
            size_t spacePos = originString.find( ' ' );
            ossimString firstOriginString  = originString.beforePos( spacePos );
            ossimString secondOriginString = originString.afterPos ( spacePos );
            
            std::string tie_point_xy;
            std::string tie_point_units;
            
            if ( pcsCode == 4326 ) // map projection
            {
               // Longitude first, e.g. (lon,lat)
               tie_point_units = "degrees";
            }
            else
            {
               tie_point_units = "meters";
            }
            
            if ( ( tie_point_units == "degrees" ) &&
                 ( axisLabelsString == "Lat Long" ) )
            {
               tie_point_xy = "(";
               tie_point_xy += secondOriginString.string();
               tie_point_xy += ",";
               tie_point_xy += firstOriginString.string();
               tie_point_xy += ")";
            }
            else
            {
               tie_point_xy = "(";
               tie_point_xy += firstOriginString.string();
               tie_point_xy += ",";
               tie_point_xy += secondOriginString.string();
               tie_point_xy += ")";
            }
            geomKwl.add( ossimKeywordNames::TIE_POINT_XY_KW, tie_point_xy.c_str() );
            geomKwl.add( ossimKeywordNames::TIE_POINT_UNITS_KW, tie_point_units.c_str() );
            status = true;
         }
      }
      
   } // Matches: if ( m_xmlDocument.valid() && (pscCode != 32767 ) )

   return status;
   
} // End: ossimGmlSupportData::addTie( ... )

bool ossimGmlSupportData::addScale( const ossimString& xpath0,
                                    bool useGmlPrefix,
                                    ossim_uint32 pcsCode,
                                    ossimKeywordlist& geomKwl ) const
{
   bool status = false;

   if ( m_xmlDocument.valid() && (pcsCode != 32767 ) )
   {
      vector< ossimRefPtr<ossimXmlNode> > xml_nodes;

      //---
      // origin:
      // Note: In GML the origin is the tie point, NOT the projection origin.
      //---

      // axis labels for rectified imagery:
      ossimString xpath_axisLabels;
      if ( useGmlPrefix )
      {
          xpath_axisLabels = "/gml:axisLabels";
      }
      else
      {
         xpath_axisLabels = "/axisLabels";
      }

      ossimString xpath = xpath0 + xpath_axisLabels;
      m_xmlDocument->findNodes( xpath, xml_nodes );
      if ( xml_nodes.size() == 1 )
      {
         ossimString axisLabelsString = xml_nodes[0]->getText();

         ossimString xpath_offsetVector;
         if ( useGmlPrefix )
         {
            xpath_offsetVector = "/gml:offsetVector";
         }
         else
         {
            xpath_offsetVector =  "/offsetVector";
         }
         
         xpath = xpath0 + xpath_offsetVector;
         xml_nodes.clear();
         m_xmlDocument->findNodes( xpath, xml_nodes );
         if ( xml_nodes.size() == 2 )
         {
            const ossimString& offsetVectorString0 = xml_nodes[0]->getText();
            const ossimString& offsetVectorString1 = xml_nodes[1]->getText();
            
            size_t spacePos0 = offsetVectorString0.find( ' ' );
            size_t spacePos1 = offsetVectorString1.find( ' ' );            
            
            ossimString firstOffsetVectorString0  = offsetVectorString0.beforePos( spacePos0 );
            ossimString secondOffsetVectorString0 = offsetVectorString0.afterPos ( spacePos0 );
            ossimString firstOffsetVectorString1  = offsetVectorString1.beforePos( spacePos1 );
            ossimString secondOffsetVectorString1 = offsetVectorString1.afterPos ( spacePos1 );
            
            // TODO: Add for rotational matrix:
            
            std::string scale_xy;
            std::string scale_units;
            if ( pcsCode == 4326 )
            {
               scale_units = "degrees";
            }
            else
            {
               scale_units = "meters";
            }

            ossimDpt scale;
            if ( ( scale_units == "degrees" ) &&
                 ( axisLabelsString == "Lat Long" ) )
            {
               scale.x = secondOffsetVectorString1.toFloat64();
               scale.y = firstOffsetVectorString0.toFloat64();
            }
            else
            {
               scale.x = firstOffsetVectorString0.toFloat64();
               scale.y = secondOffsetVectorString1.toFloat64();
            }
            if ( scale.y < 0.0 )
            {
               scale.y *= -1.0; // make positive
            }

            if ( ( scale_units == "degrees" ) && ( scale.x != scale.y ) )
            {
               // Compute/add the origin latitude of true scale.
               ossim_float64 origin_lat = ossim::acosd(scale.y/scale.x);

               geomKwl.add(  ossimKeywordNames::ORIGIN_LATITUDE_KW, origin_lat );
            }
            
            geomKwl.add( ossimKeywordNames::PIXEL_SCALE_XY_KW, scale.toString().c_str() );
            geomKwl.add( ossimKeywordNames::PIXEL_SCALE_UNITS_KW, scale_units.c_str() );                
            status = true;
         }
      }

   } // Matches: if ( m_xmlDocument.valid() && (pscCode != 32767 ) )

   return status;
   
} // End: ossimGmlSupportData::addScale( ... )
