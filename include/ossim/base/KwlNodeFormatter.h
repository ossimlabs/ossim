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
         enum FormatFlags
         {
            FORMAT_HINTS_NO_OPTION_FLAGS = 0,
            FORMAT_HINTS_PRETTY_PRINT_FLAG = 1,
            FORMAT_HINTS_OUTPUT_DOCUMENT_HEADER_FLAG = 2,
            FORMAT_HINTS_UPCASE_PARENT_TAGS_FLAG = 4,
            FORMAT_HINTS_ALL = (FORMAT_HINTS_PRETTY_PRINT_FLAG | FORMAT_HINTS_OUTPUT_DOCUMENT_HEADER_FLAG | FORMAT_HINTS_UPCASE_PARENT_TAGS_FLAG)
         };
         FormatHints(ossim_uint32 indent = 3,
                     FormatFlags formatFlags = FORMAT_HINTS_NO_OPTION_FLAGS)
             : m_indent(indent),
               m_formatFlags(formatFlags)
         {
         }
         ossim_uint32 indent()const { return m_indent; }
         void setIndent(ossim_uint32 indent) { m_indent = indent; }
         bool prettyPrint() const { return m_formatFlags & FORMAT_HINTS_PRETTY_PRINT_FLAG; }
         bool outputDocumentHeader() const { return m_formatFlags & FORMAT_HINTS_OUTPUT_DOCUMENT_HEADER_FLAG; }
         bool upcaseParentTag() const { return m_formatFlags & FORMAT_HINTS_UPCASE_PARENT_TAGS_FLAG; }
         void setFormatFlags(FormatFlags flags) { m_formatFlags = flags;}
         FormatFlags getFormatFlags()const { return m_formatFlags;}
      protected:
         ossim_uint32 m_indent;
         FormatFlags m_formatFlags;
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
