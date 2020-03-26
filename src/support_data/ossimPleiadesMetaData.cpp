#include <ossim/support_data/ossimPleiadesMetaData.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimXmlDocument.h>

ossimPleiadesMetaData::ossimPleiadesMetaData()
{

}

bool ossimPleiadesMetaData::open(const ossimFilename& filename)
{
   bool result = false;
   m_kwl.clear();

   ossimRefPtr<ossimXmlDocument> xml = new ossimXmlDocument();

   if(xml->openFile(filename))
   {
      m_kwl.clear();
      xml->toKwl(m_kwl);
      ossimString format = m_kwl.findKey("Dimap_Document.Metadata_Identification.METADATA_FORMAT");
      if(!format.empty())
      {
         result = true;
      }
      // std::cout << m_kwl << "\n";
   }

       // std::shared_ptr<ossim::istream> in =
       //     ossim::StreamFactoryRegistry::instance()->createIstream(filename.c_str());

       // if(in)
       // {
       //    result = true;
       // }
   return result;
}

ossimString ossimPleiadesMetaData::getMissionId() const
{
   ossimString mission      = m_kwl.findKey("Dimap_Document.Strip_Identification.Mission_Identification.MISSION");
   ossimString missionIndex = m_kwl.findKey("Dimap_Document.Strip_Identification.Mission_Identification.MISSION_INDEX");
   return mission+missionIndex;
}

ossimString ossimPleiadesMetaData::getSensorId() const
{
   return m_kwl.findKey("Dimap_Document.Strip_Identification.Instrument_Identification.INSTRUMENT_TYPE");
}

ossimString ossimPleiadesMetaData::getAcquisitionDate() const
{
   return m_kwl.findKey("Dimap_Document.Acquisition_Configuration.Instrument_Configuration.Time_Configuration.Time_Range.END");
}

ossimString ossimPleiadesMetaData::getStartTime() const
{
   return m_kwl.findKey("Dimap_Document.Acquisition_Configuration.Instrument_Configuration.Time_Configuration.Time_Range.START");
}

ossimString ossimPleiadesMetaData::getEndTime() const
{
   return m_kwl.findKey("Dimap_Document.Acquisition_Configuration.Instrument_Configuration.Time_Configuration.Time_Range.END");
}

bool ossimPleiadesMetaData::saveState(ossimKeywordlist &kwl,
                                    const ossimString &prefix) const
{
   kwl.add(prefix.c_str(), m_kwl);
   kwl.add(prefix + "acquisition_date", getAcquisitionDate());
   kwl.add(prefix + "mission_id", getMissionId());
   kwl.add(prefix + "sensor_id", getSensorId());

   return true;
}