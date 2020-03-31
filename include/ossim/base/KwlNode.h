#ifndef ossimKwlNode_HEADER
#define ossimKwlNode_HEADER
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimKeywordlist.h>
#include <map>
#include <memory>
namespace ossim
{
   class KwlNode
   {
   public:
      typedef std::map<ossimString, std::shared_ptr<KwlNode> > ChildMap;

      KwlNode(const ossimString &key = ossimString(),
            const ossimString &value = ossimString())
         : m_key(key),
            m_value(value),
            m_isArray(false)
      {
      }

      void setKey(const ossimString &key)
      {
         m_key = key;
      }
      void setValue(const ossimString &value)
      {
         m_value = value;
      }
      void setArrayFlag(bool flag)
      {
         m_isArray = flag;
      }
      const ossimString getKey() const
      {
         return m_key;
      }
      const ossimString getValue() const
      {
         return m_value;
      }

      const ChildMap &getChildren() const
      {
         return m_children;
      }
      ChildMap &getChildren()
      {
         return m_children;
      }
      const ChildMap &getAttributes() const
      {
         return m_attributes;
      }
      ChildMap &getAttributes()
      {
         return m_attributes;
      }
      bool hasChildren() const
      {
         return !m_children.empty();
      }
      bool hasAttributes() const
      {
         return !m_attributes.empty();
      }
      bool getIsArray() const
      {
         return m_isArray;
      }
      std::shared_ptr<KwlNode> findChild(const ossimString &key)
      {
         return findChild(m_children, key);
      }
      std::shared_ptr<const KwlNode> findChild(const ossimString &key) const
      {
         return findChild(m_children, key);
      }

      std::shared_ptr<KwlNode> findAttribute(const ossimString &key)
      {
         return findChild(m_attributes, key);
      }

      std::shared_ptr<const KwlNode> findAttribute(const ossimString &key) const
      {
         return findChild(m_attributes, key);
      }
      static std::shared_ptr<KwlNode> createTree(const ossimKeywordlist &kwl, 
                                                 const ossimString &rootTag = ossimString(""));

   protected:
      ossimString m_key;
      ossimString m_value;

      ChildMap m_children;
      ChildMap m_attributes;
      bool m_isArray;

      static std::shared_ptr<KwlNode> findChild(ChildMap &childMap,
                                                const ossimString &key);

      static std::shared_ptr<const KwlNode> findChild(const ChildMap &childMap,
                                                      const ossimString &key);
      bool checkIfAttribute(const ossimString &value)const;
      bool checkIfArray(const ossimString &value)const;
      void extractKeyAndIndex(const ossimString &value, 
                              ossimString &key, 
                              ossimString &idx)const;
      void loadPath(std::vector<ossimString> &q, const ossimString &value);
      void addOrSetAttribute(const ossimString &key, const ossimString &value);
      void loadPath(const ossimString &key, const ossimString &value);
   };
} // namespace ossim

#endif