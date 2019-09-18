//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************
#ifndef JsonInterface_HEADER
#define JsonInterface_HEADER 1

#include <ossim/json/json.h>

namespace ossim {

/**
 * Pure virtual interface for classes implementing JSON-based load/save state..
 * Refer to <a href="https://docs.google.com/document/d/1DXekmYm7wyo-uveM7mEu80Q7hQv40fYbtwZq-g0uKBs/edit?usp=sharing">3DISA API document</a>
 * for JSON formats used.
 */
class JsonInterface
{
public:
   JsonInterface() {}
   virtual ~JsonInterface() {}

   virtual void loadJSON(const Json::Value& jsonNode) = 0;
   virtual void saveJSON(Json::Value& jsonNode) const = 0;

};

}
#endif
