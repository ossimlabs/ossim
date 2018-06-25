//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//*******************************************************************

#ifndef ossimEnviCgModel_HEADER
#define ossimEnviCgModel_HEADER

#include <ossim/projection/ossimCoarseGridModel.h>
#include <ossim/base/ossimFilename.h>

class OSSIMDLLEXPORT ossimEnviCgModel : public ossimCoarseGridModel
{
public:
   ossimEnviCgModel();
   ossimEnviCgModel(const ossimFilename& geoc_file);

   bool loadEnviGeocFile(const ossimFilename& geoc_file);
   bool saveState(ossimKeywordlist& kwl, const char* prefix=0) const;

private:
   double readValue(ifstream& s, bool bigEndian) const;

TYPE_DATA
};

#endif
