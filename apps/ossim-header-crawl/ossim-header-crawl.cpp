//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Ken Melero
//         Originally written by Oscar Kramer.
//         
// Description:  This app displays a binary file in a nice
//               formatted view.  Very helpful for finding offsets of
//               binary headers.
//
//********************************************************************
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <memory>
#include <sstream>

using namespace std;

// first: header file name only, second: header file's "namespace" or full path if relative
typedef map<ossimFilename, ossimFilename> HeaderMap;

typedef vector<ossimFilename> FileList;


class CppHeaderCrawler
{
public:
   CppHeaderCrawler();

   // Mines the CMake output files for include paths and home directory
   bool loadBuild(const ossimFilename &buildDir);

   // Opens each source and header to determine external header dependencies
   bool scanForHeaders();

   // Copies all external headers to the sandbox output directory, preserving the relative namespace
   bool copyHeaders(const ossimFilename &outDir);

private:

   // Scans specified file for include files to record:
   bool scanFile(const ossimFilename &file);

   // Finds the path needed to prepend to the includeSpec to locate the existing include file.
   // Returns empty string if not found:
   ossimFilename findPath(const ossimFilename &includeSpec);

   ossimFilename m_ossimDevHome;   // = $OSSIM_DEV_HOME as specified in CMakeCache.txt
   FileList      m_includeDirs;    // List of all include paths searched in build
   FileList      m_sourceNames;    // List of all OSSIM source and header files accessed in build
   HeaderMap     m_includePathMap; // Contains existing include path for every header file
};


void usage(char* appname)
{
   cout<<"\nUsage:  " << appname << " <path/to/OSSIM_BUILD_DIR> <path-to-output-dir>\n" << endl;
   cout<<"  Utility app to copy all external header files on a system that are referenced by the \n"
       <<"  OSSIM build. The headers are copied into a \"sandbox\" directory (usually appended with \n"
       <<"  \"include\"), preserving namespaces. This is to enable sandbox builds. See the script in\n"
       <<"  ossim/scripts/ocpld.sh for copying the external libraries needed.\n"<<endl;
   return;
}

int main(int argc, char** argv)
{
   if (argc < 3)
   {
      usage(argv[0]);
      return 1;
   }

   ossimFilename buildPath = argv[1];
   ossimFilename destDir = argv[2];

   CppHeaderCrawler crawler;

   if (!crawler.loadBuild(buildPath))
      return -1;

   if (!crawler.scanForHeaders())
      return -1;

   if (!crawler.copyHeaders(destDir))
      return -1;

   return 0;
}

CppHeaderCrawler::CppHeaderCrawler()
{
}

bool CppHeaderCrawler::loadBuild(const ossimFilename &build_dir)
{
   static const string OSSIM_DEV_HOME_STR = "OSSIM_DEV_HOME:";
   ossimString line;

   // Check environment for OSSIM_DEV_HOME:
   const char* envVar = getenv("OSSIM_DEV_HOME");
   if (envVar)
   {
      m_ossimDevHome = envVar;
   }
   else
   {
      // Checkout the CMakeCache.txt to mine it for OSSIM_DEV_HOME:
      ossimFilename cmakeCacheFile = build_dir + "/CMakeCache.txt";
      ifstream f(cmakeCacheFile.string());
      if (f.fail())
      {
         cout << "Failed file open for CMake file: " << cmakeCacheFile << endl;
         return false;
      }

      // Loop to read read one line at a time to find OSSIM_DEV_HOME:
      while (getline(f, line))
      {
         if (!line.contains(OSSIM_DEV_HOME_STR))
            continue;
         m_ossimDevHome = line.after("=");
         break;
      }
      f.close();
   }
   if (m_ossimDevHome.empty())
   {
      cout << "Could not determine OSSIM_DEV_HOME. This should not happen!" << endl;
      return false;
   }

   // Now read the cmake-generated list files. First the include directories:
   ossimFilename cmakeIncludeDirs (build_dir.dirCat("CMakeIncludeDirs.txt"));
   ifstream f(cmakeIncludeDirs.string());
   if (f.fail())
   {
      cout << "Failed file open for CMake file: " << cmakeIncludeDirs << endl;
      return false;
   }
   while (getline(f, line))
   {
      if (!line.contains(m_ossimDevHome))
      {
         cout << "Adding include path <" << line << ">" << endl;
         m_includeDirs.emplace_back(line);
      }
   }
   f.close();

   // Read list of sources and headers included in the build:
   ossimFilename cmakeFilenames (build_dir.dirCat("CMakeFileNames.txt"));
   f.open(cmakeFilenames.string());
   if (f.fail())
   {
      cout << "Failed file open for CMake file: " << cmakeFilenames << endl;
      return false;
   }
   while (getline(f, line))
   {
      cout << "Adding source/header file <" << line << ">" << endl;
      m_sourceNames.emplace_back(line);
   }
   f.close();

   return true;
}


