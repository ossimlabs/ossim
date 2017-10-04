#include <iostream>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimReferenced.h>

using namespace std;

class TestClass : public ossimReferenced
{
public:
   TestClass() {}
};

int main()
{
   cout << "Hello World!" << endl; // prints Hello World!

   ossimRefPtr<TestClass> p0;
   ossimRefPtr<TestClass> p1 = new TestClass;
   ossimRefPtr<TestClass> p1c = p1;
   ossimRefPtr<TestClass> p2 = new TestClass;

   if (p0)
      cout<<"(p0) TRUE"<<endl;
   else
      cout<<"(p0) FALSE"<<endl;

   if (!p0)
      cout<<"(!p0) TRUE"<<endl;
   else
      cout<<"(!p0) FALSE"<<endl;

   if (p1)
      cout<<"(p1) TRUE"<<endl;
   else
      cout<<"(p1) FALSE"<<endl;

   if (!p1)
      cout<<"(!p1) TRUE"<<endl;
   else
      cout<<"(!p1) FALSE"<<endl;

   if (p1 == p2)
      cout<<"(p1 == p2) TRUE"<<endl;
   else
      cout<<"(p1 == p2) FALSE"<<endl;

   if (p1 != p2)
      cout<<"(p1 != p2) TRUE"<<endl;
   else
      cout<<"(p1 != p2) FALSE"<<endl;

   if (p1 == p1c)
      cout<<"(p1 == p1c) TRUE"<<endl;
   else
      cout<<"(p1 == p1c) FALSE"<<endl;

   if (p1 != p1c)
      cout<<"(p1 != p1c) TRUE"<<endl;
   else
      cout<<"(p1 != p1c) FALSE"<<endl;

   return 0;
}
