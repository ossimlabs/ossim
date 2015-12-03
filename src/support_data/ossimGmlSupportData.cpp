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

#include <ossim/support_data/ossimGmlSupportData.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimGrect.h>
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
   m_xmlDocument(0), m_mapProj(0), m_use_gmljp2_version2(true), 
   m_pcsCodeMap(UNKNOWN_PCSCODE), m_pcsCodeGeo(UNKNOWN_PCSCODE),
   m_srsNameStringMap("http://www.opengis.net/def/crs/EPSG/0/32767"),
   m_srsNameStringGeo("http://www.opengis.net/def/crs/EPSG/0/32767"),
   m_srsDimensionString("2"),
   m_axisLabelsStringMap("X Y"), m_axisLabelsStringGeo("Lat Long"),
   m_uomLabelsStringMap("m m"), m_uomLabelsStringGeo("deg deg")
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
      
      // TODO: Check for correct gml block.
      
      status = true;
   }
   else
   {
      m_xmlDocument = 0;
   }
   
   return status;
}

bool ossimGmlSupportData::initialize( const ossimImageGeometry* geom )
{
   static const char MODULE[] = "ossimGmlSupportData::initialize(geom)";
   
   bool status = false;
   
   if ( geom )
   {
      // ossim_uint32 code = UNKNOWN_PCSCODE; // unknown code
       
      ossimRefPtr<const ossimProjection> proj = geom->getProjection();
      if ( proj.valid() )
      {
         ossimRefPtr<const ossimMapProjection> mapProj =
            dynamic_cast<const ossimMapProjection*>(proj.get());
         if ( mapProj.valid() )
         {
            // Get the PCS code:
            m_pcsCodeMap = mapProj->getPcsCode();
            m_pcsCodeGeo = mapProj->getPcsCode();
            m_mapProj    = mapProj;

            // Create an SRS Name for the map projection
            std::ostringstream os;
            os << "http://www.opengis.net/def/crs/EPSG/0/" << m_pcsCodeMap;
            m_srsNameStringMap = os.str();

            // Create an SRS Name for the projection datum
            std::ostringstream os2;
            os2 << "http://www.opengis.net/def/crs/EPSG/0/" << m_pcsCodeGeo;
            m_srsNameStringGeo = os2.str();

            m_xmlDocument = new ossimXmlDocument(ossimFilename::NIL);

            if ( m_use_gmljp2_version2 == true )
            {
               ossimRefPtr<ossimXmlNode> rootNode = getGmljp2V2RootNode();
               m_xmlDocument->initRoot( rootNode );
               status = configureGmljp2V2( rootNode, geom );
            }
            else
            {
               ossimRefPtr<ossimXmlNode> rootNode = getGmljp2V1RootNode();
               m_xmlDocument->initRoot( rootNode );
               status = configureGmljp2V1( rootNode, geom );
            }

            // cout << "gmljp2Node: " << *(gmljp2Node.get()) << endl;

            // cout << "xmlDoc: " << *(m_xmlDocument.get()) << endl;
         }
      }

      if ( status == false )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " DEBUG Entered...\n";
         
      }
   }

   return status;
   
} // End: ossimGmlSupportData::initialize( geom, mapProj )

bool ossimGmlSupportData::configureGmljp2V1( 
   ossimRefPtr<ossimXmlNode> node0, const ossimImageGeometry* geom )
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

bool ossimGmlSupportData::configureGmljp2V2( 
   ossimRefPtr<ossimXmlNode> node0, const ossimImageGeometry* geom )
{
   bool success = true;
   const ossimString BLANK = "";

   ossimRefPtr<ossimXmlAttribute> attr(0);
   ossimString name;
   ossimString value;

   ossimString gridHighString;
   ossimString gridLowString;
   getLimits( geom, gridHighString, gridLowString );

   configureBounds( node0, geom );

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
                              m_mapProj->isGeographic() ? 
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
   getOrigin( geom, originString, 
              offsetVector1String, 
              offsetVector2String );

   path = "pos";
   ossimRefPtr<ossimXmlNode> node5a1a3a1 =
      node5a1a3a->addChildNode( path, originString );

   path = "offsetVector";
   ossimRefPtr<ossimXmlNode> node5a1a4 =
      node5a1a->addChildNode( path, offsetVector1String );

   attr  = new ossimXmlAttribute();
   name  = "srsName";
   attr->setNameValue( name, 
                       m_mapProj->isGeographic() ? 
                       m_srsNameStringGeo : m_srsNameStringMap );
   node5a1a4->addAttribute( attr );

   path = "offsetVector";
   ossimRefPtr<ossimXmlNode> node5a1a5 =
      node5a1a->addChildNode( path, offsetVector2String );

   attr  = new ossimXmlAttribute();
   name  = "srsName";
   attr->setNameValue( name, 
                       m_mapProj->isGeographic() ? 
                       m_srsNameStringGeo : m_srsNameStringMap );
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

   return success;
}

