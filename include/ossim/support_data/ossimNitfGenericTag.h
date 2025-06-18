//---
// File: ossimNitfGenericTag.h
//---

#include <ossim/support_data/ossimNitfRegisteredTag.h>
#include <vector>
#include <map>
#ifndef ossimNitfGenericTag_HEADER
#define ossimNitfGenericTag_HEADER 1

/**
 * @class ossimNitfGenericTag
 */
class OSSIM_DLL ossimNitfGenericTag : public ossimNitfRegisteredTag
{
public:
    ossimNitfGenericTag();

    virtual void parseStream(std::istream& in);

    virtual void writeStream(std::ostream& out);

    virtual void clearFields();

    virtual std::ostream& print(std::ostream& out,
                                const std::string& prefix) const;

    ossimString get(ossimString fieldName);

    static const ossim_int32 NUM_DEFINITIONS;
    static std::pair<ossimString, ossim_int32> FIELD_DEFINITIONS[];


protected:

    std::map<ossimString, ossimString> m_fields_map;
    std::vector<std::pair<ossimString, ossimString>> m_fields_vector;


};

#endif
