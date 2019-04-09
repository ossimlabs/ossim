//---
//
// License: MIT
//
// File: ossim-foo.cpp
//
// Description: Contains application definition "ossim-foo" app.
//
// NOTE:  This is supplied for simple quick test. DO NOT checkin your test to
//        the svn repository.  Simply edit ossim-foo.cpp and run your test.
//        After completion you can do a "git checkout -- ossimfoo.cpp" if
//        you want to keep your working repository up to snuff.
//
// $Id$
//---

// System includes:
#include <cmath>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

// Put your includes here:

using namespace std;

int main(int argc, char *argv[])
{
   struct U
   {
      union
      {
         uint64_t u64;
         int32_t u32[2];
         float f32[2];
         char valueChars[8];
      };
   };

   stringstream iostr;
   U u1;
   U u2;
   u1.f32[0] = 0.1;
   u1.f32[1] = 0.2;
   //iostr<<hex<<setfill('0') << setw(16) <<u1.u64;
   //iostr>>hex>>setw(16)>>u2.u64;
   iostr.write(reinterpret_cast<const char*>(&u1.f32[0]), sizeof(float));
   iostr.write(reinterpret_cast<const char*>(&u1.f32[1]), sizeof(float));

   float f1, f2;
   iostr.read(reinterpret_cast<char*>(&u2.u64), sizeof(uint64_t));

   int32_t i1 = ((u2.u64>>32) & 0xffffffff);
   int32_t i2 = (u2.u64 & 0xffffffff);

   memcpy(&f1, &i1, sizeof(float));
   memcpy(&f2, &i2, sizeof(float));

   cout<<hex<<setfill('0') << setw(16) <<u2.u64<<endl;
   //cout<<"u32[0] = "<<u2.u32[0]<<endl;
   //cout<<"u32[1] = "<<u2.u32[1]<<endl;
   //cout<<"f32[0] = "<<u2.f32[0]<<endl;
   //cout<<"f32[1] = "<<u2.f32[1]<<endl;
   cout<<"i1 = "<<i1<<endl;
   cout<<"i2 = "<<i2<<endl;
   cout<<"f1 = "<<f1<<endl;
   cout<<"f2 = "<<f2<<endl;

   return 0;
}
