#include <ossim/base/ossimKMeansClustering.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimException.h>
#include <fstream>

#ifdef WIN32
#define MY_POPEN(arg1, arg2) _popen(arg1, arg2)
#else
#define MY_POPEN(arg1, arg2) popen(arg1, arg2)
#endif

using namespace std;

void usage(char* appName)
{
   cout << "\nUsage: "<<appName<<" <samples-file>\n"<<endl;
}

int main(int argc, char *argv[])
{
   ostringstream xmsg;
   int returnCode = 0;
   
   if ( argc < 2 )
   {
      usage(argv[0]);
      exit(0);
   }

   // Load data:
   ossimFilename histoFile (argv[1]);
   ossimString label (histoFile.before("-"));
   ossimRefPtr<ossimMultiResLevelHistogram> mrlh = new ossimMultiResLevelHistogram;
   bool result = mrlh->importHistogram(histoFile);
   if (!result)
   {
      cout << "Could not open datafile at <"<<argv[1]<<">. Aborting."<<endl;
      exit (-1);
   }

   ossimRefPtr<ossimHistogram> band_histo = mrlh->getHistogram(0);
   if (!band_histo.valid())
   {
      xmsg<<"ossimKMeansFilter:"<<__LINE__<<"  Null band histogram returned!";
      throw ossimException(xmsg.str());
   }

   ossimRefPtr<ossimKMeansClustering> classifier = new ossimKMeansClustering;
   classifier->setNumClusters(2);
   const float* x = band_histo->GetVals();
   const ossim_int64* y = band_histo->GetCounts();
   ossim_uint32 N = (ossim_uint32) band_histo->GetRes();
   classifier->setSamples(x, N);
   classifier->setPopulations(y, N);
   if (!classifier->computeKmeans())
   {
      cout<<"ossimKMeansFilter:"<<__LINE__<<" No K-means clustering data available."<<endl;
      exit(1);
   }

   double A = classifier->getMean(0);
   double a = classifier->getSigma(0);
   double B = classifier->getMean(1);
   double b = classifier->getSigma(1);

   double T1 = classifier->getMaxValue(0);
   double T2 = (b*A + a*B)/(a+b);
   double T3 = (b*b*A + a*a*B)/(a*a+b*b);

   ostringstream xtics;
   xtics << "0.2, 0.8, "
         << "\"-a\" "<<A-a<<", "
         << "\"A\" "<<A<<", "
         << "\"a\" "<<A+a<<", "
         << "\"\" 0.5, "
         << "\"-b\" "<<B-b<<", "
         << "\"B\" "<<B<<", "
         << "\"b\" "<<B+b;

   // Create temporary data file stream:
   FILE * gnuplotPipe = MY_POPEN ("gnuplot -persistent", "w");
   //FILE * gnuplotPipe = fopen("gnuplot.dat", "w");
   if (!gnuplotPipe)
   {
      cout << "Could not create temporary gnuplot pipe. "<< endl;
      exit(1);
   }

   ostringstream xticsCmd;
   xticsCmd<<"set xtics ("<<xtics.str()<<")";
   fprintf(gnuplotPipe, "%s \n",xticsCmd.str().c_str());
   fprintf(gnuplotPipe, "set xrange [0.2:0.8]\n");

   fprintf(gnuplotPipe, "set style arrow 1 lt 1 lc rgb \"blue\" lw 1 \n");
   fprintf(gnuplotPipe, "set style arrow 2 lt 1 lc rgb \"green\" lw 1 \n");
   fprintf(gnuplotPipe, "set style arrow 3 lt 1 lc rgb \"orange\" lw 1 \n");

   fprintf(gnuplotPipe, "set arrow arrowstyle 1 from %f,graph(0.3, 0.3) to %f,graph(0,0)\n", T1, T1);
   fprintf(gnuplotPipe, "set label \"T1\" at %f, graph 0.33 center\n", T1);

   fprintf(gnuplotPipe, "set arrow arrowstyle 2 from %f,graph(0.4, 0.4) to %f,graph(0,0)\n", T2, T2);
   fprintf(gnuplotPipe, "set label \"T2\" at %f, graph 0.43 center\n", T2);

   fprintf(gnuplotPipe, "set arrow arrowstyle 3 from %f,graph(0.5, 0.5) to %f,graph(0,0)\n", T3, T3);
   fprintf(gnuplotPipe, "set label \"T3\" at %f, graph 0.53 center\n", T3);

   fprintf(gnuplotPipe, "set label \"%s\" at graph 0.15, graph 0.9 right font \",16\"\n", label.chars());

   // Output histogram data to temporary x, y file:
   ossimString command ("plot '-' with dots");
   fprintf(gnuplotPipe, "%s \n", command.chars());

   for (int i=0; i<N; ++i)
   {
      fprintf(gnuplotPipe, "%f %f \n", x[i], y[i]);
   }
   fprintf(gnuplotPipe, "e\n");

   fclose(gnuplotPipe);

   return 0;
}