bool ossimGmlSupportData::configureBounds(
   ossimRefPtr<ossimXmlNode> node0, const ossimImageGeometry* geom )
{
   bool success = true;
   const ossimString BLANK = "";

   ossimString upperCornerString;         
   ossimString lowerCornerString;
   getGeoBounds( geom, upperCornerString, lowerCornerString );

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
   bool success = true;
   
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

      /* Number of lines & samples, for either sensor or rectified imagery */

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

         ossimSensorModel* sensor_model = 0;
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
            if ( sensor_model != 0 )
            {
               geomKwl.add( ossimKeywordNames::TYPE_KW, sensorName.c_str() );
               break;
            }
         }
         if ( sensor_model == 0 )
         {
            // Add debug message
            return false;
         }

         // Check if the sensor instance is typeOf the sensor model
         if ( sensorModelHref == sensorInstanceHref )
         {
            const ossimString refStr( "ref" );

            /* sml:setValue */
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
      else
         if ( gotRectifiedImage )
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
               const ossimString& axisLabelsString = xml_nodes[0]->getText();
               size_t spacePos = axisLabelsString.find( ' ' );
               firstAxisLabelString  = axisLabelsString.beforePos( spacePos );
               secondAxisLabelString = axisLabelsString.afterPos ( spacePos );
            }

            /* origin */

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
                  if ( pcsCodeOrigin != 4326 ) // map projection
                  {
                     /* Convert to geographic */
                  }

                  if ( firstAxisLabelString == ossimString( "Lat" ) )
                  {
                     geomKwl.add( ossimKeywordNames::ORIGIN_LATITUDE_KW, 
                                  firstOriginString.c_str() );
                     geomKwl.add( ossimKeywordNames::CENTRAL_MERIDIAN_KW, 
                                  secondOriginString.c_str() );
                  }
                  else
                  {
                     geomKwl.add( ossimKeywordNames::ORIGIN_LATITUDE_KW, 
                                  secondOriginString.c_str() );
                     geomKwl.add( ossimKeywordNames::CENTRAL_MERIDIAN_KW, 
                                  firstOriginString.c_str() );
                  }
               }
            }

            /* offset vector */

            ossimString xpath_offsetVector = "/gml:offsetVector";
            xpath_offsetVector = xpath_offsetVector.replaceAllThatMatch( defaultNamespaceStr.c_str(), "" );
            xpath = xpath0 + xpath_offsetVector;
            xml_nodes.clear();
            m_xmlDocument->findNodes( xpath, xml_nodes );
            size_t nNodes = xml_nodes.size();
            for ( size_t i=0; i<nNodes; ++i )
            {
               const ossimString& offsetVectorString = xml_nodes[i]->getText();
               size_t spacePos = offsetVectorString.find( ' ' );
               ossimString firstOffsetVectorString  = offsetVectorString.beforePos( spacePos );
               ossimString secondOffsetVectorString = offsetVectorString.afterPos ( spacePos );

               const ossimRefPtr<ossimXmlAttribute> hrefAttribute = xml_nodes[i]->findAttribute( srsNameStr );
               const ossimString& offsetVectorSrsName = hrefAttribute->getValue();
               ossimString pcsCodeOffsetVectorStr = offsetVectorSrsName.after( pcsCodeDefinitionStr.string() );
               ossim_uint32 pcsCodeOffsetVector = pcsCodeOffsetVectorStr.toUInt32();
               if ( pcsCodeOffsetVector == 4326 )
               {
                  if ( firstAxisLabelString == ossimString( "Lat" ) )
                  {
                     if ( firstOffsetVectorString.toDouble() != 0.0 )
                     {    
                        geomKwl.add( ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT, 
                                     firstOffsetVectorString.c_str() );
                     }
                     else
                     {
                        geomKwl.add( ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON, 
                                     secondOffsetVectorString.c_str() );
                     }
                  }
                  else
                  {
                     if ( firstOffsetVectorString.toDouble() != 0.0 )
                     { 
                        geomKwl.add( ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON, 
                                     firstOffsetVectorString.c_str() );
                     }
                     else
                     {
                        geomKwl.add( ossimKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT, 
                                     secondOffsetVectorString.c_str() );
                     }
                  }
               }
               else // map projection
                  if ( pcsCodeOffsetVector == pcsCodeGrid )
                  {
                     if ( firstAxisLabelString == ossimString( "X" ) )
                     {
                        if ( firstOffsetVectorString.toDouble() != 0.0 )
                        {    
                           geomKwl.add( ossimKeywordNames::METERS_PER_PIXEL_X_KW, 
                                        firstOffsetVectorString.c_str() );
                        }
                        else
                        {
                           geomKwl.add( ossimKeywordNames::METERS_PER_PIXEL_Y_KW, 
                                        secondOffsetVectorString.c_str() );
                        }
                     }
                     else
                     {
                        if ( firstOffsetVectorString.toDouble() != 0.0 )
                        { 
                           geomKwl.add( ossimKeywordNames::METERS_PER_PIXEL_Y_KW, 
                                        firstOffsetVectorString.c_str() );
                        }
                        else
                        {
                           geomKwl.add( ossimKeywordNames::METERS_PER_PIXEL_X_KW, 
                                        secondOffsetVectorString.c_str() );
                        }
                     }
                  }
                  else
                  {
                     /*
                       Need to perform a coordinate conversion on the 
                       offset vector to the pcs code of the grid
                     */
                  }
            }
         }
   }

   return success;
   
} // End: ossimGmlSupportData::getImageGeometry( geoKwl )


