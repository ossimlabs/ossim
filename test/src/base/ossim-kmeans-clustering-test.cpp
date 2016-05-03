#include <ossim/base/ossimKMeansClustering.h>
#include <ossim/base/ossimRefPtr.h>
#include <fstream>

using namespace std;

void usage(char* appName)
{
   cout << "\nUsage: "<<appName<<" <samples-file>\n"<<endl;
}

int main(int argc, char *argv[])
{
   int returnCode = 0;
   
   if ( argc < 2 )
   {
      usage(argv[0]);
      exit(0);
   }

   // Load data:
   ifstream datafile (argv[1]);
   if (datafile.fail())
   {
      cout << "Could not open datafile at <"<<argv[1]<<">. Aborting."<<endl;
      exit (-1);
   }

   double* vals = new double [256];
   double* pops = new double [256];
   for (size_t i=0; i<256; i++)
   {
      datafile >> vals[i] >> pops[i];
   }

   ossimRefPtr<ossimKMeansClustering> kmeans = new ossimKMeansClustering;
   kmeans->setSamples(vals, 256);
   kmeans->setPopulations(pops, 256);
   kmeans->setNumClusters(2);
   kmeans->setVerbose();
   kmeans->computeKmeans();

   return 0;
}