bool CppHeaderCrawler::scanForHeaders()
{
   // First find all files that match pattern:
   for (auto &sourceName : m_sourceNames)
   {
      scanFile(sourceName);
   }
   return true;
}

bool CppHeaderCrawler::scanFile(const ossimFilename& sourceName)
{
   static const ossimString INCLUDE_STRING = "#include ";
   static const size_t SIZE_INCLUDE_STRING = INCLUDE_STRING.length();
   
   // The file may be an absolute path or may need to be searched among include paths:
   // Open one file:
   ifstream f(sourceName.string());
   if (f.fail())
   {
      cout << "Failed file open for: " << sourceName << endl;
      return false;
   }

   cout << "Scanning file: " << sourceName << endl;
   bool foundInclude = false;
   int noIncludeCount = 0;
   ossimString lineOfCode;

   // Loop to read read one line at a time to check for includes:
   while (getline(f, lineOfCode) && (noIncludeCount < 10))
   {
      ossimString substring(lineOfCode.substr(0, SIZE_INCLUDE_STRING));
      if (substring != INCLUDE_STRING)
      {
         if (foundInclude)
            noIncludeCount++;
         continue;
      }
      foundInclude = true;
      noIncludeCount = 0;

      // Get the include file path/name. Determine if relative or "namespaced". Truly relative
      // include spec need not be copied to destination directory since the shall be present with
      // the source build anyway.
      ossimString includeSpec = lineOfCode.after(INCLUDE_STRING);
      includeSpec.trim();
      if (includeSpec.empty())
         continue;
      if (includeSpec[0] == '"')
      {
         // Relative. Some people are sloppy and use quoted header file spec even when it is really
         // namespaced, so need to search relative first:
         includeSpec = includeSpec.after("\"").before("\""); // stop before second quote (in case comments follow)
         ossimFilename pathFile = sourceName.path().dirCat(includeSpec);
         if (pathFile.exists())
            continue;
      }
      else
      {
         includeSpec = includeSpec.after("<").before(">"); // stop before second quote (in case comments follow)
      }

      // Search the namespaced include spec list if already entered:
      auto entry = m_includePathMap.find(includeSpec);
      if (entry != m_includePathMap.end())
         continue;


      // Exclude copying headers that are in the source tree (not external):
      auto sourcePath = m_sourceNames.begin();
      for (; sourcePath != m_sourceNames.end(); ++sourcePath)
      {
         if (sourcePath->contains(includeSpec))
            break;
      }
      if (sourcePath!=m_sourceNames.end())
         continue;

      // First time this external header has been encountered, Find it on the system and save the
      // associated include path and namespace portion:
      ossimFilename path = findPath(includeSpec);
      if (!path.empty())
      {
         cout << "Inserting " << includeSpec << endl;
         m_includePathMap.emplace(includeSpec, path);
      }

      // Now recursion into the rabbit hole of header dependencies:
      ossimFilename fullPathFile = path.dirCat(includeSpec);
      if (fullPathFile.ext().empty())
         continue; // System include should be on target already
      scanFile(fullPathFile);
   }
   f.close();
   return true;
}

bool CppHeaderCrawler::copyHeaders(const ossimFilename& outputDir)
{
   ossimFilename path, existingLocation, newLocation;
   for (auto &header : m_includePathMap)
   {
      // Check existence of header on system:
      existingLocation = header.second.dirCat(header.first);
      if (!existingLocation.isFile())
      {
         cout << "ERROR: Could not find <" << existingLocation << ">. Header was not copied." << endl;
         continue;
      }

      // Copy the file to the output directory:
      newLocation = outputDir.dirCat(header.first);
      ossimFilename newDir (newLocation.path());
      ossimFilename newFile(newLocation.file());
      if (!newDir.createDirectory())
      {
         cout << "ERROR: Could not create directory <" << newDir << ">. Check permissions." << endl;
         return false;
      }

      existingLocation.copyFileTo(newLocation);
      cout << "Copied <" << header.first << ">"<< endl;
   }
   return true;
}

ossimFilename CppHeaderCrawler::findPath(const ossimFilename &file)
{
   ossimFilename fullPath, result;
   for (auto &path: m_includeDirs)
   {
      fullPath = path.dirCat(file);
      if (fullPath.exists())
      {
         result = path;
         break;
      }
   }
   return result;
}
