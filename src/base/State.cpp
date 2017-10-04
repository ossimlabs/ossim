#include <ossim/base/State.h>


void ossim::State::load(const ossimKeywordlist& kwl,
                        const ossimString& prefix)
{
}

void ossim::State::save(ossimKeywordlist& kwl,
                        const ossimString& prefix)const
{
   kwl.add(prefix, "type", getTypeName(), true);
}
