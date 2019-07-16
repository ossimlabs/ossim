//*******************************************************************
//
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:  Contains class definition for ossim2dTo2dCompositeTransform.
// 
//*******************************************************************
//  $Id$
#ifndef ossim2dTo2dCompositeTransform_HEADER
#define ossim2dTo2dCompositeTransform_HEADER
#include <ossim/base/ossim2dTo2dTransform.h>
#include <ossim/base/ossimRefPtr.h>
#include <vector>

/**
 * This is the identity transform and just passes the input to the output.
 */
class OSSIM_DLL ossim2dTo2dCompositeTransform : public ossim2dTo2dTransform
{
public:
   typedef std::vector<ossimRefPtr<ossim2dTo2dTransform> > CompositeTransformList;
   ossim2dTo2dCompositeTransform(const CompositeTransformList& compositeTransformList = CompositeTransformList());
   ossim2dTo2dCompositeTransform(const ossim2dTo2dCompositeTransform& rhs);
   virtual ~ossim2dTo2dCompositeTransform();
   virtual ossimObject* dup()const;


   void add(ossimRefPtr<ossim2dTo2dTransform> transform);
   void clear();

   const CompositeTransformList& getCompositeTransformList()const;

   /**
    * 
    */
   virtual void forward(const ossimDpt& input,
                        ossimDpt& output) const;

   
   /**
    * 
    */
   virtual void forward(ossimDpt&  /* modify_this */) const;
   
   /**
    * inverse transform just passes the point to the output.
    */
   virtual void inverse(const ossimDpt& input,
                        ossimDpt&       output) const;
   
   /**
    * inverse transform nothing is modified on the input point.
    */
   virtual void inverse(ossimDpt&  /* modify_this */) const;
   
   /**
    * Pass equality to the parent
    */
   virtual const ossim2dTo2dCompositeTransform& operator=(
                                                 const ossim2dTo2dCompositeTransform& rhs)
   {
      ossim2dTo2dTransform::operator =(rhs);
      
      return *this;
   }
   
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

protected:
   CompositeTransformList m_compositeTransforms;

   TYPE_DATA
};
#endif
