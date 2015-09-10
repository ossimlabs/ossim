//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Oscar Kramer
//
//*************************************************************************
// $Id: ossimEquationUtil.h 23408 2015-07-07 10:20:17Z okramer $

#ifndef ossimEquationUtil_HEADER
#define ossimEquationUtil_HEADER

#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <vector>

class OSSIMDLLEXPORT ossimEquationUtil : public ossimObject,
                                         public ossimProcessInterface,
                                         public ossimListenerManager
{
public:
   ossimEquationUtil(ossimArgumentParser& ap);
   ~ossimEquationUtil() {}

   /**
    * Returns true if successful
    */
   virtual bool execute();

   virtual ossimObject* getObject() { return this; }
   virtual const ossimObject* getObject() const  { return this; }
   virtual ossimListenerManager* getManager()  { return this; };

protected:
   ossimEquationUtil() : m_argumentParser(0) {}

   void addArguments(ossimArgumentParser& ap);
   void outputTemplateKeywordlist(const ossimFilename &templateFilename);
   bool parseKwl(const ossimFilename& kwl_file,
                 std::vector<ossimFilename>& inputs,
                 ossimString& equationSpec,
                 ossimFilename& output,
                 ossimString& writerType);
   bool initInputSources(std::vector<ossimFilename>& fileList);

   ossimArgumentParser* m_argumentParser;
   ossimApplicationUsage m_usage;
   ossimConnectableObject::ConnectableObjectList m_inputSources;
   ossimRefPtr<ossimImageGeometry> m_prodGeometry;
};




#endif
