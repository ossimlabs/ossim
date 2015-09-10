//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimSlopeUtil.h 23443 2015-07-17 15:57:00Z okramer $

#ifndef ossimSlopeUtil_HEADER
#define ossimSlopeUtil_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ostream>

/*!
 *  Class for computing the slope on each elevation post and generatinga corresponding slope image.
 *  The output scalar type is a normalized float unless unsigned 8-bit is selected via the options.
 */
class OSSIMDLLEXPORT ossimSlopeUtil : public ossimObject,
                                      public ossimProcessInterface,
                                      public ossimListenerManager
{
public:
   ossimSlopeUtil();
   ~ossimSlopeUtil();

   /**
    * Initializes from command line arguments.
    */
   bool initialize(ossimArgumentParser& ap);

   /**
    * Returns true if successful
    */
   virtual bool execute();

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };

   void printApiJson(std::ostream& out) const;

protected:
   bool initializeChain();
   bool loadDemFile();
   bool loadElevDb();
   void usage(ossimArgumentParser& ap);
   void addArguments(ossimArgumentParser& ap);
   bool writeJsonApi(const ossimFilename& outfile);

   ossimFilename m_demFile;
   ossimFilename m_slopeFile;
   ossimFilename m_lutFile;
   ossimGpt m_centerGpt;
   double m_aoiRadius; // meters
   bool m_remapToByte;
   ossimRefPtr<ossimImageSource> m_procChain;
};

#endif
