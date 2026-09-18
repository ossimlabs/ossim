//----------------------------------------------------------------------------
//
// License:  MIT
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Ryan Feldbush
//
// Description: COMNTA tag class definition.
//
// Exploitation Reference Data TRE.
//
// See document App AU - COMNTA for more info.
//
//----------------------------------------------------------------------------

#include <iomanip>

#include <ossim/support_data/ossimNitfComntaTag.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/support_data/ossimNitfCommon.h>
#include <ossim/support_data/ossimNitfCommonFieldNames.h>

namespace
{
//---
// ossimNitfInfo::getKeywordlist() round-trips print() output back through
// ossimKeywordlist::parseStream(), which is line oriented. A COMNTA comment is
// free text and routinely contains newlines (e.g. a multi-line license), which
// would terminate the value early and discard every tag printed after this one.
// Escape them so the value stays on one line.
//---
std::string escapeNewlines(const std::string& text)
{
   std::string out;
   out.reserve(text.size());
   for (char c : text)
   {
      if (c == '\n')
         out += "\\n";
      else if (c == '\r')
         out += "\\r";
      else
         out += c;
   }
   return out;
}
}

ossimNitfComntaTag::ossimNitfComntaTag()
   : ossimNitfRegisteredTag("COMNTA", 0)
{
   clearFields();
}

/**
 * Parse method.
 *
 * @param in Stream to parse.
 */
void ossimNitfComntaTag::parseStream(std::istream& in)
{
   const ossim_uint32 bytes = getTagLength();
   comment.clear();

   if ( bytes )
   {
      comment.resize(bytes);
      in.read(&comment.front(), bytes);
   }

   setTagLength(computeTagLength());
}

bool ossimNitfComntaTag::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   const char* lookup;
   lookup = kwl.find(prefix, ossim::nitf::COMMENT_KW.c_str());
   if(lookup)
   {
      comment = lookup;
      setTagLength(computeTagLength());
      return true;
   }
   return false;
}

/**
 * Write method.
 *
 * @param out Stream to write to.
 */
void ossimNitfComntaTag::writeStream(std::ostream& out)
{
   out.write(comment.c_str(), getTagLength());
}

/**
 * Clears all string fields within the record to some default nothingness.
 */
void ossimNitfComntaTag::clearFields()
{
   comment = "";
   setTagLength(0);
}

/**
 * @brief Print method that outputs a key/value type format
 * adding prefix to keys.
 * @param out Stream to output to.
 * @param prefix Prefix added to key like "image0.";
 */
std::ostream& ossimNitfComntaTag::print(std::ostream& out,
                            const std::string& prefix) const
{
   std::string pfx = prefix;
   pfx += getTagName();
   pfx += ".";

   out << std::setiosflags(std::ios::left)
       << pfx << std::setw(24) << "CETAG:"
       << getTagName() << "\n"
       << pfx << std::setw(24) << "CEL:"
       << getTagLength() << "\n"
       << pfx << std::setw(24) << "Comment:"
       << escapeNewlines(comment) << "\n";

   return out;
}

ossimString ossimNitfComntaTag::getComment()
{
 return comment;
}
void ossimNitfComntaTag::setComment(const ossimString& fieldValue)
{
   comment = fieldValue.c_str();
}

ossim_uint32 ossimNitfComntaTag::computeTagLength() const
{
   return comment.length();
}
