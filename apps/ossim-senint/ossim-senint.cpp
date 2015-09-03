//----------------------------------------------------------------------------
//
// License: See top level LICENSE.txt file.
//
// Author: Dave Hicks
//
// Description:
//
// Application to do multi-image (sensor model) intersection or single-image/
// earth (ellipsoid, DEM) intersection.  Also outputs estimated CE/LE
// and error ellipse parameters at the specified probability level for the
// intersected point based covariance propagation of support data error.
// Propagated CE/LE available only if ossimSensorModelTuple::intersect returns
// a (status!=ERROR_PROP_FAIL).  This status is only possible if
// ossimSensorModelTuple is able to obtain full covariance info from the
// sensor model(s) being used.
//
// Demonstrates use of ossimSensorModelTuple for single and stereo.  Also
// demonstrates several uses of ossimPositionQualityEvaluator class.
//
// One or two images may be used.  The mensuration files must have the same
// number of points for both images, in the same order.
//
//----------------------------------------------------------------------------
// $Id: senint.cpp 13025 2008-06-13 17:06:30Z sbortman $

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <cstdlib>



#include <ossim/projection/ossimSensorModel.h>
#include <ossim/projection/ossimSensorModelFactory.h>
#include <ossim/projection/ossimSensorModelTuple.h>
#include <ossim/projection/ossimRpcModel.h>
#include <ossim/projection/ossimPositionQualityEvaluator.h>
#include <ossim/elevation/ossimHgtRef.h>
#include <ossim/base/ossimColumnVector3d.h>

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimFilename.h>


static ossimTrace traceExec("senint:exec");
static ossimTrace traceDebug("senint:debug");


//**************************************************************************
// usage()
//**************************************************************************
void usage()
{
   std::cout << " example:\n\n"
        << "    senint -pt rpc -i1 image1.geom [-i2 image2.geom] [-surf surf.dat]\n"
        << "      For pt=std, performs multi & single-image intersections\n"
        << "      for all image points found in image1.men/image2.men files.\n"
        << "      Second image argument is optional.  Defaults to pt=std.\n"
        << "\n"
        << "      For pt=rpc, exercises optional positionQualityEvaluator\n"
        << "      constructors/extractors.  Only valid for RPC models.\n"
        << "\n"
        << "      The -surf option allows setting of DEM/slope accuracy and\n"
        << "      orientation.  The file format is:\n"
        << "         CE90 LE90      [DEM CE/LE]\n"
        << "            x    y    z [surface normal unit vector]\n"
        << "         varx vary varz [diagonal elements of covariance matrix]\n"
        << "\n"
        << "      The -losoverride option allows setting of LOS direction\n"
        << "      and accuracy.  The file format is:\n"
        << "         elev azim  [target elevation, azimuth]\n"
        << "         bias rand  [LOS 1-sigma bias, random components]\n"
        << "\n"
        << "      The -plevel option allows setting of probability level\n"
        << "      for output CE/LE and ellipse.  Choices are:\n"
        << "         0 - 1-sigma\n"
        << "         1 - .5P\n"
        << "         2 - .9P    Default\n"
        << "         3 - .95P\n"
        << "\n"
        << std::endl;
}


