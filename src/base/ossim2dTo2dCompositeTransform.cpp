#include <ossim/base/ossim2dTo2dCompositeTransform.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossim2dTo2dTransformRegistry.h>

RTTI_DEF1_INST(ossim2dTo2dCompositeTransform, "ossim2dTo2dCompositeTransform", ossim2dTo2dTransform);

ossim2dTo2dCompositeTransform::ossim2dTo2dCompositeTransform(const CompositeTransformList& compositeTransformList )
: m_compositeTransforms(compositeTransformList)
{

}

ossim2dTo2dCompositeTransform::ossim2dTo2dCompositeTransform(const ossim2dTo2dCompositeTransform& rhs)
:m_compositeTransforms(rhs.m_compositeTransforms)
{

}

ossim2dTo2dCompositeTransform::~ossim2dTo2dCompositeTransform()
{
   clear();
}

ossimObject* ossim2dTo2dCompositeTransform::dup()const
{
   return new ossim2dTo2dCompositeTransform(*this);
}


void ossim2dTo2dCompositeTransform::add(ossimRefPtr<ossim2dTo2dTransform> transform)
{
   if(transform)
   {
      m_compositeTransforms.push_back(transform);
   }
}

void ossim2dTo2dCompositeTransform::clear()
{
   m_compositeTransforms.clear();
}

const ossim2dTo2dCompositeTransform::CompositeTransformList& ossim2dTo2dCompositeTransform::getCompositeTransformList()const
{
   return m_compositeTransforms;
}

void ossim2dTo2dCompositeTransform::forward(const ossimDpt& input,
                                            ossimDpt& output) const
{
   ossimDpt tempInput = input;
   output             = input;

   for(auto transform:m_compositeTransforms)
   {
      transform->forward(tempInput, output);
      tempInput=output;
   }
}

/**
 * 
 */
void ossim2dTo2dCompositeTransform::forward(ossimDpt&  modify_this) const
{
   forward(ossimDpt(modify_this), modify_this);
}

/**
 * inverse transform just passes the point to the output.
 */
void ossim2dTo2dCompositeTransform::inverse(const ossimDpt& input,
                                            ossimDpt&       output) const
{
   ossimDpt tempInput = input;
   output             = input;

   for(CompositeTransformList::const_reverse_iterator iter = m_compositeTransforms.rbegin();
       iter != m_compositeTransforms.rend();
       ++iter
       )
   {
       (*iter)->inverse(tempInput, output);
       tempInput = output;     
   }
}

/**
 * inverse transform nothing is modified on the input point.
 */
void ossim2dTo2dCompositeTransform::inverse(ossimDpt&  modify_this) const
{
   inverse(ossimDpt(modify_this), modify_this);
}

bool ossim2dTo2dCompositeTransform::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   bool result = ossim2dTo2dTransform::saveState(kwl, prefix);
   ossim_uint64 idx = 0;

   for(; idx < m_compositeTransforms.size();++idx)
   {
      ossimString newPrefix(prefix);

      newPrefix += ossimString("object") +
                   ossimString::toString(idx) + 
                   ossimString(".");
      result &= m_compositeTransforms[idx]->saveState(kwl, newPrefix.c_str());
   }

   return result;
}

bool ossim2dTo2dCompositeTransform::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool result = ossim2dTo2dTransform::loadState(kwl, prefix);
   
   std::vector<ossimString> prefixValues;
   clear();
   kwl.getSubstringKeyList(prefixValues, ossimString("^(")+ossimString(prefix)+"object[0-9]+.)");

   for(auto newPrefix:prefixValues)
   {
      ossimRefPtr<ossim2dTo2dTransform> trans = ossim2dTo2dTransformRegistry::instance()->createTransform(kwl, newPrefix.c_str());
      if(trans)
      {
            add(trans);
      }
   }

   return result;
}
