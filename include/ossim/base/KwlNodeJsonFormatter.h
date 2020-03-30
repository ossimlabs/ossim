#ifndef ossimKwlNodeJsonFormatter_HEADER
#define ossimKwlNodeJsonFormatter_HEADER
#include <ossim/base/KwlNodeFormatter.h>

namespace ossim
{
/*
* @code
 #include <ossim/base/ossimKeywordlist.h>
 #include <ossim/base/KwlNode.h>
 #include <ossim/base/KwlNodeXmlFormatter.h>
 int main()int argc, char* argv[])
 {
   ossimKeywordlist kwl;

   kwl.add("tiff.@version", "1");
   kwl.add("tiff.what2", "hey1");
   kwl.add("tiff.what1.@f", "hey2");
   kwl.add("tiff.what1", "hey2");
   kwl.add("tiff.what1.ff", "hey3");
   kwl.add("tiff.what10014.ads1.dd", "hey3");
   kwl.add("tiff.what10015.ads2.dd", "hey4");
   kwl.add("tiff.what2", "hey2");
   kwl.add("tiff.what3", "hey3");

   std::shared_ptr<ossim::KwlNodeFormatter> formatter =
       std::make_shared<ossim::KwlNodeJsonFormatter>(kwl);

   formatter->write(std::cout,
                    ossim::KwlNodeFormatter::FormatHints(3, true, true, true));

   std::ostringstream out;
   formatter->write(out,
                    ossim::KwlNodeFormatter::FormatHints(3, true, true, true));

}
* @endcode
* SAMPLE OUTPUT:
*
* @code
{"tiff" : {"what" : [{"ff" : "hey3"},"hey2","hey3",{"ads" : [{"dd" : "hey3"}]},{"ads" : [{"dd" : "hey4"}]}],"version" : "1"}}
* @endcode
*/
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
