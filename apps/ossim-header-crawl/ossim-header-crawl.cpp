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
#include <unistd.h>
#include <glob.h>
#include <vector>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <libgen.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimDirectory.h>

using namespace std;

bool mineCMake (const ossimFilename& build_dir,
                ossimFilename& ossimDevHome,
                vector<ossimFilename>& includePaths);
bool searchSet (const string& pattern,
                const ossimFilename& topDir,
                map<ossimFilename, ossimFilename>& headerMap);
void searchFile(const ossimFilename& file,
                map<ossimFilename, ossimFilename>& headerMap);
bool copyHeaders(const vector<ossimFilename>& includePaths,
                 map<ossimFilename, ossimFilename>& headerMap,
                 const ossimFilename& outputDir);


int main(int argc, char** argv)
{
   if (argc < 3)
   {
      cout << "\nUsage:  " << argv[0] << " <path/to/OSSIM_BUILD_DIR> <path-to-output-dir>\n" << endl;
      return 1;
   }

   ossimFilename buildPath = argv[1];
   ossimFilename destFile = argv[2];

   string cppFilePattern ("*.cpp");
   string cFilePattern ("*.c");
   vector<ossimFilename> includePaths;
   map<ossimFilename, ossimFilename> headerMap;
   ossimFilename ossimDevHome;

   if (!mineCMake(buildPath, ossimDevHome, includePaths))
      return -1;

   if (!searchSet(cppFilePattern, ossimDevHome, headerMap))
      return -1;

   searchSet(cFilePattern, ossimDevHome, headerMap);
   copyHeaders(includePaths, headerMap);

   return 0;
}

bool mineCMake(const ossimFilename& build_dir,
               ossimFilename& ossimDevHome,
               vector<ossimFilename>& includePaths)
{
   static const string OSSIM_DEV_HOME_STR = "OSSIM_DEV_HOME:";
   static const int OSSIM_DEV_HOME_STR_SIZE = OSSIM_DEV_HOME_STR.length();

   ossimFilename cmakeCacheFile = build_dir + "/CMakeCache.txt";
   ifstream f(cmakeCacheFile.string());
   if (f.fail())
   {
      cout << "Failed file open for CMake file: " << cmakeCacheFile << end;
      return false;
   }

   // Loop to read read one line at a time to find OSSIM_DEV_HOME:
   ossimString line;
   while (std::getline(f, line))
   {
      if (!line.contains(OSSIM_DEV_HOME_STR))
         continue;
      ossimDevHome = line.after("=");
   }
   f.close();
   if (ossimDevHome.empty())
      return false;

   // Now scan for include path specs:
   f.open(cmakeCacheFile.string());
   while (std::getline(f, line))
   {
      if (!line.contains("_INCLUDE_"))
         continue;

      // Found include spec:
      ossimFilename includePath = line.after("PATH=");
      if (includePath.empty())
         continue;
      cout << "includePath=" << includePath;
      includePaths.emplace_back(includePath);
   }
   f.close();
   return true;
}


// Called recursively
bool searchSet(const string& pattern,
               const ossimFilename& topDir,
               map<ossimFilename, ossimFilename>& headerMap)
{
   // First find all files that match pattern:
   ossimDirectory directory (topDir);
   if (!directory.isOpened())
      return false;

   vector<ossimFilename> fileList;
   directory.findAllFilesThatMatch(fileList, pattern, ossimDirectory::OSSIM_DIR_FILES);
   if (fileList.empty())
      return false;

   // With list of files, open each and look for includes:
   for (auto &xfile : fileList)
      searchFile(xfile, headerMap);

   return true;
}

void searchFile(const ossimFilename& xfile, map<ossimFilename, ossimFilename>& headerMap)
{
   static const string INCLUDE_STRING = "#include ";

   // Open one file:
   ifstream f (xfile.string());
   if (f.fail())
   {
      cout<<"Failed file open for: "<<xfile<<end;
      return;
   }

   bool foundInclude;
   int noIncludeCount = 0;
   ossimString lineOfCode;

   // Loop to read read one line at a time to check for includes:
   while ((std::getline(f, lineOfCode) && (noIncludeCount < 10))
   {
      if ((lineOfCode.empty() || lineOfCode[0] != '#'))
      {
         if (foundInclude)
            noIncludeCount++;
         continue;
      }

      // Get the include file path/name:
      ossimFilename includeStr = lineOfCode.after(INCLUDE_STRING);
      if (includeStr.empty())
      {
         if (foundInclude)
            noIncludeCount++;
         continue;
      }
      includeStr.trim("\"<>");

      // Fetch the filename part of the include path:
      ossimFilename includePath = includeStr.path();
      cout << "includePath = " << includePath << endl;
      ossimFilename includeFile = includeStr.file();
      cout << "includeFile = " << includeFile << endl;
      noIncludeCount = 0;

      // Search the map if already entered:
      auto mapEntry = headerMap.find(includeFile);
      if (mapEntry == headerMap.end())
         headerMap.emplace(includeFile, includePath);
   }

   f.close();

}

bool copyHeaders(const vector<ossimFilename>& includePaths,
                 const map<ossimFilename, ossimFilename>& headerMap,
                 const ossimFilename& outputDir)
{
   ossimFilename newLocation;
   for (auto &header : headerMap)
   {
      for (auto &includePath : includePaths)
      {
         ossimFilename fullPath = includePath.dirCat(header.second);
         if (fullPath.exists())
         {
            // Copy the file to the output directory:
            newLocation = outputDir.dirCat(header.second);
            fullPath.copyFileTo(newLocation);
            cout<<"Copied <"<<header.first<<"> to "<<outputDir<<endl;
         }
         else
         {
            cout << "ERROR: Could not find <" << fullPath << ">. Header was not copied."<< endl;
         }
      }
   }
}

