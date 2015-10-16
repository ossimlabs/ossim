//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id: ossimSlopeUtil.cpp 23450 2015-07-27 13:58:00Z okramer $

#include <ossim/util/ossimUtility.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimNotify.h>
#include <iostream>

using namespace std;

ossimUtility::ossimUtility()
{
}

ossimUtility::~ossimUtility()
{
}

void ossimUtility::setUsage(ossimArgumentParser& ap)
{
   // Add global usage options.
   ossimInit::instance()->addOptions(ap);

   ossimApplicationUsage* au = ap.getApplicationUsage();
   au->addCommandLineOption(
         "--write-template <filename>",
         "Writes a template keyword-list to the specified filename.");

}

bool ossimUtility::initialize(ossimArgumentParser& ap)
{
   if ( (ap.argc() == 1) || ap.read("-h") || ap.read("--help") )
   {
      // Write usage.
      setUsage(ap);
      ap.getApplicationUsage()->write(ossimNotify(ossimNotifyLevel_INFO));
      return false;
   }

   std::string ts1;
   ossimArgumentParser::ossimParameter sp1(ts1);

   if ( ap.read("--write-template", sp1))
   {
      ofstream ofs ( ts1.c_str() );
      ossimKeywordlist kwl;
      getTemplate(kwl);
      kwl.print(ofs);
      ofs.close();
      return false;
   }

   return true;
}

