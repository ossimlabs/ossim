//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Application to compare to workspaces.
//
// $Id: foo.cpp 14815 2009-07-01 14:59:11Z gpotts $
//----------------------------------------------------------------------------

#include <cstdlib> /* for system command */
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/init/ossimInit.h>
#include <iostream>
using namespace std;

static bool trace = false; // Turn on to see more stuff...

static void cmpDir(const ossimFilename& wsa,
                   const ossimFilename& wsb,
                   const ossimFilename& dir);

static void cmpFile(const ossimFilename& wsa,
                    const ossimFilename& wsb,
                    const ossimFilename& file);

static bool isIgnoredFile(const ossimFilename& file);

static void usage()
{
   cout << "Usage:  ossim-ws-cmp <wsa> <wsb>\n"
        << "Compares workspace a (wsa) to workspace b (wsb).\n"
        << "NOTE: requires \"diff\" command."
        << endl;
}

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
      usage();
      return 0;
   }

   ossimFilename wsa = argv[1];
   ossimFilename wsb = argv[2];

   if ( !wsa.isDir() )
   {
      cout << "<wsa> is NOT a directory: " << wsa << endl;
      usage();
      return 0;
   }

   if ( !wsb.isDir() )
   {
      cout << "<wsb> is NOT a directory: " << wsb << endl;
      usage();
      return 0;
   }

   cout << "wsa: " << wsa
        << "\nwsb: " << wsb
        << endl;

   cmpDir(wsa, wsb, wsa);
   
   return 0;
}

void cmpDir(const ossimFilename& wsa,
            const ossimFilename& wsb,
            const ossimFilename& dir)
{
   ossimDirectory d;
   if ( d.open(dir) )
   {
      ossimFilename f;
      if (d.getFirst(f))
      {
         while ( f != "" )
         {
            // cout << f << endl;

            if ( isIgnoredFile(f) == false )
            {
               if ( f.isDir() )
               {
                  cmpDir(wsa, wsb, f);
               }
               else
               {
                  cmpFile(wsa, wsb, f);
               }
            }
            d.getNext(f);
         }
      }
   }
   else
   {
      cout << "Could not open: " << wsa << endl;
   }
}

void cmpFile(const ossimFilename& wsa,
             const ossimFilename& wsb,
             const ossimFilename& file)
{
   ossimFilename bFile = file.substitute(wsa, wsb);

   if ( !file.exists() )
   {
      cout << "\nnotice: wsb file: " << bFile
           << "\nnotice: wsa file does not exists: " << file
           << "\nb -> a copy command:"
           << "\ncp " << bFile << " " << file << "\n"
           << endl;
   }
   if ( !bFile.exists() )
   {
      cout << "\nnotice: wsa file: " << file
           << "\nnotice: wsb file does not exists: " << bFile
           << "\na -> b copy command:"
           << "\ncp " << file << " " << bFile << "\n"
           << endl;
   }

   if ( file.exists() && bFile.exists() )
   {
      std::string command = "diff -w --ignore-matching-lines=\\$Id ";
      command += file.string();
      command += " ";
      command += bFile.string();
      
      int status = system( command.c_str() );
      
      if ( status != 0 )
      {
         cout << "\nnotice files differ:"
              << "\nwsa file: " << file
              << "\nwsb file: " << bFile
              << "\na -> b copy command:"
              << "\ncp " << file << " " << bFile
              << "\nb -> a copy command:"
              << "\ncp " << bFile << " " << file << "\n"
              << endl;
      }
   }
}

bool isIgnoredFile(const ossimFilename& file)
{
   bool status = false;

   if (trace)
   {
      cout << "file: " << file << endl;
   }
   
   if ( file.size() )
   {
      ossimFilename f = file.file();
      ossimFilename e = file.ext();
      
      if ( ( f == ".moc" )                  ||
           ( f == ".svn" )                  ||
           ( f == "CMakeCache.txt" )        ||
           ( f == "CMakeFiles" )            ||
           ( f == "cmake_install.cmake" )   ||
           ( f == "cmake_uninstall.cmake" ) ||
           ( f == "CVS" )                   ||
           ( f == "doc" )                   || 
           ( f == ".cvsignore" )            ||
           ( f == "bin" )                   ||
           ( f == "build" )                 ||
           ( f == "builds" )                ||
           ( f == "configure")              ||
           ( f == "config.log" )            ||
           ( f == "config.status")          ||
           ( f == "lib" )                   ||
           ( f == "Makefile" )              ||
           ( f == "Makefile.common" )       ||
           ( f == "make.out" )              ||
           ( f == "projects")               ||
           ( f == "wxmac.icns")             ||
           ( f == "xcode" )                 ||
           ( e == "d" )                     || // dot d file
           ( e == "o" )                     || // object file
           ( e == "obj" )                   ||
           ( e == "exe" )                   ||
           ( e == "tmp" )
           )
      {
         status = true;
      }
      else if ( file.contains("apps") || file.contains("test") )
      {
         if ( file.isDir() ) // Go into apps and test dir.
         {
            status = false;
         }
         else if ( (e != "h") && (e != "cpp") )
         {
            // Ignore binary files. Only diff headers and source files.
            status = true;
         }
      }
      if ( file[file.size()-1] == '~' )
      {
         status = true; // xemacs
      }
      
   } // if ( file.size() )
   else
   {
      status = true; // empty
   }

   if ( trace && (status == true) )
   {
      cout << "ignoring file: " << file << endl;
   }
   
   return status;
}
