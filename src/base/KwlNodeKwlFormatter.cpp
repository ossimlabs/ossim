#include <ossim/base/KwlNodeKwlFormatter.h>

namespace ossim
{
void KwlNodeKwlFormatter::write(std::ostream &out,
                                 const FormatHints &hints) const
{
   ossimKeywordlist result;
   ossim_uint32 idx = 0;

   if(!m_kwlNode) return;
   ossimString prefix = m_rootTag + (m_rootTag.empty()?"":".");
   
   if(!m_kwlNode->getKey().empty())
   {
      prefix += m_kwlNode->getKey() + ".";
   }
   for (auto n : m_kwlNode->getChildren())
   {
      saveState(result, *(n.second), prefix);
      ++idx;
   }

   out << result;
}

void KwlNodeKwlFormatter::saveState(ossimKeywordlist& kwl,
                                    const KwlNode &node,
                                    const ossimString &prefix) const
{
   if (node.getIsArray())
   {
      for (auto n : node.getChildren())
      {
         ossimString newPrefix = prefix + node.getKey() + n.first;
         if (n.second->hasChildren())
         {
            for (auto child : n.second->getChildren())
            {
               saveState(kwl, *child.second, newPrefix + ".");
            }
         }
         else
         {
            if (n.second->hasAttributes())
            {
               for(auto attribute : n.second->getAttributes())
               {
                  kwl.add((newPrefix + ".@" + attribute.first.c_str(), attribute.second->getValue().c_str()));
               }
            }
            kwl.add(newPrefix.c_str(), n.second->getValue().c_str());
         }
      }
   }
   else if (node.hasChildren())
   {
      for (auto n : node.getChildren())
      {
         ossimString newPrefix = prefix + node.getKey();
         saveState(kwl, *n.second, newPrefix + ".");
      }
   }
   else
   {
      kwl.add(prefix + node.getKey().c_str(), node.getValue().c_str());
   }
   if (node.hasAttributes())
   {
      for( auto n : node.getAttributes())
      {
         kwl.add((prefix + node.getKey() + ".@" + n.first).c_str(), n.second->getValue().c_str());
      }
   }
}
} // namespace ossim