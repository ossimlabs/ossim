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

// List of all include paths as specified in CMake cache
typedef vector<ossimFilename> PathList;


class CppHeaderCrawler
{
public:
   CppHeaderCrawler();

   // Mines the CMake cache file for include paths and home directory
   bool mineCMake(const ossimFilename &buildDir);

   bool scanCollection(const string &pattern);
   bool copyHeaders(const ossimFilename &outDir);
   size_t getNumScans() const { return m_numScans; }

private:

   // Scans specified file for include files to record:
   void scanFile(const ossimFilename &file);

   // Finds the path needed to prepend to the includeSpec to locate the existing include file.
   // Returns empty string if not found:
   ossimFilename findPath(const ossimFilename &includeSpec);

   ossimFilename m_ossimDevHome;   // = $OSSIM_DEV_HOME as specified in CMakeCache.txt
   PathList      m_includePaths;   // List of all include paths searched in build
   PathList      m_sourcePaths;    // List of all source paths searched in build
   HeaderMap     m_includePathMap; // Contains existing include path for every header file
   HeaderMap     m_namespaceMap;   // Contains namespace portion of header to reproduce in destination
   size_t        m_numScans;
};


int main(int argc, char** argv)
{
   if (argc < 3)
   {
      cout << "\nUsage:  " << argv[0] << " <path/to/OSSIM_BUILD_DIR> <path-to-output-dir>\n" << endl;
      return 1;
   }

   ossimFilename buildPath = argv[1];
   ossimFilename destDir = argv[2];

   CppHeaderCrawler crawler;

   if (!crawler.mineCMake(buildPath))
      return -1;

   string cppFilePattern ("\\*.cpp");
   string cFilePattern ("\\*.c");
   if (!crawler.scanCollection(cppFilePattern) || !crawler.scanCollection(cFilePattern))
      return -1;

   if (crawler.getNumScans() == 0)
   {
      cout << "No files were scanned! Check command line and permissions."<<endl;
      return -1;
   }
   else if (!crawler.copyHeaders(destDir))
      return -1;

   return 0;
}

CppHeaderCrawler::CppHeaderCrawler()
:  m_numScans(0)
{
   m_includePaths.emplace_back("/usr/include");
   cout << "Adding includePath </usr/include>" << endl;
   m_includePaths.emplace_back("/usr/local/include");
   cout << "Adding includePath </usr/local/include>" << endl;
}

bool CppHeaderCrawler::mineCMake(const ossimFilename& build_dir)
{
   static const string OSSIM_DEV_HOME_STR = "OSSIM_DEV_HOME:";

   ossimFilename cmakeCacheFile = build_dir + "/CMakeCache.txt";
   ifstream f(cmakeCacheFile.string());
   if (f.fail())
   {
      cout << "Failed file open for CMake file: " << cmakeCacheFile << endl;
      return false;
   }

   // Loop to read read one line at a time to find OSSIM_DEV_HOME:
   ossimString line;
   while (std::getline(f, line.string()))
   {
      if (!line.contains(OSSIM_DEV_HOME_STR))
         continue;
      m_ossimDevHome = line.after("=");
      break;
   }
   f.close();
   if (m_ossimDevHome.empty())
   {
      cout << "Did not find OSSIM_DEV_HOME in CMake file: " << cmakeCacheFile << endl;
      return false;
   }

   // Add ossim search path to the list:
   ossimFilename ossimIncludes (m_ossimDevHome.dirCat("ossim/include"));
   m_includePaths.emplace_back(ossimIncludes);

   // Now scan for include path specs:
   f.open(cmakeCacheFile.string());
   while (std::getline(f, line.string()))
   {
      if (line.contains("_INCLUDE_"))
      {
         // Found include spec. Check if already added:
         ossimFilename includePath = line.after("PATH=");
         if (includePath.empty() || includePath.contains("-NOTFOUND"))
            continue;
         bool found = false;
         for (auto &existingPath : m_includePaths)
         {
            if (includePath == existingPath)
            {
               found = true;
               break;
            }
         }
         if (!found)
         {
            cout << "Adding include path <" << includePath << ">" << endl;
            m_includePaths.emplace_back(includePath);
         }
      }
      else if (line.contains("_SOURCE_DIR:STATIC="))
      {
         // Found source spec:
         ossimFilename sourcePath = line.after("STATIC=");
         if (sourcePath.empty())
            continue;
         bool found = false;
         if (sourcePath == m_ossimDevHome)
            found = true;
         else
         {
            for (auto &existingPath : m_sourcePaths)
            {
               if (sourcePath.contains(existingPath))
               {
                  found = true;
                  break;
               }
               else if (existingPath.contains(sourcePath+"/"))
               {
                  // Found one level higher, replace it
                  cout << "Replacing existing path <" << existingPath << "> with higher source path"
                       << " <"<<sourcePath<<">"<< endl;
                  existingPath = sourcePath;
                  found = true;
                  break;
               }
            }
         }
         if (!found)
         {
            cout << "Adding source path <" << sourcePath << ">" << endl;
            m_sourcePaths.emplace_back(sourcePath);
         }
      }
   }
   f.close();
   return true;
}


