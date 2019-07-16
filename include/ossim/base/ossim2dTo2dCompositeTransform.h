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
    * Will iterate through the list of transforms from index 0 ... n -1 calling
    * the forward transform.  The output of the previous is used as the input to the next
    * transform.
    */
   virtual void forward(const ossimDpt& input,
                        ossimDpt& output) const;


   /**
    * will call the forward method with 2 arguments
    */
   virtual void forward(ossimDpt&  /* modify_this */) const;
   
   /**
    * Will iterate through the transformation list in reverse order from
    * n-1 ... 0 calling the inverse for each one.  The output is passed in as input
    * to the next transform.
    */
   virtual void inverse(const ossimDpt& input,
                        ossimDpt&       output) const;
   
   /**
    * will call the inverse method with 2 arguments
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
   
   /**
    * Saves the state of the composite by iterating through all transformas and saving
    * them to the keyword list with it's own prefix.
    * 
    * values will be:
    * <prefix>.object0
    * <prefix>.object1
    * <prefix>.object<n-1>
    * 
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;

   /**
    * Will laod the state of the object.  Note, it will clear the list and then add
    * all prefix found from the save state.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

protected:
   /**
    * Holds the list of transforms.
    */
   CompositeTransformList m_compositeTransforms;

   TYPE_DATA
};
#endif