int main(int argc, char *argv[])
{
   const char* MODULE = "senint";
   ossimString line(100,'~');
   ossimString sp26(26,' ');
   ossimString sp20(20,' ');
   
   typedef enum
   {
      STD,
      RPC
   } procType_t;

   char timeString[22];
   time_t now = time(NULL);
   strftime(timeString, 21, "%Y-%m-%d %H:%M:%S", localtime(&now));
   std::cout << "\n" << line <<std::endl;
   std::cout << " senint Geopositioning Report"<<std::endl;
   std::cout << line << std::endl;
   std::cout << " "<<string(timeString)<<std::endl;

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Perform OSSIM initialization
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimArgumentParser argumentParser(&argc, argv);
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);

   argumentParser.getApplicationUsage()->
      setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->
      setDescription(argumentParser.getApplicationName()+
                     " Performs single/multi-image intersection");
   argumentParser.getApplicationUsage()->
      setCommandLineUsage(argumentParser.getApplicationName()+
                          "-pt processing type -i1 input geometry file");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-h", "Display this information");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-i1", "REQUIRED: input geometry file for image 1");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-i2", "OPTIONAL: input geometry file for image 2");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-pt", "OPTIONAL: processing type (std or rpc)");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-surf", "OPTIONAL: filename of surface parameters");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-losoverride", "OPTIONAL: filename of LOS parameters");
   argumentParser.getApplicationUsage()->
      addCommandLineOption("-plevel", "OPTIONAL: probability level (0-3)");


   if (traceDebug())
      ossimNotify(ossimNotifyLevel_DEBUG)
      << "DEBUG senint: entering..." << std::endl;


   if(argumentParser.read("-h") ||
      argumentParser.read("--help")||
      argumentParser.argc() <2)
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      usage();
      return 0;
   }


   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Parse the input arguments
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // processing type
   ossimString senintARGS0;
   while(argumentParser.read("-pt", stringParam))
   {
      senintARGS0 = tempString;
   }
   if (senintARGS0.size() == 0)
   {
      senintARGS0 = "std";
   }
   procType_t procType;
   if (senintARGS0=="std")
      procType = STD;
   else if (senintARGS0=="rpc")
      procType = RPC;
   else
   {
      std::cout << " Invalid processing type..."<<std::endl;
      return 0;
   }   

   // image 1
   ossimString senintARGS1;
   while(argumentParser.read("-i1", stringParam))
   {
      senintARGS1 = tempString;
   }

   // image 2
   ossimString senintARGS2;
   while(argumentParser.read("-i2", stringParam))
   {
      senintARGS2 = tempString;
   }

   // surface parameters
   ossimString senintARGS3;
   while(argumentParser.read("-surf", stringParam))
   {
      senintARGS3 = tempString;
   }

   // LOS parameters
   ossimString senintARGS4;
   while(argumentParser.read("-losoverride", stringParam))
   {
      senintARGS4 = tempString;
   }

   // probability level
   ossimString senintARGS5;
   while(argumentParser.read("-plevel", stringParam))
   {
      senintARGS5 = tempString;
   }

 
   argumentParser.reportRemainingOptionsAsUnrecognized();
   if (argumentParser.errors())
   {
      argumentParser.writeErrorMessages(std::cout);
      exit(1);
   }

   std::cout << "      " << MODULE << " arguments...\n"
             << sp20 << "         -pt: "<< senintARGS0 << "\n"
             << sp20 << "         -i1: "<< senintARGS1 << "\n"
             << sp20 << "         -i2: "<< senintARGS2 << "\n"
             << sp20 << "       -surf: "<< senintARGS3 << "\n"
             << sp20 << "-losoverride: "<< senintARGS4 << "\n"
             << sp20 << "     -plevel: "<< senintARGS5
             << std::endl;
   
  
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //       Load the model(s)
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   int numImages = 0;
   ossimFilename inputFile[2];
   ossimRefPtr<ossimProjection> model[2];
   
   // Load 1st image
   inputFile[0] = senintARGS1;
   model[0] = ossimSensorModelFactory::instance()->createProjection(inputFile[0], 0);
   if (!dynamic_cast<ossimSensorModel*>(model[0].get())) 
   {
      std::cout << " createProjection for image 1 failed..."<<std::endl;
      return 0;
   }
   else
   {
      ++numImages;
   }

   // Load 2nd image if present
   if (senintARGS2.size()>0)
   {
      inputFile[1] = senintARGS2;
      model[1] = PTR_CAST(ossimSensorModel,
         ossimSensorModelFactory::instance()->createProjection(inputFile[1],0));

      if (model[1] == NULL)
      {
         std::cout << " loadState 2 Failed"<<std::endl;
         return 0;
      }
      else
      {
         ++numImages;
      }
   }

   // Load the models into the tuple
   ossimSensorModelTuple iSet;
   for (int i=0; i<numImages; ++i)
   {
      ossimSensorModel* m = dynamic_cast<ossimSensorModel*>(model[i].get());
      if(m)
      {
         iSet.addImage(m);
      }
   }
   
  
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //       Load the surface parameters
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ossim_float64 surfCE90 = 0.0;
   ossim_float64 surfLE90 = 0.0;
   ossimColumnVector3d surfNormal(0,0,1);
   NEWMAT::Matrix surfCovMat(3,3);
   surfCovMat = 0.0;
   if (senintARGS3.size()>0)
   {
      ossimString surfFileName = senintARGS3;
      ifstream surfFile(surfFileName.data(), ios::in);
      if (surfFile.good() == 0)
      {
         std::cout << "\n "<< surfFileName << " load failed..." << std::endl;
         return 0;
      }
      surfFile >> surfCE90 >> surfLE90;
      surfFile >> surfNormal[0] >> surfNormal[1] >> surfNormal[2];
      surfFile >> surfCovMat(1,1) >> surfCovMat(2,2) >> surfCovMat(3,3);
      std::cout << "\n      "<< surfFileName << " dump...\n"
                << sp20<<surfCE90 <<" "<< surfLE90 << "\n"
                << sp20<<surfNormal << "\n"
                << sp20<<surfCovMat(1,1)<<" "<<surfCovMat(2,2)<<" "<<surfCovMat(3,3)
                << std::endl;
   }
   
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Set the DEM accuracy
   //  Note: this only affects the covariance matrix output from DEM
   //  intersection - ossimSensorModelTuple::(img,obs[img][pt],intECF,covMat)
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   iSet.setIntersectionSurfaceAccuracy(surfCE90, surfLE90);
   
  
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //       Load the LOS parameters
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ossim_float64 overTel;
   ossim_float64 overTaz;
   ossim_float64 overBias;
   ossim_float64 overRand;
   bool LOSoverride = false;
   if (senintARGS4.size()>0)
   {
      ossimString overFileName = senintARGS4;
      ifstream overFile(overFileName.data(), ios::in);
      if (overFile.good() == 0)
      {
         std::cout << "\n "<< overFileName << " load failed..." << std::endl;
         return 0;
      }
      overFile >> overTel >> overTaz;
      overFile >> overBias >> overRand;
      LOSoverride = true;
      std::cout << "\n      "<< overFileName << " dump...\n"
                << sp20<<overTel <<" "<< overTaz << "\n"
                << sp20<<overBias <<" "<< overRand
                << std::endl;
   }
   
  
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //       Load the probability level
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   pqeProbLev_t pLev = P90;
   ossimString pLevC(" 0.9P");
   if (senintARGS5.size()>0)
   {
      ossim_int32 lev = senintARGS5.toInt32();
      switch (lev)
      {
         case ONE_SIGMA:
            pLev = ONE_SIGMA;
            pLevC = " 1-Sigma";           
            break;
         case P50:
            pLev = P50;
            pLevC = " 0.5P";           
            break;
         case P90:
            pLev = P90;
            pLevC = " 0.9P";           
            break;
         case P95:
            pLev = P95;
            pLevC = " 0.95P";           
            break;
         default:
            break;
      }
   }
   std::cout << "\n      Probability level: "<< pLevC << std::endl;
 

   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   //       Load image mensuration info
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   std::vector<ossimString> chkIds;
   vector<DptSet_t> obs;
   vector<ossimString> imageIDs;
   int numPts = 0;
   char* tmpBuf = new char[256];
   
   for (int im=0; im<numImages; ++im)
   {
      int numMeas;
      ossimString measFile = inputFile[im];
      measFile.replace(measFile.find("geom"), 4, "men");
      ifstream imgFile(measFile.data(), ios::in);
      if (imgFile.good() == 0)
      {
         std::cout << "\n "<< measFile << " load failed..." << std::endl;
         return 0;
      }
      
      imageIDs.push_back(measFile.erase(measFile.find(".men"),4));
      imgFile >> numMeas;
      DptSet_t meas;
      numPts = numMeas;
      for (int j=0; j<numPts; j++)
      {
         ossimString id;
         ossim_float64 u,v;
         ossimDpt uv;
         imgFile >> tmpBuf;
         id = tmpBuf;
         chkIds.push_back(id);
         imgFile >> u >> v;
         uv.u = u; uv.v = v;
         meas.push_back(uv);
      }
      imgFile.close();
      obs.push_back(meas);
   }

   delete [] tmpBuf;
   tmpBuf = 0;

   ossimSensorModelTuple::IntersectStatus opOK;

   ossim_float64 atHgt;
   ossimEcefPoint intECF;
   ossim_float64 CEpLev;
   ossim_float64 LEpLev;
   NEWMAT::Matrix covMat(3,3);
   pqeErrorEllipse ell;
      
   
   
   //=======================================================================
   //    procType switch
   //=======================================================================
   switch (procType)
   {
   
   
   case STD:
   {
   
      std::cout << "\n Standard processing mode...\n";
         
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // Process the measurements
   //   First, stereo if 2 images present, then individual processing
   //   of each image (DEM & ellipsoid intersection)
   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   for (int pt=0; pt<numPts; ++pt)
   {
      //~~~~~~~~
      // Stereo
      //~~~~~~~~
      if (numImages==2)
      {
         DptSet_t obst;
         obst.push_back(obs[0][pt]);
         obst.push_back(obs[1][pt]);
         
         opOK = iSet.intersect(obst, intECF, covMat);
         
         std::cout << "\n"<<line << "\n Point: " <<chkIds[pt]
                   <<"   ossimSensorModelTuple operations...";
         std::cout << "\n [stereo intersection]    ";
         std::cout << " projected: " << intECF.toString(3) << std::endl;
         ossimGpt intG(intECF);
         std::cout << sp26 << " geo:       " << intG.toString(8)<<std::endl;
         std::cout << sp26 << " geo(dmsf): ( " <<intG.toDmsString() << " ht: "
                   <<setprecision(4)<<intG.height()/MTRS_PER_FT<<" )"
                   <<std::endl;
         if (opOK != ossimSensorModelTuple::ERROR_PROP_FAIL)
         {
            ossimPositionQualityEvaluator qev(intECF, covMat);
            bool epOK = qev.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
               qev.extractErrorEllipse(pLev, ell);

            std::cout << sp26 <<pLevC<<" CE/LE: ( "<<setprecision(3)<<CEpLev<<" / "
                      << LEpLev << " ) [ft]"<<std::endl;
         }
         else
         {
            std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
         }
      }

      //~~~~~~
      // Mono
      //~~~~~~
      for (int img=0; img<numImages; ++img)
      {
         std::cout<< "\n >> Image: " << imageIDs[img] << "  mono..."<<std::endl;
                  
         //~~~~~~~~~~~~~~~~~~
         // DEM Intersection
         //~~~~~~~~~~~~~~~~~~        
         std::cout << " [DEM intersection]       ";
         
         opOK = iSet.intersect(img, obs[img][pt], intECF, covMat);
         
         std::cout << " projected: "<<intECF.toString(3)<<std::endl;
         ossimGpt intGD(intECF);
         std::cout << sp26 << " geo:       " << intGD.toString(8)
                   << std::endl;
         std::cout << sp26 << " geo(dmsf): ( " << intGD.toDmsString()
                   << " ht: " << setprecision(4)
                   << intGD.height()/MTRS_PER_FT<<" )" <<std::endl;

         if (opOK != ossimSensorModelTuple::ERROR_PROP_FAIL)
         {
            ossimPositionQualityEvaluator qev(intECF, covMat);
            bool epOK = qev.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
               qev.extractErrorEllipse(pLev, ell);

            std::cout << sp26 <<pLevC<<" CE/LE: ( "
                      <<setprecision(4)<<CEpLev<<" / "<<LEpLev
                      <<" ) [ft]"<<std::endl;
            std::cout << sp26 <<" Ellipse:   ( "
                      <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                      <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD<<" deg"<<std::endl;
               
         }
         else
         {
            std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
         }


         //~~~~~~~~~~~~~~~~~~~~~~~~
         // Ellipsoid Intersection
         //~~~~~~~~~~~~~~~~~~~~~~~~
         std::cout << "\n [ellipsoid intersection] ";
         atHgt = intGD.height();
         
         opOK = iSet.intersect(img, obs[img][pt], atHgt, intECF, covMat);

         std::cout << " projected: "<<intECF.toString(3)<<std::endl;
         ossimGpt intGE(intECF);
         std::cout << sp26 <<" geo:       "<<intGE.toString(8)<<std::endl;
         std::cout << sp26 <<" geo(dmsf): ( "<<intGE.toDmsString()
                   <<" ht: "<<setprecision(4)
                   <<intGE.height()/MTRS_PER_FT<<" )"<<std::endl;
         if (opOK != ossimSensorModelTuple::ERROR_PROP_FAIL)
         {
            ossimPositionQualityEvaluator qev(intECF, covMat);
            bool epOK = qev.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
               qev.extractErrorEllipse(pLev, ell);

            std::cout << sp26 <<pLevC<<" CE/LE: ( "
                      <<setprecision(4)<<CEpLev<<" / "<<LEpLev
                      <<" ) [ft]"<<std::endl;
            std::cout << sp26 <<" Ellipse:   ( "
                      <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                      <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD<<" deg"<<std::endl;
         }
         else
         {
            std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
         }
      }
   }
   
   }  //case STD
   break;
   
   
   
   case RPC:
   {
   
      std::cout << "\n RPC test mode...\n";
         
   for (int pt=0; pt<numPts; ++pt)
   {
      for (int img=0; img<numImages; ++img)
      {
         if (model[img]->getClassName().contains("Rpc"))
         {
         
         std::cout<< "\n >> Image: " << imageIDs[img] << "  mono..."<<std::endl;
         
         
         // Perform the standard intersection and error propagation
         std::cout << " [DEM intersection]       ";
         opOK = iSet.intersect(img, obs[img][pt], intECF, covMat);
         
         std::cout << " projected: "<<intECF.toString(3)<<std::endl;
         ossimGpt intGD(intECF);
         std::cout << sp26 << " geo:       " << intGD.toString(8)
                   << std::endl;
         std::cout << sp26 << " geo(dmsf): ( " << intGD.toDmsString()
                   << " ht: " << setprecision(4)
                   << intGD.height()/MTRS_PER_FT<<" )" <<std::endl;
            
            
         // Set the acquisition angles and LOS error components
         ossim_float64 tel;
         ossim_float64 taz;
         ossim_float64 biasError;
         ossim_float64 randError;
         ossimRpcModel* mdl = PTR_CAST(ossimRpcModel, model[img].get());

         if (LOSoverride)
         {
            std::cout << sp26 <<"   Overridden geometry.......... ";
            tel = overTel*RAD_PER_DEG;
            taz = overTaz*RAD_PER_DEG;
            biasError = overBias;
            randError = overRand;
         }
         else
         {
            // -------------acquisition geometry check code
            std::cout << sp26 <<"   Acquisition geometry check.......... ";
            ossimEcefPoint intECF1;
            ossimEcefPoint intECF2;
            NEWMAT::Matrix covMatChk(3,3);
            ossim_float64 vectorLength = 100.0;
            atHgt = intGD.height() - vectorLength/2.0;
            opOK = iSet.intersect(img, obs[img][pt], atHgt, intECF1, covMatChk);
            atHgt = intGD.height() + vectorLength/2.0;
            opOK = iSet.intersect(img, obs[img][pt], atHgt, intECF2, covMatChk);
            ossimGpt ptG(intECF1);
            ossimLsrSpace enu(ptG);
            ossimEcefVector v12(intECF1,intECF2);
            ossimLsrVector v1(v12,enu);
            cout<< setprecision(4);
            // cout<<" v1 "<<v1<<endl;
            // cout<<" v1 unit "<<v1.unitVector()<<endl;
            tel = asin(v1.unitVector().z());
            taz = atan2(v1.unitVector().x(),v1.unitVector().y());
            // -------------acquisition geometry check code
            biasError = mdl->getBiasError();
            randError = mdl->getRandError();
         }
         std::cout <<" tel/taz:   "<<tel*DEG_PER_RAD<<"/"
                                   <<taz*DEG_PER_RAD
                                   <<" [deg]"<<std::endl;
         std::cout << sp26 <<" bias/rand: "<<biasError<<"/"
                                           <<randError<<" [m]"<<std::endl;




         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         // Test ossimPositionQualityEvaluator constructors
         //    1 - standard (surface acc set by setIntersectionSurfaceAcc...)
         //    2 - error components, acquisition angles, def surface parms
         //    3 - error components, acquisition angles, ext surface parms
         //    4 - RPC error components & coefficients, def surface parms
         //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         if (opOK != ossimSensorModelTuple::ERROR_PROP_FAIL)
         {

            //===================
            // qev constructor 1
            //    Usage:   standard, covariance matrix provided
            //    Errors:  all errors included by the external covariance
            //             matrix provider are reflected
            //===================
            std::cout << sp20
               <<"Case 1:  standard, setIntersectionSurfaceAcc..."<<std::endl;
            ossimPositionQualityEvaluator qev1(intECF, covMat);
            bool epOK = qev1.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
            {
               qev1.extractErrorEllipse(pLev, ell);
               std::cout << sp26 <<pLevC<<" CE/LE: ( "
                         <<setprecision(4)<<CEpLev<<" / "<<LEpLev
                         <<" ) [ft]"<<std::endl;
               std::cout << sp26 <<" Ellipse:   ( "
                         <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                         <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD
                         <<" deg"<<std::endl;
            }
            else
            {
               std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
            }
           

            //===================
            // qev constructor 2
            //    Usage:   LOS errors & look angles provided
            //    Errors:  LOS errors are propagated thru acquisition
            //             angles; no surface geometry or uncertainty
            //             is reflected
            //===================
            std::cout << sp20
               <<"Case 2:  acquisition angles, def surface parms"<<std::endl;
            ossimPositionQualityEvaluator qev2
               (intECF, biasError, randError, tel, taz);
            epOK = qev2.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
            {
               qev2.extractErrorEllipse(pLev, ell);

               std::cout << sp26 <<pLevC<<" CE/LE: ( "
                         <<setprecision(4)<<CEpLev<<" / "<<LEpLev
                         <<" ) [ft]"<<std::endl;
               std::cout << sp26 <<" Ellipse:   ( "
                         <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                         <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD
                         <<" deg"<<std::endl;
            }
            else
            {
               std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
            }
            
            
            //====================
            // qev constructor 3
            //    Usage:   LOS errors, look angles, & surface info provided
            //    Errors:  LOS errors are propagated thru acquisition
            //             angles; surface geometry and uncertainty
            //             are reflected
            //====================
            std::cout << sp20
               <<"Case 3:  acquisition angles, ext surface parms"<<std::endl;
            ossimColumnVector3d sNormal;
            NEWMAT::Matrix sCovMat(3,3);
            sNormal = surfNormal;
            sCovMat = surfCovMat;
            std::cout << sp26 <<" surface normal:    "<<sNormal<<std::endl;
            std::cout << sp26 <<" surface variances: "
                              <<surfCovMat(1,1)<<" "
                              <<surfCovMat(2,2)<<" "
                              <<surfCovMat(3,3)
                              <<std::endl;
            ossimPositionQualityEvaluator qev3
              (intECF, biasError, randError, tel, taz, sNormal, sCovMat);
            epOK = qev3.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
            {
               qev3.extractErrorEllipse(pLev, ell);
               std::cout << sp26 <<pLevC<<" CE/LE: ( "
                         <<setprecision(2)<<CEpLev<<" / "<<LEpLev
                         <<" ) [ft]"<<std::endl;
               std::cout << sp26 <<" Ellipse:   ( "
                         <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                         <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD
                         <<" deg"<<std::endl;
            }
            else
            {
               std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
            }
            
            
            //====================
            // qev constructor 4
            //    Usage: LOS errors & RPC parameters provided
            //    Errors:  LOS errors are propagated thru acquisition
            //             angles computed from RPC coefficients; no
            //             surface geometry or uncertainty is reflected
            //====================
            std::cout << sp20
               <<"Case 4:  RPC parms, def surface parms"<<std::endl;
            pqeRPCModel rpc;
            ossimRpcModel::rpcModelStruct rpcPar;
            mdl->getRpcParameters(rpcPar);
            rpc.theLineScale  = rpcPar.lineScale;
            rpc.theSampScale  = rpcPar.sampScale;
            rpc.theLatScale   = rpcPar.latScale;
            rpc.theLonScale   = rpcPar.lonScale;
            rpc.theHgtScale   = rpcPar.hgtScale;
            rpc.theLineOffset = rpcPar.lineOffset;
            rpc.theSampOffset = rpcPar.sampOffset;
            rpc.theLatOffset  = rpcPar.latOffset;
            rpc.theLonOffset  = rpcPar.lonOffset;
            rpc.theHgtOffset  = rpcPar.hgtOffset;
            rpc.theType       = rpcPar.type;
            for (int i=0; i<20; i++)
            {
               rpc.theLineNumCoef[i] = rpcPar.lineNumCoef[i];
               rpc.theLineDenCoef[i] = rpcPar.lineDenCoef[i];
               rpc.theSampNumCoef[i] = rpcPar.sampNumCoef[i];
               rpc.theSampDenCoef[i] = rpcPar.sampDenCoef[i];
            }
            biasError = mdl->getBiasError();
            randError = mdl->getRandError();
              
            std::cout << sp26 <<" bias/rand: "<<biasError<<"/"
                                              <<randError<<" [m]"<<std::endl;
            ossimPositionQualityEvaluator qev4
               (intECF, biasError, randError, rpc);
            epOK = qev4.computeCE_LE(pLev, CEpLev, LEpLev);
            if (epOK)
            {
               qev4.extractErrorEllipse(pLev, ell);
               std::cout << sp26 <<pLevC<<" CE/LE: ( "
                         <<setprecision(4)<<CEpLev<<" / "<<LEpLev
                         <<" ) [ft]"<<std::endl;
               std::cout << sp26 <<" Ellipse:   ( "
                         <<ell.theSemiMajorAxis<<" / "<<ell.theSemiMinorAxis
                         <<" ) [m]  @ "<<ell.theAzimAngle*DEG_PER_RAD
                         <<" deg"<<std::endl;
               pqeImageErrorEllipse_t ellImg;
               qev4.extractErrorEllipse(pLev, 30.0, ell, ellImg);
//                std::cout << sp26 <<" Image ellipse test..........(s,l)"<<endl;
//                for (unsigned int i=0; i<ellImg.size(); ++i)
//                   std::cout << sp26<<"   "<<i<<" "<<ellImg[i]<<endl;
            }
            else
            {
               std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
            }
         
         }
         else
         {
            std::cout << sp26 <<" Error Prop Not Available"<<std::endl;
         }

         } // check for RPC
         else
         {
            std::cout << " This is not an RPC model..."<<std::endl;
         }
         
      } //img loop
      
   } //pt loop

  
   }  //case RPC
   break;
   
   
   
   default:
      cout << "\nInvalid processing type...\n" << endl;
      
   } //procType switch
      
      
   std::cout <<"\n"<< line<< std::endl;

   exit(0);
}
