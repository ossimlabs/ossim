#ifndef ossimKwlNodeKwlFormatter_HEADER
#define ossimKwlNodeKwlFormatter_HEADER
#include <ossim/base/KwlNodeFormatter.h>

namespace ossim
{
/*
* @code
* @endcode
* SAMPLE OUTPUT:
*
* @code
* @endcode
*/
class KwlNodeKwlFormatter : public KwlNodeFormatter
{
public:
   KwlNodeKwlFormatter(std::shared_ptr<KwlNode> kwlNode,
                       const ossimString &rootTag = ossimString(""))
       : KwlNodeFormatter(kwlNode),
         m_rootTag(rootTag)
   {
   }
   KwlNodeKwlFormatter(const ossimKeywordlist &kwl,
                       const ossimString &rootTag = ossimString(""))
       : KwlNodeFormatter(kwl),
         m_rootTag(rootTag) {}
   virtual void write(std::ostream &out,
                      const FormatHints &hints = FormatHints()) const;

protected:
   void saveState(ossimKeywordlist &kwl,
                  const KwlNode &node,
                  const ossimString &prefix = ossimString("")) const;

   ossimString m_rootTag;
};
} // namespace ossim
#endif
