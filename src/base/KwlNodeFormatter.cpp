#include <ossim/base/KwlNodeFormatter.h>
#include <sstream>

namespace ossim
{
   void KwlNodeFormatter::write(ossimString &result, const FormatHints &hints) const
   {
      std::ostringstream out;

      write(out, hints);

      result = out.str();
   }
} // namespace ossim
