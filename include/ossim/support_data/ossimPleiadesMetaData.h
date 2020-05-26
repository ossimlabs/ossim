#ifndef ossimPleiades_HEADER
#define ossimPleiades_HEADER
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimFilename.h>

class OSSIM_DLL ossimPleiadesMetaData
{
public:
   ossimPleiadesMetaData();

   bool open(const ossimFilename& filename);

   ossimString getMissionId() const;
   ossimString getSensorId() const;
   ossimString getAcquisitionDate() const;
   ossimString getStartTime() const;
   ossimString getEndTime() const;
   bool saveState(ossimKeywordlist &kwl,
                  const ossimString &prefix = ossimString()) const;

protected:
   ossimKeywordlist m_kwl;

};

#endif
