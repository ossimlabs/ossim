//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Piecewise remapper class declaration.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimPiecewiseRemapper_HEADER
#define ossimPiecewiseRemapper_HEADER 1

#include <ossim/imaging/ossimTableRemapper.h>
#include <ossim/base/ossimRtti.h>
#include <vector>

/**
 * @class ossimPiecewiseRemapper
 *
 * Given object2 is a ossimPiecewiseRemapper with a state of:
 * 
 * object2.band0.remap0:  ((0,127,0,127),(128,255,128,382))
 * object2.band0.remap1:  ((0,382,0,255))
 *
 * Quadruples map to:
 * (<min_in> <max_in> <min_out> <max_out>)
 *
 * There can be any number of quadrupals.
 *
 * From above example:
 * 
 * 0 <-> 127 is mapped to 0 <-> 127
 * 128 <->  255 is mapped to 128 <-> 382
 * 0 <-> 382 is mapped to 0 <-> 255
 *
 * Notes:
 * 
 * 1) Currently there is only one remap type so it's not really needed but
 *    left in the code for future type remaps, e.g. a remap where there is
 *    a linear and a logarithmic section.
 *
 * 2) Any number of "remaps" are allowed.
 *
 * 3) Last remap should get data between scalar types.
 * 
 */
class OSSIMDLLEXPORT ossimPiecewiseRemapper : public ossimTableRemapper
{
public:

   enum PiecewiseRemapType
   {
      UNKNOWN       = 0,
      LINEAR_NATIVE = 1
   };
   
   /** default constructor */
   ossimPiecewiseRemapper();

   /** @return "ossimPiecewiseRemapper" */
   virtual ossimString getClassName()const;

   /** @return "OSSIM Piecewise Remapper" */
   virtual ossimString getLongName()  const;

   /** @return "Piecewise Remapper" */
   virtual ossimString getShortName() const;

   /**
    * @brief Get tile method.
    * @param tileRect Region of interest.
    * @param resLevel Resolution level.
    * @param The requested region of interest for resolution level.
    */
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect,
                                               ossim_uint32 resLevel = 0);

   /** @brief Initialization method.  Called on state change of chain. */ 
   virtual void initialize();

   /**
    * @brief Sets remap type.
    *
    * Current types: "linear_native"
    *
    * @param type
    */
   void setRemapType( const std::string& type );
   
   /**
    * @brief Saves the state to a keyword list.
    * @return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * @brief Loads (recreates) the state of an object from a keyword
    * list.
    * @return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);
   

   /** @brief Print method.  Called by ossimObject::operator<<. */
   virtual ostream& print(ostream& os) const;

   /** @return The min pixel of the band. */
   virtual double getMinPixelValue(ossim_uint32 band=0)const;
   
   /** @return The max pixel of the band. */
   virtual double getMaxPixelValue(ossim_uint32 band=0)const;

protected:

   /**
    * @brief Protected virtual destructor.
    *
    * This class is derived from ossimReferenced; hence, will auto destruct
    * provided it is wrapped/stored in an ossimRefPtr. Example usage:
    *
    * ossimRefPtr<ossimPiecewiseRemapper> pwr = new ossimPiecewiseRemapper();
    */
   virtual ~ossimPiecewiseRemapper();

private:

   /**
    * @class ossimRemapSet
    *
    * Private container class. Currently hold an array of doubles.
    */
   class ossimRemapSet
   {
   public:
      ossimRemapSet();
      ossimRemapSet(const ossimRemapSet& obj);
      const ossimRemapSet& operator=(const ossimRemapSet& rhs);
      std::vector<ossim_float64> m_set;
   };
   
   /**
    * @class ossimBandRemap
    *
    * Private container class. Holds array of ossimRemapSets for a given band.
    */
   class ossimBandRemap
   {
   public:
      ossimBandRemap();
      ossimBandRemap(const ossimBandRemap& obj);
      const ossimBandRemap& operator=(const ossimBandRemap& rhs);

      void loadState( const ossimKeywordlist& kwl,
                      const std::string& prefix,
                      ossim_uint32 band );
      
      void saveState( ossimKeywordlist& kwl,
                      const std::string& prefix,
                      ossimPiecewiseRemapper::PiecewiseRemapType remapType,
                      ossim_uint32 band ) const;

      /**
       * @brief Initializes set from string.
       *
       * Example input: ((0, 127, 0, 127), (128, 255, 128, 382))
       * 
       * @param s String to initialize from.
       * @param set Initialized by this.
       */
      bool initRemapSetFromString( const std::string& s,
                                   ossimPiecewiseRemapper::ossimRemapSet& set ) const;

      /**
       * @brief Gets a string from remap set for type.
       * @param remapType
       * @param set
       * @param s Initialized by this.
       */
      void getRemapSetString( ossimPiecewiseRemapper::PiecewiseRemapType remapType,
                              const ossimPiecewiseRemapper::ossimRemapSet& set, 
                              std::string& s ) const;

      void getLinearRemapSetString( const ossimPiecewiseRemapper::ossimRemapSet& set, 
                                    std::string& s ) const;
         
      std::vector<ossimRemapSet> m_remap;
      
   }; // End: class ossimBandRemap 
   
   /**
    * @brief Gets a string from remap type.
    * @param remapType
    * @param s Initialized by this.
    */
   void getRemapTypeString( ossimPiecewiseRemapper::PiecewiseRemapType remapType,
                            std::string& s ) const;

   /**
    * @brief Gets a string from remap set.
    *
    * Example output: ((0, 127, 0, 127), (128, 255, 128, 382))
    * 
    * @param set
    * @param s Initialized by this.
    */   
   void getLinearRemapSetString(
      const ossimPiecewiseRemapper::ossimRemapSet& set, 
      std::string& s ) const;

   /** @brief Builds the table. */
   void buildTable();

   /** @brief Builds the linear native table. */
   void buildLinearNativeTable();

   /** @brief Builds the linear native table for scalar type T. */
   template <class T> void buildLinearNativeTable(T dummy);
   
   /**
    * @brief Initialized base class (ossimTableRemapper) values:
    * - theTableBinCount
    * - theTableBandCount
    * - theTableType
    * - theTable (resizes if not correct size.
    */
   void setupTable();

   /** @brief Computes min/max from remaps in m_bandRemap. */
   void initMinMax();

   /** @private(not allowed) copy constructor. */
   ossimPiecewiseRemapper(const ossimPiecewiseRemapper& obj);

   /** @private(not allowed) assignment(operator=) */
   ossimPiecewiseRemapper& operator=(const ossimPiecewiseRemapper& hr);

   /** @brief Dirty flag to indicate table needs to be rebuilt. */
   bool m_dirty;
   
   PiecewiseRemapType m_remapType;

   // One set per band.
   std::vector<ossimPiecewiseRemapper::ossimBandRemap> m_bandRemap;

#if 1 /* Not sure if we need min/max right now.(drb) */
   // Stores the min/max from the table for each band.
   vector<ossim_float64> m_min;
   vector<ossim_float64> m_max;
#endif

   TYPE_DATA
};

#endif  /* #ifndef ossimPiecewiseRemapper_HEADER */
