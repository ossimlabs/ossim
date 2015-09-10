//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//
//*******************************************************************
#ifndef ossimGoogleProjection_HEADER
#define ossimGoogleProjection_HEADER 1
#include <ossim/projection/ossimMapProjection.h>

class OSSIM_DLL ossimGoogleProjection : public ossimMapProjection
{
public:
   ossimGoogleProjection(const ossimEllipsoid& ellipsoid = ossimEllipsoid(),
                        const ossimGpt& origin = ossimGpt());
   ossimGoogleProjection(const ossimGoogleProjection&);
   ~ossimGoogleProjection();
   virtual ossimObject *dup()const{return new ossimGoogleProjection(*this);}

   virtual ossimDpt forward(const ossimGpt &worldPoint)    const;
   virtual ossimGpt inverse(const ossimDpt &projectedPoint)const;
   virtual void update();

	virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
	virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
   virtual bool operator==(const ossimProjection& projection) const;

protected:

	TYPE_DATA;
};
#endif