bool CppHeaderCrawler::scanCollection(const string &pattern)
{
   // First find all files that match pattern:
   char buffer[1024];
   ossimFilename filename;
   bool statusOK = true;

   for (auto &sourcePath : m_sourcePaths)
   {
      ostringstream command;
      command<<"find "<<sourcePath<<" -type f -name "<<pattern<<" -print";
      cout << "Running command \"" << command.str() << "\"" << endl;

      // Need to change to top dir, save current:
      //char cwd[1024];
      //getcwd(cwd, sizeof(cwd));
      //if (chdir(m_ossimDevHome.chars()) < 0)
      //{
      //   cout <<"ERROR: Could not chdir to top <"<<m_ossimDevHome<<">!"<<endl;
      //   return false;
      //}

      std::shared_ptr<FILE> pipe(popen(command.str().c_str(), "r"), pclose);
      if (pipe)
      {
         // Fetch the console output of the find command:
         while (!feof(pipe.get()))
         {
            if (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
            {
               filename = buffer;
               filename.trim();
               scanFile(filename);
            }
         }
         if (m_numScans == 0)
            cout << "No files were scanned." << endl;
      }
      else
      {
         cout << "ERROR: Could not create pipe to run find command!" << endl;
         statusOK = false;
      }
   }
   //chdir(cwd);
   return statusOK;
}


void CppHeaderCrawler::scanFile(const ossimFilename &xfile)
{
   static const ossimString INCLUDE_STRING = "#include ";
   static const size_t SIZE_INCLUDE_STRING = INCLUDE_STRING.length();

   // The file may be an absolute path or may need to be searched among include paths:
   // Open one file:
   ifstream f (xfile.string());
   if (f.fail())
   {
      cout<<"Failed file open for: "<<xfile<<endl;
      return;
   }

   cout<<"Scanning file: "<<xfile<<endl;
   ++m_numScans;
   bool foundInclude=false;
   int noIncludeCount = 0;
   ossimString lineOfCode;

   // Loop to read read one line at a time to check for includes:
   while (getline(f, lineOfCode.string()) && (noIncludeCount < 10))
   {
      ossimString substring (lineOfCode.substr(0,SIZE_INCLUDE_STRING));
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
      ossimFilename includeSpec = lineOfCode.after(INCLUDE_STRING);
      includeSpec.trim();
      if (includeSpec.empty())
         continue;
      if (includeSpec[0] == '"')
      {
         // Relative. Some people are sloppy and use quoted header file spec even when it is really
         // namespaced, so need to search relative first:
         includeSpec = includeSpec.after("\"").before("\""); // stop before second quote (in case comments follow)
         ossimFilename pathFile = xfile.path().dirCat(includeSpec);
         if (pathFile.exists())
            continue;
      }
      else
      {
         includeSpec = includeSpec.after("<").before(">"); // stop before second quote (in case comments follow)
      }

      // Namespaced. Search the map if already entered:
      ossimFilename filepart (includeSpec.file());
      auto mapEntry = m_namespaceMap.find(filepart);
      if (mapEntry != m_namespaceMap.end())
         continue;

      // Exclude copying headers that are already in the source tree:
      bool doInsert = true;
      for (auto &sourcePath : m_sourcePaths)
      {
         if (includeSpec.contains(sourcePath))
         {
            doInsert = false;
            break;
         }
      }

      // First time this header has been encountered, Find it and save the associated include path
      // and namespace portion:
      ossimFilename path = findPath(includeSpec);
      if (doInsert)
      {
         cout << "Inserting " << filepart << endl;
         m_namespaceMap.emplace(filepart, includeSpec);
         m_includePathMap.emplace(filepart, path);
      }

      // Now recursion into the rabbit hole of header dependencies:
      ossimFilename fullPathFile = path.dirCat(includeSpec);
      if (fullPathFile.ext().empty())
         continue; // System include should be on target already
      scanFile(fullPathFile);
   }

   f.close();
}

bool CppHeaderCrawler::copyHeaders(const ossimFilename& outputDir)
{
   ossimFilename path, existingLocation, newLocation;
   for (auto &header : m_namespaceMap)
   {
      auto includePathIter = m_includePathMap.find(header.first);
      if (includePathIter == m_includePathMap.end())
      {
         cout << "ERROR: Could not find <" << header.first << "> in map. This should not happen! "
              << "Skipping." << endl;
         continue;
      }

      existingLocation = includePathIter->second.dirCat(header.second);
      if (!existingLocation.isFile())
      {
         cout << "ERROR: Could not find <" << existingLocation << ">. Header was not copied." << endl;
         continue;
      }

      // Copy the file to the output directory:
      newLocation = outputDir.dirCat(header.second);
      existingLocation.copyFileTo(newLocation);
      cout << "Copied <" << header.first << ">"<< endl;
   }
}

ossimFilename CppHeaderCrawler::findPath(const ossimFilename &file)
{
   ossimFilename fullPath, result;
   for (auto &path: m_includePaths)
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
