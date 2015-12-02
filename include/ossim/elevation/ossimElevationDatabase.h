#ifndef ossimElevationDatabase_HEADER
#define ossimElevationDatabase_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimGeoid.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/elevation/ossimElevSource.h>
#include <ossim/elevation/ossimElevCellHandler.h>
#include <ossim/base/ossimTimer.h>
#include <vector>
#include <map>

class OSSIM_DLL ossimElevationDatabase : public ossimElevSource
{
public:
   
   ossimElevationDatabase()
   :  ossimElevSource(),
      m_meanSpacing(0.0)
   {
   }

   ossimElevationDatabase(const ossimElevationDatabase& src)
   :  ossimElevSource(src),
      m_connectionString(src.m_connectionString),
      m_geoid(src.m_geoid),
      m_meanSpacing(src.m_meanSpacing)
   {
   }

   virtual bool getAccuracyInfo(ossimElevationAccuracyInfo& /*info*/, const ossimGpt& /*gpt*/) const
   {
      return false;
   }
  virtual double getMeanSpacingMeters() const
   {
      return m_meanSpacing;
   }
   virtual void setGeoid(ossimGeoid* geoid)
   {
      m_geoid = geoid;
   }
   ossimGeoid* getGeoid()
   {
      return m_geoid.get();
   }
   const ossimGeoid* getGeoid()const
   {
      return m_geoid.get();
   }
   
   /**
    * Open a connection to a database.  In most cases this will be a pointer
    * to a directory like in a Dted directory reader.  
    *
    */
   virtual bool open(const ossimString& connectionString)=0;

   const ossimString& getConnectionString()const
   {
      return m_connectionString;
   }

   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
   
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   
   virtual std::ostream& print(std::ostream& out) const;

protected:
   virtual ~ossimElevationDatabase()
   {
      m_geoid = 0;
   }
   virtual double getOffsetFromEllipsoid(const ossimGpt& gpt);

   ossimString m_connectionString;
   ossimRefPtr<ossimGeoid>    m_geoid;
   ossim_float64              m_meanSpacing;
   
TYPE_DATA
   
}; // End: class ossimElevationDatabase

#endif /* #ifndef ossimElevationDatabase_HEADER */
