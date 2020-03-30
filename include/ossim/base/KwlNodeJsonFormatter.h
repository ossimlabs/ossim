#ifndef ossimKwlNodeJsonFormatter_HEADER
#define ossimKwlNodeJsonFormatter_HEADER
#include <ossim/base/KwlNodeFormatter.h>

namespace ossim
{
   class KwlNodeJsonFormatter : public KwlNodeFormatter
   {
   public:
      KwlNodeJsonFormatter(std::shared_ptr<KwlNode> kwlNode,
                           const ossimString &rootTag = ossimString(""))
          : KwlNodeFormatter(kwlNode),
            m_rootTag(rootTag)
      {
      }
      KwlNodeJsonFormatter(const ossimKeywordlist &kwl,
                           const ossimString &rootTag = ossimString(""))
          : KwlNodeFormatter(kwl),
            m_rootTag(rootTag) {}
      virtual void write(std::ostream &out,
                        const FormatHints &hints = FormatHints()) const;

   protected:
      void toJSON(std::ostream &out, const KwlNode *currentNode,
                  int indent = 0,
                  const FormatHints &hints = FormatHints()) const;
      void toJSONAttributes(std::ostream &out,
                           const KwlNode *currentNode,
                           int indent = 0,
                           const FormatHints &hints = FormatHints()) const;
      void toJSONChildren(std::ostream &out,
                        const KwlNode *currentNode,
                        int indent = 0,
                        const FormatHints &hints = FormatHints()) const;
      void toJSONArrayType(std::ostream &out,
                           const KwlNode *currentNode,
                           int indent = 0,
                           const FormatHints &hints = FormatHints()) const;
      ossimString fixValue(const ossimString &value) const;

      ossimString m_rootTag;
   };
}
#endif
