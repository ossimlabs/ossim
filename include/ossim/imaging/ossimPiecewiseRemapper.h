//---
// License: MIT
// 
// Author:  David Burken
//
// Description:
//
// Piecewise remapper class declaration.
// 
//---
// $Id$

#ifndef ossimPiecewiseRemapper_HEADER
#define ossimPiecewiseRemapper_HEADER 1

#include <ossim/imaging/ossimTableRemapper.h>
#include <ossim/base/ossimRtti.h>
#include <vector>

/**
 * @class ossimPiecewiseRemapper
 *
 * This class provides piecewise linear remapping of input pixels to output pixels. The
 * object's state is represented as:
 *
 *   band0.remap0:  ( <segment0>, <segment1>, ...)
 *   band0.remap1:  ( <segment0>, ... )
 *
 * The vertices of each linear segment are specified as four numbers in parentheses:
 *
 *   (<min_in>, <max_in>, <min_out>, <max_out>)
 *
 * A linear function is computed to map min_in to min_out and likewise max_in to max_out,
 * with all intermediate points distributed linearly between the two extremes. Multiple
 * remaps are cascaded: if an input pixel's value falls outside the ranges defined in the
 * first remapper, then the second remapper's table is referenced. The cascading continues
 * until a value is found. If no lookup is available, the output pixel is assigned to the
 * NULL value.
 *
 * Example: Given object2 with a state of:
 * 
 *   object2.type: ossimPiecewiseRemapper
 *   object2.remap_type: linear_native
 *   object2.band0.remap0:  ( (0, 127, 0, 127), (128, 255, 128, 382) )
 *   object2.band0.remap1:  ( (0, 382, 0, 255) )
 *
 * remap0 applied:   
 * The input range 0 <-> 127 passes through with a one-to-one mapping.
 * 128 <-> 255 is stretched out to 128 <-> 382.
 * 
 * remap0 output if fed to remap1:
 * 0 maps to 0 and 382 maps to 255 (compression).
 * 
 * Notes:
 * 
 * 1) Currently there is only one remap type (linear_native) is supported so it's not
 *    needed but left in the code for future type remaps, e.g. a remap where there is
 *    a linear and a logarithmic section.
 *
 * 2) Any number of "remaps" are allowed.
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
