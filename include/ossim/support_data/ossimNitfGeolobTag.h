//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description: GEOLOB tag class declaration.
//
// References:
//
// 1) DIGEST 2.1 Part 2 - Annex D
// Appendix 1 - NSIF Standard Geospatial Support Data Extension
//
// 2) STDI-0006
//---
// $Id$

#ifndef ossimNitfGeolobTag_HEADER
#define ossimNitfGeolobTag_HEADER 1

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <ossim/base/ossimConstants.h>
#include <string>

class OSSIM_DLL ossimNitfGeolobTag : public ossimNitfRegisteredTag
{
public:
   enum 
   {
      ARV_SIZE = 9,
      BRV_SIZE = 9,
      LSO_SIZE = 15,
      PSO_SIZE = 15,
      TAG_SIZE = 48
      //      -----
      //         48
   };
   
   /** default constructor */
   ossimNitfGeolobTag();
  
   /**
    * Parse method.
    *
    * @param in Stream to parse.
    */
   virtual void parseStream(ossim::istream& in);
   
   /**
    * Write method.
    *
    * @param out Stream to write to.
    */
   virtual void writeStream(ossim::ostream& out);

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;

   /**
    * @brief Gets the ARV field.
    * 
    * Longitude density:
    * This field shall contain the pixel ground spacing in E/W direction that
    * is the number of pixels or elements intervals in 360°
    *
    * Pixel size in decimal degree = 360.0 / AVR
    * 
    * @return ARV field as a string.
    */
   std::string getArvString() const;

   /**
    * @brief Gets the ARV field.
    * @return ARV field as a positive integer.
    **/
   ossim_uint32 getArv() const;

   /**
    * @brief Gets degrees per pixel in lonitude direction from BRV field.
    * 
    * Pixel size in decimal degree = 360.0 / AVR
    * 
    * @return Degrees per pixel in lonitude direction.
    */
   ossim_float64 getDegreesPerPixelLon() const;

   /**
    * @brief Sets the ARV field. Valid range: 000000002 to 999999999
    * @pararm arv
    */
   void setArv(ossim_uint32 arv);

   /**
    * @brief Sets the ARV field from decimal degrees per pixel longitude.
    * @pararm deltaLon
    */
   void setDegreesPerPixelLon(const ossim_float64& deltaLon);
   
   /**
    * @brief Gets the BRV field.
    * 
    * Latitude density:
    * This field shall contain the pixel ground spacing in N/S direction that
    * is the number of pixels or elements intervals in 360°.
    *
    * Pixel size in decimal degree = 360.0 / BVR
    * 
    * @return BRV field as a string.
    */
   std::string getBrvString() const;

   /**
    * @brief Gets the BRV field.
    * @return BRV field as a positive integer.
    **/
   ossim_uint32 getBrv() const;

   /**
    * @brief Gets degrees per pixel in latitude direction from BRV field.
    * 
    * Pixel size in decimal degree = 360.0 / BVR
    * 
    * @return Degrees per pixel in latitude direction.
    */
   ossim_float64 getDegreesPerPixelLat() const;

   /**
    * @brief Sets the BRV field. Valid range: 000000002 to 999999999
    * @pararm brv
    */
   void setBrv(ossim_uint32 brv);
   
   /**
    * @brief Sets the BRV field from decimal degrees per pixel latitude.
    * @pararm deltaLon
    */
   void setDegreesPerPixelLat(const ossim_float64& deltaLat);

   /**
    * @brief Gets the LSO field.
    * 
    * Longitude of Reference Origin:
    * This field shall contain the longitude of the origin pixel
    * (row number 0, column number 0) in the absolute coordinate system.
    * 
    * @return LSO field as a string.
    */
   std::string getLsoString() const;

   /**
    * @brief Gets the LSO field(Longitude Origin).
    * @return LSO field as a positive integer.
    **/
   ossim_float64 getLso() const;

   /**
    * @brief Sets the LSO field(Longitude Origin).
    * Valid range: -180.0 to +180.0
    * @pararm lso
    */
   void setLso(const ossim_float64& lso);

   /**
    * @brief Gets the PSO field.
    * 
    * Latitude of Reference Origin:
    * This field shall contain the latitude of the origin pixel
    * (row number 0, column number 0) in the absolute coordinate system.
    * 
    * @return PSO field as a string.
    */
   std::string getPsoString() const;

   /**
    * @brief Gets the PSO field(Latitude Origin).
    * @return PSO field as a positive integer.
    **/
   ossim_float64 getPso() const;

   /**
    * @brief Sets the PSO field(Latitude Origin).
    * Valid range: -90.0 to +90.0
    * @pararm pso
    */
   void setPso(const ossim_float64& pso);

protected:

   std::string m_tagData;
   
TYPE_DATA   
};

#endif /* End of "#ifndef ossimNitfGeolobTag_HEADER" */
