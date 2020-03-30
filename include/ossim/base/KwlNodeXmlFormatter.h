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
       std::make_shared<ossim::KwlNodeXmlFormatter>(kwl);

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
<?xml version='1.0'?>
<TIFF version="1">
   <WHAT f="hey2">
      <ff>
         hey3
      </ff>
      hey2
   </WHAT>
   <what>
      hey2
   </what>
   <what>
      hey3
   </what>
   <WHAT>
      <ADS>
         <dd>
            hey3
         </dd>
      </ADS>
   </WHAT>
   <WHAT>
      <ADS>
         <dd>
            hey4
         </dd>
      </ADS>
   </WHAT>
</TIFF>
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