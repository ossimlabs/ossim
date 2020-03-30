#include <ossim/base/KwlNodeJsonFormatter.h>


namespace ossim
{
   void KwlNodeJsonFormatter::write(std::ostream &out,
                                 const FormatHints &hints) const
   {
      ossim_uint32 nChildren = m_kwlNode->getChildren().size();
      ossim_uint32 childIdx = 0;
      if(!m_rootTag.empty())
      {
         out << "{\"" << m_rootTag << "\" : ";
      }
      out << "{";
      for (auto n : m_kwlNode->getChildren())
      {
         toJSON(out, n.second.get(), 0, hints);
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << ",";
         }
      }
      out << "}";
      if (!m_rootTag.empty())
      {
         out << "}";
      }
   }

   void KwlNodeJsonFormatter::toJSON(std::ostream &out, const KwlNode *currentNode,
                                     int indent,
                                     const FormatHints &hints) const
   {
      std::string indentStr = hints.m_prettyPrint ? std::string(indent, ' ') : "";
      std::string separator = hints.m_prettyPrint ? "\n" : "";
      bool keyEmpty = currentNode->getKey().empty();
      ossimString value = currentNode->getValue();
      bool needsCommaForAttributes = false;

      if (currentNode->getIsArray())
      {
         out << "\"" << currentNode->getKey() << "\" : ";
         toJSONArrayType(out, currentNode, indent, hints);
      }
      else
      {
         // special case for attributes
         needsCommaForAttributes = true;

         if (currentNode->hasChildren() ||
             currentNode->hasAttributes())
         {
            out << "\"" << currentNode->getKey() << "\" : ";
            out << "{";
            toJSONChildren(out, currentNode, indent, hints);
            if (currentNode->hasAttributes())
            {
               bool needsCommaForAttributes = currentNode->hasChildren();
               if (needsCommaForAttributes)
                  out << ",";
               toJSONAttributes(out, currentNode, indent, hints);
            }
            out << "}";
         }
         else
         {
            std::string indentValueStr = hints.m_prettyPrint ? std::string(indent + hints.m_indent, ' ') : "";
            out << "\"" << currentNode->getKey() << "\" : \""
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
      ossim_uint32 nChildren = currentNode->getAttributes().size();
      ossim_uint32 childIdx = 0;
      for (auto attribute : currentNode->getAttributes())
      {
         out << "\"" << attribute.second->getKey()
             << "\" : \"" << fixValue(attribute.second->getValue()) << "\"";
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << ",";
         }
      }
   }
   void KwlNodeJsonFormatter::toJSONChildren(std::ostream &out,
                                             const KwlNode *currentNode,
                                             int indent,
                                             const FormatHints &hints) const
   {
      ossim_uint32 childIdx = 0;
      ossim_uint32 nChildren = currentNode->getChildren().size();
      if (!nChildren)
         return;
      for (auto child : currentNode->getChildren())
      {
         toJSON(out, child.second.get(), indent + hints.m_indent, hints);
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << ",";
         }
      }
   }
   void KwlNodeJsonFormatter::toJSONArrayType(std::ostream &out,
                                              const KwlNode *currentNode,
                                              int indent,
                                              const FormatHints &hints) const
   {
      ossim_uint32 nChildren = currentNode->getChildren().size();
      ossim_uint32 childIdx = 0;
      out << "[";
      // sort the child array
      std::map<ossim_int32, std::shared_ptr<KwlNode>> sortedMap;
      for (auto x : currentNode->getChildren())
      {
         sortedMap.insert(std::make_pair(x.first.toInt32(), x.second));
      }
      // iterate through sorted array
      for (auto child : sortedMap)
      {
         if (child.second->hasChildren())
         {
            ossim_uint32 nGrandChildren = child.second->getChildren().size();
            ossim_uint32 grandChildIdx = 0;
            out << "{";
            for (auto skippedChild : child.second->getChildren())
            {
               toJSON(out, skippedChild.second.get(), indent + hints.m_indent, hints);
               ++grandChildIdx;
               if (grandChildIdx != nGrandChildren)
               {
                  out << ",";
               }
            }
            out << "}";
         }
         else
         {
            out << "\"" << fixValue(child.second->getValue()) << "\"";
         }
         ++childIdx;
         if (childIdx != nChildren)
         {
            out << ",";
         }
      }
      out << "]";
   }

   ossimString KwlNodeJsonFormatter::fixValue(const ossimString &value) const
   {
      ossimString result(value);

      result = result.substitute("\n", "\\n", true);
      result = result.substitute("\"", "'", true);

      return result;
   }
}