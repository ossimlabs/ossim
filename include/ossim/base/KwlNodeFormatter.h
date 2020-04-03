#ifndef ossimKwlNodeFormatter_HEADER
#define ossimKwlNodeFormatter_HEADER
#include <ossim/base/KwlNode.h>

namespace ossim
{
   class KwlNodeFormatter
   {
   public:
      class FormatHints
      {
      public:
         FormatHints(int indent = 3,
                     bool prettyPrint = false,
                     bool outputDocumentHeader = false,
                     bool upcaseParentTags = false)
             : m_indent(indent),
               m_prettyPrint(prettyPrint),
               m_outputDocumentHeader(outputDocumentHeader),
               m_upcaseParentTags(upcaseParentTags)
         {
         }

         int m_indent;
         bool m_prettyPrint;
         bool m_outputDocumentHeader;
         bool m_upcaseParentTags;
      };
      KwlNodeFormatter(std::shared_ptr<KwlNode> kwlNode) : m_kwlNode(kwlNode)
      {
      }
      KwlNodeFormatter(const ossimKeywordlist &kwl)
      {
         m_kwlNode = KwlNode::createTree(kwl);
      }
      virtual void write(ossimString &result, const FormatHints &hints = FormatHints()) const;
      virtual void write(std::ostream &out, const FormatHints &hints = FormatHints()) const = 0;

   protected:
      std::shared_ptr<KwlNode> m_kwlNode;
   };
}
#endif
