//---
// Copyright (C) 2000 ImageLinks Inc. 
//
// License: MIT
//
// Author: Garrett Potts
//
//---
// $Id$

#include <ossim/base/ossimStdOutProgress.h>
#include <ossim/base/ossimPreferences.h>
#include <iomanip>

#if defined(WIN32) || defined(_MSC_VER) && !defined(__CYGWIN__) && !defined(__MWERKS__)
   #include <io.h>
   #define ISATTY _isatty
   #define FILENO _fileno
#else
   #include <unistd.h>
   #define ISATTY isatty
   #define FILENO fileno
#endif

RTTI_DEF1(ossimStdOutProgress, "ossimStdOutProgress", ossimProcessListener);

ossimStdOutProgress theStdOutProgress;

ossimStdOutProgress::ossimStdOutProgress(ossim_uint32 precision,
                                         bool flushStream)
   :
      ossimProcessListener(),
      thePrecision(precision),
      theFlushStreamFlag(flushStream),
      theRunningInConsoleFlag(true)
{
   // Determine if running in a terminal window. Progress reports are only written to console if
   // running in one.

   bool prefsFlag = theRunningInConsoleFlag;
   ossimString stdOutConsole;
   stdOutConsole.string() = ossimPreferences::instance()->
      preferencesKWL().findKey(std::string("ossim.std.out.progress"));

   if ( stdOutConsole.size() )
   {
      // Override auto detected console.
      theRunningInConsoleFlag = stdOutConsole.toBool();
   }
   else if ( !ISATTY(FILENO(stdout) ) )
   {
      theRunningInConsoleFlag = false;
   }
}

void ossimStdOutProgress::processProgressEvent(ossimProcessProgressEvent& event)
{
   if (!theRunningInConsoleFlag)
      return;

   if (event.getOutputMessageFlag())
   {
      ossimString s;
      event.getMessage(s);
      if (!s.empty())
      {
         ossimNotify(ossimNotifyLevel_NOTICE) << s.c_str() << std::endl;
      }
      return; // Don't output percentage on a message update.
   }
   
   double p = event.getPercentComplete();
   ossimNotify(ossimNotifyLevel_NOTICE)
      << std::setiosflags(std::ios::fixed)
      << std::setprecision(thePrecision)
      << p << "%\r";
   
   if(theFlushStreamFlag)
   {
      (p != 100.0) ?
         ossimNotify(ossimNotifyLevel_NOTICE).flush() :
         ossimNotify(ossimNotifyLevel_NOTICE) << "\n";
   }
}

void ossimStdOutProgress::setFlushStreamFlag(bool flag)
{
   theFlushStreamFlag = flag;
}

