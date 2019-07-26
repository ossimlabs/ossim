#include <ossim/elevation/ossimElevationDatabase.h>

RTTI_DEF1(ossimElevationDatabase, "ossimElevationDatabase", ossimObject);

using namespace std;

double ossimElevationDatabase::getOffsetFromEllipsoid(const ossimGpt& gpt)
{
   double result = 0.0;
   if(m_geoid.valid())
   {
      result = m_geoid->offsetFromEllipsoid(gpt);
   }
   else 
   {
      result = ossimGeoidManager::instance()->offsetFromEllipsoid(gpt);
   }
   
   if(ossim::isnan(result))
   {
      result = 0.0;
   }
   
   return result;
}

bool ossimElevationDatabase::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   // Connection string:
   m_connectionString = kwl.find(prefix, "connection_string");
   if(m_connectionString.empty())
   {
      // Try backward compatability to a filename:
      m_connectionString = kwl.find(prefix, ossimKeywordNames::FILENAME_KW);
   }

   // Geoid:
   ossimString value = kwl.find(prefix, "geoid.type");
   if( value.size() )
   {
      m_geoid = ossimGeoidManager::instance()->findGeoidByShortName(value);
   }
   
   // Mean spacing:
   value = kwl.find(prefix, "mean_spacing");
   if( value.size() )
   {
      m_meanSpacing = value.toFloat64();
   }
   
   return ossimSource::loadState(kwl, prefix);
}

bool ossimElevationDatabase::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, "connection_string", m_connectionString, true);
   
   if(m_geoid.valid())
   {
      kwl.add(prefix, "geoid.type", m_geoid->getShortName(), true);
   }
   
   return ossimSource::saveState(kwl, prefix);
}

std::ostream& ossimElevationDatabase::print(ostream& out) const
{
   ossimKeywordlist kwl;
   saveState(kwl);
   out << "\nossimElevationDatabase @ "<< (ossim_uint64) this
         << kwl <<ends;
   return out;
}
