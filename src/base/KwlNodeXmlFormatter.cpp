#include <ossim/base/KwlNodeXmlFormatter.h>
#include <sstream>

namespace ossim
{
   void KwlNodeXmlFormatter::write(std::ostream &out,
                                 const FormatHints &hints) const
   {
      std::string separator = hints.prettyPrint() ? "\n" : "";
      if (hints.outputDocumentHeader())
      {
         out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << separator;
      }
      if(!m_rootTag.empty())
      {
         out << "<" << (hints.upcaseParentTag()?m_rootTag.upcase():m_rootTag) << ">" << separator;
      }
      for (auto n : m_kwlNode->getChildren())
      {
         toXML(out, n.second.get(), m_rootTag.empty()?0:hints.indent(), hints);
      }
      if (!m_rootTag.empty())
      {
         out << "</" << (hints.upcaseParentTag() ? m_rootTag.upcase() : m_rootTag) << ">" << separator;
      }
      //toXML(out, m_kwlNode.get(), 0, hints);
   }
   void KwlNodeXmlFormatter::toXML(std::ostream &out, const KwlNode *currentNode,
                                 int indent,
                                 const FormatHints &hints) const
   {
      bool hasChildren = false;
      std::string indentStr = hints.prettyPrint() ? std::string(indent, ' ') : "";
      std::string separator = hints.prettyPrint() ? "\n" : "";
      bool keyEmpty = currentNode->getKey().empty();
      ossimString value = currentNode->getValue();

      if (currentNode->getIsArray())
      {
         // sort the child array
         std::map<ossim_int32, std::shared_ptr<KwlNode> > sortedMap;
         for (auto x : currentNode->getChildren())
         {
            sortedMap.insert(std::make_pair(x.first.toInt32(), x.second));
         }
         ossimString parentKey = currentNode->getKey();
         // if (hints.upcaseParentTag() && currentNode->hasChildren())
         // {
         //    parentKey = parentKey.upcase();
         // }
         // iterate through sorted array
         for (auto child : sortedMap)
         {
            ossimString idxString;
            bool hasGrandChildren = false;
            ossimString key = parentKey;
            if (child.second->hasChildren()&&hints.upcaseParentTag())
            {
               key = key.upcase();
            }
            if(!child.second->hasChildren()) idxString = ossimString::toString(child.first);
            out << indentStr << "<" << replaceSpecialCharactersXML(key)
                << outputAttributesXml(child.second->getAttributes())+idxString << ">";
            if (child.second->hasChildren())
            {
               hasGrandChildren = true;
               out << separator;
               for (auto skippedChild : child.second->getChildren())
               {
                  toXML(out, skippedChild.second.get(), indent + hints.indent(), hints);
               }
            }
            if (!child.second->getValue().empty())
            {
               std::string indentValueStr = hints.prettyPrint() ? std::string(indent + hints.indent(), ' ') : "";
               ossimString childValue = child.second->getValue();
               if (childValue.startsWith("<\\?xml"))
               {
                  childValue = "XML Not Converted";
               }

               if (needsCdata(childValue))
               {
                  // we do not support currently trying to embed an xml 
                  // document inside another XML tag
                  //
                 // out << indentValueStr << "<![CDATA[" << childValue << "]]>" << separator;
                  out << "<![CDATA[" << childValue << "]]>";
               }
               else
               {
                  //out << indentValueStr << childValue << separator;
                  out << childValue ;
               }
            }
            out << (hasGrandChildren?indentStr:"") << "</" << replaceSpecialCharactersXML(key)+idxString << ">" << separator;
         }
      }
      else
      {
         ossimString key = currentNode->getKey();
         if(currentNode->hasChildren())
         {
            hasChildren = true;
            key = hints.upcaseParentTag()?key.upcase():key;
         }
         if (!keyEmpty)
         {
            out << indentStr << "<" << replaceSpecialCharactersXML(key)
                << outputAttributesXml(currentNode->getAttributes()) << ">"
                << (currentNode->hasChildren()?separator:"");
            // out << indentStr << "<" << replaceSpecialCharactersXML(key)
            //     << outputAttributesXml(currentNode->getAttributes()) << ">" << separator;
         }
         for (auto child : currentNode->getChildren())
         {
            toXML(out, child.second.get(), indent + hints.indent(), hints);
         }
         if (!value.empty())
         {
            std::string indentValueStr = hints.prettyPrint() ? std::string(indent + hints.indent(), ' ') : "";
            if (value.startsWith("<\\?xml"))
            {
               value = "XML Not Converted";
            }
            if (needsCdata(value))
            {
               //out << indentValueStr << "<![CDATA[" << value << "]]>" << separator;
               out << "<![CDATA[" << value << "]]>";
            }
            else
            {
               // out << indentValueStr << value << separator;
               out << value;
            }
         }
         if (!keyEmpty)
         {
            out << (hasChildren?indentStr:"") << "</" << replaceSpecialCharactersXML(key) << ">" << separator;
         }
      }
   }
   ossimString KwlNodeXmlFormatter::outputAttributesXml(const KwlNode::ChildMap &attributes) const
   {
      std::ostringstream out;

      for (auto attribute : attributes)
      {
         out << " " << attribute.second->getKey() << "=\"" << attribute.second->getValue() << "\"";
      }

      return ossimString((out.str()));
   }

   ossimString KwlNodeXmlFormatter::replaceSpecialCharactersXML(const ossimString &value) const

   {
      ossimString result = value;
      ossimString::iterator iter = result.begin();

      while (iter != result.end())
      {
         if (!(isdigit(*iter) ||
               isalpha(*iter) ||
               (*iter == '/')))
         {
            *iter = '_';
         }
         ++iter;
      }

      return result;
   }

   bool KwlNodeXmlFormatter::needsCdata(const ossimString &value) const
   {
      for (ossimString::const_iterator it = value.begin(); it != value.end(); ++it)
      {
         switch (*it)
         {
         case '&':
         case '<':
         case '>':
         case '"':
         case '\n':
         case '\'':
         {
            return true;
         }
         default:
         {
            break;
         }
         }
      }
      return false;
   }
}