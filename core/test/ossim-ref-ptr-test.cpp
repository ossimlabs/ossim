//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Test application for ossimRefPtr class.
// 
//----------------------------------------------------------------------------
// $Id: ossim-ref-ptr-test.cpp 19980 2011-08-17 20:14:46Z dburken $

#include <iostream>
using namespace std;

#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/init/ossimInit.h>

// Referenced object.
class Obj : public ossimReferenced
{
public:
   Obj(){}
   virtual ~Obj(){ cout << "~Obj..." << endl; }
};

// Owns a ref ptr.
class Foo
{
public:
   Foo() : m_refPtr(new Obj) {}
   ~Foo() { m_refPtr = 0; }
   ossimRefPtr<Obj>& getRefPtr() { return m_refPtr; }

private:
   ossimRefPtr<Obj> m_refPtr;
};

static void assignRefPtr(ossimRefPtr<Obj>& rp)
{
   rp = new Obj;
}

static ossimRefPtr<Obj> getRefPtr()
{
   return ossimRefPtr<Obj>(new Obj);
}

// Test app
int main( /* int argc, char* argv[] */ )
{
   ossimInit::instance()->initialize();

   Foo* f = new Foo;

   cout << "count(1): " << f->getRefPtr()->referenceCount() << endl;

   ossimRefPtr<Obj> rp = f->getRefPtr();

   cout << "count(2): " << f->getRefPtr()->referenceCount() << endl;

   delete f;  
   f = 0;

   cout << "count(1): " << rp->referenceCount() << endl;

   rp = 0; // Should destroy here...

   rp = 0;
   assignRefPtr(rp);

   if ( rp.valid() )
   {
      cout << "assignRefPtr rp count(1): " << rp->referenceCount() << endl;
   }

   rp = 0;

   rp = getRefPtr();

   if ( rp.valid() )
   {
      cout << "getRefPtr rp count(1): " << rp->referenceCount() << endl;
   }
   
   
   return 0;
}
