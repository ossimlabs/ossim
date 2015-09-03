//---
// File: ossimNitfRsmModel.h
//---
#ifndef ossimNitfRsmModel_HEADER
#define ossimNitfRsmModel_HEADER 1

#include <ossim/projection/ossimRsmModel.h>

class ossimNitfImageHeader;
class ossimNitfRsmecaTag;
class ossimNitfRsmidaTag;
class ossimNitfRsmpcaTag;
class ossimNitfRsmpiaTag;

/**
 * @class ossimNitfRsmModel
 */
class OSSIM_DLL ossimNitfRsmModel : public ossimRsmModel
{
public:

   /** @brief default constructor */
   ossimNitfRsmModel();

   /** @brief copy constructor */
   ossimNitfRsmModel( const ossimNitfRsmModel& obj );

   /** @brief assignment operator */
   const ossimNitfRsmModel& operator=( const ossimNitfRsmModel& rhs );

   virtual bool parseFile(const ossimFilename& nitfFile,
                          ossim_uint32 entryIndex=0);

   virtual bool parseImageHeader(const ossimNitfImageHeader* ih);

   bool getRsmData(const ossimNitfImageHeader* ih);

   /**
    * @brief saveState
    * Fulfills ossimObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /**
    * @brief loadState
    * Fulfills ossimObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * @brief dup()
    * Returns pointer to a new instance, copy of this.
    */
   virtual ossimObject* dup() const;

   /**
    * @brief Initializes model from RSMECA tag.
    * @param rsmecaTag
    */
   bool initializeModel( const ossimNitfRsmecaTag* rsmecaTag );

   /**
    * @brief Initializes model from RSMECA tag.
    * @param rsmecaTag
    */
   bool initializeModel( const ossimNitfRsmidaTag* rsmidaTag );
   
   /**
    * @brief Initializes model from RSMECA tag.
    * @param rsmecaTag
    */ 
   bool initializeModel( const ossimNitfRsmpiaTag* rsmpiaTag );
   
   /**
    * @brief Initializes model from RSMECA tag.
    * @param rsmecaTag
    */   
   bool initializeModel( const ossimNitfRsmpcaTag* rsmpcaTag );

protected:
   /** @brief virtual destructor */
   virtual ~ossimNitfRsmModel();

   TYPE_DATA
      
}; // End: class ossimRsmModel

#endif /* #ifndef ossimNitfRsmModel_HEADER */