void ossimGmlSupportData::getOrigin( 
   const ossimImageGeometry* geom,
   ossimString& originString,
   ossimString& offsetVector1String, 
   ossimString& offsetVector2String ) const
{
   if ( m_mapProj != 0 )
   {
      // In meters or decimal degrees.
      if ( m_mapProj->isGeographic() )
      {
         getGeoOrigin( geom, originString, 
                       offsetVector1String, offsetVector2String );
      }
      else
      {
         getMapOrigin( geom, originString, 
                       offsetVector1String, offsetVector2String );
      }
   }
} // End: ossimGmlSupportData::getOrigin


void ossimGmlSupportData::getGeoBounds( const ossimImageGeometry* geom,
                                        ossimString& upperCornerString,
                                        ossimString& lowerCornerString ) const
{
   if ( geom != 0 && m_mapProj != 0 )
   {
      // Get the bounding rect:
      ossimGrect rect;
      geom->getBoundingGroundRect( rect );
      
      ossimDpt upperEdge( rect.ul().lon, rect.ul().lat );
      ossimDpt lowerEdge( rect.lr().lon, rect.lr().lat );

      std::ostringstream os;
      os.precision(5);
      os << upperEdge.lat << " " << upperEdge.lon;
      upperCornerString = os.str();
      
      std::ostringstream os2;
      os2.precision(5);
      os2 << lowerEdge.lat << " " << lowerEdge.lon;
      lowerCornerString = os2.str();
   }
   
} // End: ossimGmlSupportData::getGeoBounds

void ossimGmlSupportData::getGeoOrigin( 
   const ossimImageGeometry* geom,
   ossimString& originString,
   ossimString& offsetVector1String, 
   ossimString& offsetVector2String ) const
{
   if ( geom )
   {
      // Get the gsd and bounding rect:
      ossimDpt gsd;
      ossimGrect rect;
      geom->getDegreesPerPixel( gsd );
      geom->getBoundingGroundRect( rect );

      ossimDpt halfGsd = gsd/2.0;
      ossimDpt upperEdge( rect.ul().lon + halfGsd.x, rect.ul().lat + halfGsd.y );

      std::ostringstream os;
      os.precision(15);
      os << upperEdge.lat << " " << upperEdge.lon;
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
   ossimString& originString,
   ossimString& offsetVector1String, 
   ossimString& offsetVector2String ) const
{
   if ( geom )
   {
      if ( m_mapProj != 0 )
      {
         ossimDpt originMap = m_mapProj->getUlEastingNorthing();

         std::ostringstream os;
         os.precision(15);
         os << originMap.x << " " << originMap.y;
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

void ossimGmlSupportData::getLimits( const ossimImageGeometry* geom,
                                     ossimString& gridHighString,
                                     ossimString& gridLowString ) const
{
   if ( geom )
   {
      // Get the image bounding rect:
      ossimIrect rect;
      geom->getBoundingRect( rect );
      
      if ( rect.hasNans() == false )
      {
         std::ostringstream os;
         os << rect.ul().x << " " << rect.ul().y;
         gridLowString = os.str();
         
         std::ostringstream os2;
         os2 << rect.lr().x << " " << rect.lr().y;
         gridHighString = os2.str();
      }
   }
   
}
