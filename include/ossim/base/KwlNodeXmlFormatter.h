#ifndef ossimKwlNodeXmlFormatter_HEADER
#define ossimKwlNodeXmlFormatter_HEADER

#include <ossim/base/KwlNodeFormatter.h>

/*
* @code
 #include <ossim/base/ossimKeywordlist.h>
 #include <ossim/base/KwlNode.h>
 #include <ossim/base/KwlNodeXmlFormatter.h>
 int main()int argc, char* argv[])
 {
    kwl.add("tiff.@version", "1");
   kwl.add("tiff.hello.property1", "value1");
   kwl.add("tiff.hello.property2", "value2");
   kwl.add("tiff.what1.object1.dd", "hey1");
   kwl.add("tiff.what2.object2.dd", "hey2");

   std::shared_ptr<ossim::KwlNodeFormatter> formatter =
       std::make_shared<ossim::KwlNodeXmlFormatter>(kwl);
   ossim::KwlNodeFormatter::FormatHints hints(3, ossim::KwlNodeFormatter::FormatHints::FORMAT_HINTS_PRETTY_PRINT_FLAG);
   formatter->write(std::cout, hints);
}
* @endcode
* SAMPLE OUTPUT:
*
* @code
<tiff version="1">
   <hello>
      <property>value1</property>
      <property>value2</property>
   </hello>
   <what>
      <object>
         <dd>hey1</dd>
      </object>
   </what>
   <what>
      <object>
         <dd>hey2</dd>
      </object>
   </what>
</tiff>
* @endcode
*/

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