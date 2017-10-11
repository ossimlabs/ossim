#include <ossim/base/State.h>


bool ossim::State::load(const ossimKeywordlist& kwl,
                        const ossimString& prefix)
{
   return true;
}

bool ossim::State::save(ossimKeywordlist& kwl,
                        const ossimString& prefix)const
{
   kwl.add(prefix, "type", getTypeName(), true);

   return true;
}
