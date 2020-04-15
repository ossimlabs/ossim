#ifndef ossimKwlNodeKwlFormatter_HEADER
#define ossimKwlNodeKwlFormatter_HEADER
#include <ossim/base/KwlNodeFormatter.h>

namespace ossim
{
/*
* @code
 #include <ossim/base/ossimKeywordlist.h>
 #include <ossim/base/KwlNode.h>
 #include <ossim/base/KwlNodeKwlFormatter.h>
 int main()int argc, char* argv[])
 {
   ossimKeywordlist kwl;
   kwl.add("tiff.@version", "1");
   kwl.add("tiff.hello.property1", "value1");
   kwl.add("tiff.hello.property2", "value2");
   kwl.add("tiff.what1.object1.dd", "hey1");
   kwl.add("tiff.what2.object2.dd", "hey2");

   std::shared_ptr<ossim::KwlNodeFormatter> formatter =
       std::make_shared<ossim::KwlNodeKwlFormatter>(kwl);
   formatter->write(std::cout);


   return 0
}

* @endcode
* SAMPLE OUTPUT:
*
* @code
tiff.@version:  1
tiff.hello.property1:  value1
tiff.hello.property2:  value2
tiff.what1.object1.dd:  hey1
tiff.what2.object2.dd:  hey2
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
