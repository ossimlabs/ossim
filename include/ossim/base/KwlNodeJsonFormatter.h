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
   kwl.add("tiff.hello.property1", "value1");
   kwl.add("tiff.hello.property2", "value2");
   kwl.add("tiff.what1.object1.dd", "hey1");
   kwl.add("tiff.what2.object2.dd", "hey2");

   std::shared_ptr<ossim::KwlNodeFormatter> formatter =
       std::make_shared<ossim::KwlNodeJsonFormatter>(kwl);
   ossim::KwlNodeFormatter::FormatHints hints(3, ossim::KwlNodeFormatter::FormatHints::FORMAT_HINTS_PRETTY_PRINT_FLAG);
   formatter->write(std::cout, hints);
   
   std::ostringstream out;
   formatter->write(out, hints);

}
* @endcode
* SAMPLE OUTPUT:
*
* @code
{
   "tiff" : {
      "hello" : {
         "property" : [
            "value1",
            "value2"
         ]
      },
      "what" : [
         {
            "object" : [
               {
                  "dd" : "hey1"
               }
            ]
         },
         {
            "object" : [
               {
                  "dd" : "hey2"
               }
            ]
         }
      ],
      "version" : "1"
   }
}
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
