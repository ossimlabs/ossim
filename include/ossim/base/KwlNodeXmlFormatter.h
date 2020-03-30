#ifndef ossimKwlNodeXmlFormatter_HEADER
#define ossimKwlNodeXmlFormatter_HEADER
#include <ossim/base/KwlNodeFormatter.h>

namespace ossim
{
   class KwlNodeXmlFormatter : public KwlNodeFormatter
   {
   public:
      KwlNodeXmlFormatter(std::shared_ptr<KwlNode> kwlNode,
                          const ossimString& rootTag=ossimString(""))
         : KwlNodeFormatter(kwlNode),
           m_rootTag(rootTag) {}
      KwlNodeXmlFormatter(const ossimKeywordlist &kwl,
                          const ossimString &rootTag = ossimString(""))
          : KwlNodeFormatter(kwl),
            m_rootTag(rootTag) {}
      virtual void write(std::ostream &out,
                        const FormatHints &hints = FormatHints()) const;

   protected:
      void toXML(std::ostream &out, const KwlNode *currentNode,
               int indent = 0,
               const FormatHints &hints = FormatHints()) const;

      ossimString outputAttributesXml(const KwlNode::ChildMap &attributes) const;

      ossimString replaceSpecialCharactersXML(const ossimString &value) const;
      bool needsCdata(const ossimString &value) const;

      ossimString m_rootTag;
   };
} // namespace ossim

#endif