#include <ossim/projection/ossimEquDistCylProjection.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   ossimGpt gpt_ul (38.0, -77.0);
   ossimDpt ipt_ul (0.0, 0.0);
   ossimDpt ipt_lr (255.0, 255.0);
   ossimGpt gpt;
   ossimDpt gsd;

   ossimEquDistCylProjection proj;

   proj.setUlTiePoints(gpt_ul);
   proj.setDecimalDegreesPerPixel(ossimDpt(0.0000090,0.0000090));

   cout << "\n Normal initialization ***************"<<endl;
   cout << "ORIG: "<<proj.getOrigin()<<endl;
   cout << "TP: "<<proj.getUlGpt()<<endl;
   cout << "GSD: "<<proj.getMetersPerPixel() << endl;
   proj.lineSampleHeightToWorld(ipt_ul, 0.0, gpt);
   cout << "UL: " <<gpt << endl;
   proj.lineSampleHeightToWorld(ipt_lr, 0.0, gpt);
   cout << "LR: " <<gpt << endl;

   proj.setOrigin(gpt_ul);

   cout << "\n Origin set to UL ***************"<<endl;
   cout << "ORIG: "<<proj.getOrigin()<<endl;
   cout << "TP: "<<proj.getUlGpt()<<endl;
   cout << "GSD: "<<proj.getMetersPerPixel() << endl;
   proj.lineSampleHeightToWorld(ipt_ul, 0.0, gpt);
   cout << "UL: " <<gpt << endl;
   proj.lineSampleHeightToWorld(ipt_lr, 0.0, gpt);
   cout << "LR: " <<gpt << endl;

   proj.setUlTiePoints(gpt_ul);

   cout << "\n Tiepoint reset to UL ***************"<<endl;
   cout << "ORIG: "<<proj.getOrigin()<<endl;
   cout << "TP: "<<proj.getUlGpt()<<endl;
   cout << "GSD: "<<proj.getMetersPerPixel() << endl;
   proj.lineSampleHeightToWorld(ipt_ul, 0.0, gpt);
   cout << "UL: " <<gpt << endl;
   proj.lineSampleHeightToWorld(ipt_lr, 0.0, gpt);
   cout << "LR: " <<gpt << endl;

   return 0;
}
