#include <ossim/base/KwlNodeJsonFormatter.h>


namespace ossim
{
   void KwlNodeJsonFormatter::write(std::ostream &out,
                                 const FormatHints &hints) const
   {
      ossim_uint32 indent = hints.indent();
      std::string separator = hints.prettyPrint() ? "\n" : "";
      std::string indentStr = hints.prettyPrint() ? std::string(hints.indent(), ' ') : "";
      ossim_uint32 nChildren = m_kwlNode->getChildren().size();
      ossim_uint32 childIdx = 0;
      if(!m_rootTag.empty())
      {
         indent += hints.indent();
         out << "{" << separator << indentStr << "\"" << m_rootTag << "\" : {" << separator;
      }
      else
      {
         out << "{" << separator;
      }
      
      for (auto n : m_kwlNode->getChildren())
      {
         toJSON(out, n.second.get(), indent, hints);
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << "," << separator;
         }
      }
      if(indent)
      {
         out << indentStr << separator << "}" << separator;
      }
      else
      {
         out << separator << "}" << separator;
      }
      if (!m_rootTag.empty())
      {
         out << separator << "}" << separator;
      }
   }

   void KwlNodeJsonFormatter::toJSON(std::ostream &out, 
                                     const KwlNode *currentNode,
                                     int indent,
                                     const FormatHints &hints) const
   {
      std::string indentStr = hints.prettyPrint() ? std::string(indent, ' ') : "";
      std::string separator = hints.prettyPrint() ? "\n" : "";
      bool keyEmpty = currentNode->getKey().empty();
      ossimString value = currentNode->getValue();
      bool needsCommaForAttributes = false;

      if (currentNode->getIsArray())
      {
         out << indentStr << "\"" << currentNode->getKey() << "\" : ";
         toJSONArrayType(out, currentNode, indent, hints);
      }
      else
      {
         // special case for attributes
         needsCommaForAttributes = true;

         if (currentNode->hasChildren() ||
             currentNode->hasAttributes())
         {
            out << indentStr << "\"" << currentNode->getKey() << "\" : ";
            out << "{" << separator;
            toJSONChildren(out, currentNode, indent, hints);
            if (currentNode->hasAttributes())
            {
               bool needsCommaForAttributes = currentNode->hasChildren();
               if (needsCommaForAttributes)
                  out << "," << separator;
               toJSONAttributes(out, currentNode, indent + hints.indent(), hints);
            }
            out << separator << indentStr << "}";
         }
         else
         {
            std::string indentValueStr = hints.prettyPrint() ? std::string(indent + hints.indent(), ' ') : "";
            out << indentStr << "\"" << currentNode->getKey() << "\" : \""
                << fixValue(currentNode->getValue()) << "\"";

            // toJSONAttributes(out, currentNode, indent, hints);
         }
      }
   }
   void KwlNodeJsonFormatter::toJSONAttributes(std::ostream &out,
                                               const KwlNode *currentNode,
                                               int indent,
                                               const FormatHints &hints) const
   {
      std::string indentStr = hints.prettyPrint() ? std::string(indent, ' ') : "";
      std::string separator = hints.prettyPrint() ? "\n" : "";
      ossim_uint32 nChildren = currentNode->getAttributes().size();
      ossim_uint32 childIdx = 0;
      for (auto attribute : currentNode->getAttributes())
      {
         out << indentStr << "\"" << attribute.second->getKey()
             << "\" : \"" << fixValue(attribute.second->getValue()) << "\"";
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << "," << separator;
         }
      }
   }
   void KwlNodeJsonFormatter::toJSONChildren(std::ostream &out,
                                             const KwlNode *currentNode,
                                             int indent,
                                             const FormatHints &hints) const
   {
      std::string indentStr = hints.prettyPrint() ? std::string(indent, ' ') : "";
      std::string separator = hints.prettyPrint() ? "\n" : "";
      ossim_uint32 childIdx = 0;
      ossim_uint32 nChildren = currentNode->getChildren().size();
      if (!nChildren)
         return;
      for (auto child : currentNode->getChildren())
      {
         toJSON(out, child.second.get(), indent + hints.indent(), hints);
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << "," << separator;
         }
      }
   }
   void KwlNodeJsonFormatter::toJSONArrayType(std::ostream &out,
                                              const KwlNode *currentNode,
                                              int indent,
                                              const FormatHints &hints) const
   {
      std::string indentStr = hints.prettyPrint() ? std::string(indent, ' ') : "";
      std::string indentStr2 = hints.prettyPrint() ? std::string(indent + hints.indent(), ' ') : "";
      std::string separator = hints.prettyPrint() ? "\n" : "";
      ossim_uint32 nChildren = currentNode->getChildren().size();
      ossim_uint32 childIdx = 0;
      out << "[" << separator;
      // sort the child array
      std::map<ossim_int32, std::shared_ptr<KwlNode>> sortedMap;
      for (auto x : currentNode->getChildren())
      {
         sortedMap.insert(std::make_pair(x.first.toInt32(), x.second));
      }
      // iterate through sorted array
      for (auto child : sortedMap)
      {
         ++childIdx;
         if (child.second->hasChildren())
         {
            ossim_uint32 nGrandChildren = child.second->getChildren().size();
            ossim_uint32 grandChildIdx = 0;
            out << indentStr2 << "{" << separator;
            for (auto skippedChild : child.second->getChildren())
            {
               ++grandChildIdx;
               toJSON(out, skippedChild.second.get(), indent + hints.indent() * 2, hints);
               if (grandChildIdx < nGrandChildren)
               {
                  out << "," << separator;
               }
            }
            out << separator << indentStr2 << "}";
         }
         else
         {
            out << indentStr2 << "\"" << fixValue(child.second->getValue()) << "\"";
         }
         if (childIdx < nChildren)
         {
            out << "," << separator;
         }
      }
      out << separator << indentStr << "]";
   }

   ossimString KwlNodeJsonFormatter::fixValue(const ossimString &value) const
   {
      ossimString result(value);

      result = result.substitute("\n", "\\n", true);
      result = result.substitute("\"", "'", true);

      return result;
   }
}