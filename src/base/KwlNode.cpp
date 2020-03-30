#include <ossim/base/KwlNode.h>

namespace ossim
{
   std::shared_ptr<KwlNode> KwlNode::createTree(const ossimKeywordlist &kwl, 
                                                const ossimString &rootTag)
   {
      std::shared_ptr<KwlNode> result = std::make_shared<KwlNode>(rootTag);

      for (auto x : kwl.getMap())
      {
         result->loadPath(x.first, x.second);
      }

      return result;
   }

   std::shared_ptr<KwlNode> KwlNode::findChild(ChildMap &childMap,
                                      const ossimString &key)
   {
      std::shared_ptr<KwlNode> result;

      ChildMap::iterator iter = childMap.find(key);
      bool found = iter != childMap.end();

      if (found)
      {
         result = iter->second;
      }

      return result;
   }
   std::shared_ptr<const KwlNode> KwlNode::findChild(const ChildMap &childMap,
                                            const ossimString &key)
   {
      std::shared_ptr<const KwlNode> result;

      ChildMap::const_iterator iter = childMap.find(key);
      bool found = iter != childMap.end();

      if (found)
      {
         result = iter->second;
      }

      return result;
   }
   void KwlNode::loadPath(std::vector<ossimString> &q, const ossimString &value)
   {
      KwlNode *currentNode = this;
      ossim_uint32 idx = 0;
      bool attributeFlag = false;
      for (idx = 0; (idx < q.size()) && (!attributeFlag); ++idx)
      {
         ossimString tempKey = q[idx];
         ossimString arrayNumber = "";
         bool arrayFlag = checkIfArray(q[idx]);
         attributeFlag = checkIfAttribute(q[idx]);
         bool lastOne = (idx == q.size() - 1) || attributeFlag;

         if (attributeFlag)
         {
            tempKey = tempKey.after("@");
            currentNode->addOrSetAttribute(tempKey, value);
         }
         else
         {
            if (arrayFlag)
            {
               extractKeyAndIndex(q[idx], tempKey, arrayNumber);
            }
            std::shared_ptr<KwlNode> child = currentNode->findChild(tempKey);
            if (!child)
            {
               child = std::make_shared<KwlNode>(tempKey);
               currentNode->m_children.insert(std::make_pair(tempKey, child));
            }
            currentNode = child.get();
            currentNode->setArrayFlag(arrayFlag);

            if (arrayFlag)
            {
               child = currentNode->findChild(arrayNumber);
               if (!child)
               {
                  child = std::make_shared<KwlNode>(arrayNumber);
                  currentNode->m_children.insert(std::make_pair(arrayNumber, child));
               }
               currentNode = child.get();
            }
            if (lastOne)
            {
               currentNode->setValue(value);
            }
         }
      }
   }
   void KwlNode::addOrSetAttribute(const ossimString &key, const ossimString &value)
   {
      std::shared_ptr<KwlNode> attribute = findAttribute(key);
      if (attribute)
      {
         attribute->setValue(value);
      }
      else
      {
         attribute = std::make_shared<KwlNode>(key, value);
         m_attributes.insert(std::make_pair(key, attribute));
      }
   }
   bool KwlNode::checkIfAttribute(const ossimString &value) const
   {
      return value.startsWith("@");
   }

   bool KwlNode::checkIfArray(const ossimString &value) const
   {
      return value.startsWith("[A-Z|a-z]") &&
             value.endsWith("[0-9]");
   }

   void KwlNode::extractKeyAndIndex(const ossimString &value, ossimString &key, ossimString &idx) const
   {
      idx = value.fromRegExp("[0-9]*$");
      key = value.beforeRegExp("[0-9]*$");
   }
   void KwlNode::loadPath(const ossimString &key, const ossimString &value)
   {
      std::vector<ossimString> splitArray;

      key.split(splitArray, ".");
      loadPath(splitArray, value);
   }

} // namespace ossim
