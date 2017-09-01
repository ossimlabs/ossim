//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
//
// Description: STREOB tag class declaration.
//
// Dataset Indentification TRE.
//
// 
//----------------------------------------------------------------------------
// $Id

#ifndef ossimNitfStreobTag_HEADER
#define ossimNitfStreobTag_HEADER 1
#include <ossim/support_data/ossimNitfRegisteredTag.h>

class OSSIM_DLL ossimNitfStreobTag : public ossimNitfRegisteredTag
{
public:
   /** @brief default constructor */
   ossimNitfStreobTag();

   /** @brief destructor */
   virtual ~ossimNitfStreobTag();

   /** @brief Method to parse data from stream. */
   virtual void parseStream(std::istream& in);

   /** @brief Method to write data to stream. */
   virtual void writeStream(std::ostream& out);

   /** @brief Method to clear all fields including null terminating. */
   virtual void clearFields();

   /**
    * @brief Print method that outputs a key/value type format
    * adding prefix to keys.
    * @param out Stream to output to.
    * @param prefix Prefix added to key like "image0.";
    */
   virtual std::ostream& print(std::ostream& out,
                               const std::string& prefix) const;
   
protected:

   /**
    * FIELD: ST_ID
    * 
    * 60 byte field string
    *
    * alphanumeric
    *
    * The image ID of the first stereo mate. The fields ACQUISITION_DATE 
    * through END_ROW in the STDIDC tag constitute the image ID
    * 
    */
   char m_stdId[61];

   /**
    * FIELD: N_MATES
    *
    * 1 byte integer
    *
    * values: 1 to 3
    *
    * Number of Stereo Mates. If there are no stereo mates, 
    * there will not be any STREOB (TBR) extensions in the file. 
    * If there is a STREOB (TBR) extension, then there will be at 
    * least 1 stereo mate.
    */
   char n_nMates[2];


   /**
    * FIELD: MATE_INSTANCE
    *
    * 1 byte integer
    * 
    * values: 1 to 3
    *
    * Mate Instance identifies which stereo mate is described in 
    * that extension. For example, this field contains a 2 for 
    * the second stereo mate.
    */
    char m_mateInstance[2];

   /**
    * FIELD: B_CONV
    *
    * 5 byte field in degrees
    * 
    * Value: 00.00 to 90.00
    *
    * Beginning Convergence Angle defined at the first lines of 
    * the fore/left and aft/right images, unless those images are 
    * more than 90 degrees apart. If the images are more than 90 degrees 
    * apart, the first line of the fore and the last line of the 
    * aft shall be used.
    *
    */
   char m_bConv[6];

   /**
    * FIELD: E_CONV
    *
    * 5 byte field in degrees
    * 
    * Values: 00.00 to 90.00
    *
    * Ending Convergence Angle defined at the last lines of the fore/left 
    * and aft/right images, unless those images are more than 90 degrees apart. 
    * If the images are more than 90 degrees apart, the last line of the fore and 
    * the first line of the aft shall be used.
    */
   char m_eConv[6];

   /**
    * FIELD: B_ASYM
    *
    * 5 byte field
    * 
    * Values: 00.00 to 90.00
    * 
    * Beginning Asymmetry Angle defined at the first lines of the fore/left and 
    * aft/right images, unless those images are more than 90 degrees apart. 
    * If the images are more than 90 degrees apart, the first line of 
    * the fore and the last line of the aft shall be used.
    */
   char m_bAsym[6];

   /**
    * FIELD: E_ASYM
    *
    * 5 byte field 
    * 
    * Values: 00.00 to 90.00
    * 
    * Ending Asymmetry Angle defined at the last lines of the fore/left and aft/right 
    * images, unless those images are more than 90 degrees apart. If the 
    * images are more than 90 degrees apart, the last line of the fore and 
    * the first line of the aft shall be used.
    */
   char m_eAsym[6];

   /**
    * FIELD: B_BIE
    * 
    * 6 byte field 
    *
    * Values: +- 90.00
    *
    * Beginning BIE less Convergence Angle of Stereo Mate, defined at the first 
    * lines of the fore/left and aft/right images, unless those images are 
    * more than 90 degrees apart. If the images are more than 90 degrees apart,
    * the first line of the fore and the last line of the aft shall be used.
    */
   char m_bBie[7];

   /**
    * FIELD: E_BIE
    *
    * 6 byte field 
    *
    * Values: +- 90.00
    * 
    * Ending BIE less Convergence Angle of Stereo Mate, 
    * defined at the last lines of the fore/left and aft/right images, 
    * unless those images are more than 90 degrees apart. 
    * If the images are more than 90 degrees apart, the last 
    * line of the fore and the first line of the aft shall be used.
    */
   char m_eBie[7];

   
TYPE_DATA   
};

#endif /* matches #ifndef ossimNitfStreobTag_HEADER */
