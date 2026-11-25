//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// ossimTempFilename class test app.
//
// Covers issue #333: generated temporary names must be unpredictable and the
// file must be created atomically so an existing file or symbolic link is
// never followed or clobbered.
//
//----------------------------------------------------------------------------

#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimTempFilename.h>
#include <ossim/base/Thread.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#if !defined(_WIN32)
#  include <sys/stat.h>
#endif

namespace
{
   int g_failureCount = 0;

   void check(bool condition, const std::string& description)
   {
      if (condition)
      {
         std::cout << "PASSED: " << description << std::endl;
      }
      else
      {
         std::cout << "FAILED: " << description << std::endl;
         ++g_failureCount;
      }
   }

   std::string toString(const ossimFilename& filename)
   {
      return std::string(filename.c_str());
   }

   // Splits "ossim-temp-0123456789abcdef.tmp" into the random part, that is
   // the 16 characters preceding the extension.
   std::string randomPart(const std::string& name)
   {
      const std::size_t dot = name.find_last_of('.');
      const std::string base = (dot == std::string::npos) ? name : name.substr(0, dot);
      if (base.size() <= 16)
      {
         return base;
      }
      return base.substr(base.size() - 16);
   }

   /**
    * @brief Creates temporary files from several threads at the same time.
    */
   class TempFileThread : public ossim::Thread
   {
   public:
      TempFileThread(int numberToCreate)
         : m_numberToCreate(numberToCreate)
      {
      }

      virtual void run()
      {
         for (int i = 0; i < m_numberToCreate; ++i)
         {
            ossimTempFilename tempFile(".", "ossim-temp-thread-", "tmp", true, false);
            tempFile.generateRandomFile();

            const bool exists = tempFile.exists();

            std::lock_guard<std::mutex> lock(m_mutex);
            if (!exists)
            {
               ++m_missingCount;
            }
            m_names.push_back(toString(tempFile));
         }
      }

      static int getMissingCount()
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_missingCount;
      }

      static std::vector<std::string> getNames()
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         return m_names;
      }

   private:
      int m_numberToCreate;
      static std::mutex m_mutex;
      static std::vector<std::string> m_names;
      static int m_missingCount;
   };

   std::mutex TempFileThread::m_mutex;
   std::vector<std::string> TempFileThread::m_names;
   int TempFileThread::m_missingCount = 0;

   void testFileCreation()
   {
      std::cout << "\n--- file creation ---" << std::endl;

      ossimTempFilename tempFile(".", "ossim-temp-", "txt", true, false);
      tempFile.generateRandomFile();

      check(!tempFile.empty(), "generateRandomFile() assigns a name");
      check(tempFile.exists(), "generated file exists on disk");
      check(tempFile.isFile(), "generated path is a regular file");
      check(tempFile.ext() == ossimString("txt"), "extension is applied");

      const std::string name = toString(tempFile);
      check(name.find("ossim-temp-") != std::string::npos, "prefix is honored");

#if !defined(_WIN32)
      struct stat st;
      if (::stat(name.c_str(), &st) == 0)
      {
         check((st.st_mode & 0777) == 0600,
               "temporary file is created with owner only permissions (0600)");
      }
#endif
   }

   void testUniqueness()
   {
      std::cout << "\n--- uniqueness ---" << std::endl;

      const int fileCount = 200;
      std::set<std::string> names;
      int missingCount = 0;

      for (int i = 0; i < fileCount; ++i)
      {
         ossimTempFilename tempFile(".", "ossim-temp-unique-", "tmp", true, false);
         tempFile.generateRandomFile();
         if (!tempFile.exists())
         {
            ++missingCount;
         }
         names.insert(toString(tempFile));
      }

      check(missingCount == 0, "every generated file exists");
      check(static_cast<int>(names.size()) == fileCount,
            "200 generated names are all distinct");
   }

   void testNameEntropy()
   {
      std::cout << "\n--- name entropy ---" << std::endl;

      const int fileCount = 100;
      std::set<std::uint64_t> values;
      bool allHex = true;
      bool allLength16 = true;

      for (int i = 0; i < fileCount; ++i)
      {
         ossimTempFilename tempFile(".", "ossim-temp-entropy-", "tmp", true, false);
         tempFile.generateRandomFile();

         const std::string random = randomPart(toString(tempFile));
         allLength16 = allLength16 && (random.size() == 16);
         allHex = allHex && (random.find_first_not_of("0123456789abcdef") == std::string::npos);
         if (random.size() == 16)
         {
            values.insert(static_cast<std::uint64_t>(std::strtoull(random.c_str(), 0, 16)));
         }
      }

      check(allLength16, "random part is 16 characters long");
      check(allHex, "random part is lowercase hexadecimal");

      // Sequential names ("prefix" + n) would be no more than a few units
      // apart; the generated values must be far apart from each other.
      bool spread = true;
      std::uint64_t previous = 0;
      for (std::set<std::uint64_t>::const_iterator it = values.begin();
           it != values.end();
           ++it)
      {
         if (previous != 0)
         {
            spread = spread && ((*it - previous) > 1024);
         }
         previous = *it;
      }
      check(spread, "generated names are not a sequential counter");
   }

   void testDirectoryCreation()
   {
      std::cout << "\n--- directory creation ---" << std::endl;

      ossimTempFilename tempDir1(".", "ossim-temp-dir-", "", true, false);
      tempDir1.generateRandomDir();

      ossimTempFilename tempDir2(".", "ossim-temp-dir-", "", true, false);
      tempDir2.generateRandomDir();

      check(!tempDir1.empty(), "generateRandomDir() assigns a name");
      check(tempDir1.exists(), "generated directory exists on disk");
      check(tempDir1.isDir(), "generated path is a directory");
      check(toString(tempDir1) != toString(tempDir2),
            "consecutive calls generate different names");
   }

   void testGlobalRandomState()
   {
      std::cout << "\n--- global random state ---" << std::endl;

      std::srand(12345);
      const int expected = std::rand();

      std::srand(12345);
      ossimTempFilename tempFile(".", "ossim-temp-rand-", "tmp", true, false);
      tempFile.generateRandomFile();
      const int actual = std::rand();

      check(expected == actual,
            "generate() does not reseed the global rand() sequence");
   }

   void testFailure()
   {
      std::cout << "\n--- failure handling ---" << std::endl;

      ossimFilename bogusDir("/ossim/does/not/exist/temp");
      bogusDir = bogusDir.expand();

      ossimTempFilename tempFile(bogusDir, "ossim-temp-", "tmp", true, false);
      tempFile.generateRandomFile();

      check(tempFile.empty(),
            "an unwritable temp directory leaves the filename empty");
   }

   void testThreads()
   {
      std::cout << "\n--- threads ---" << std::endl;

      const int threadCount = 4;
      const int filesPerThread = 50;

      std::vector<TempFileThread*> threads;
      for (int i = 0; i < threadCount; ++i)
      {
         threads.push_back(new TempFileThread(filesPerThread));
      }
      for (std::size_t i = 0; i < threads.size(); ++i)
      {
         threads[i]->start();
      }
      for (std::size_t i = 0; i < threads.size(); ++i)
      {
         delete threads[i]; // Joins.
      }

      check(TempFileThread::getMissingCount() == 0,
            "every thread created its files");

      const std::vector<std::string> names = TempFileThread::getNames();
      const std::set<std::string> uniqueNames(names.begin(), names.end());
      check(static_cast<int>(uniqueNames.size()) == (threadCount * filesPerThread),
            "names generated by concurrent threads are all distinct");
   }

} // End anonymous namespace.

int main(int argc, char* argv[])
{
   ossimInit::instance()->initialize(argc, argv);

   testFileCreation();
   testUniqueness();
   testNameEntropy();
   testDirectoryCreation();
   testGlobalRandomState();
   testFailure();
   testThreads();

   if (g_failureCount != 0)
   {
      std::cout << "\n" << g_failureCount << " test(s) FAILED" << std::endl;
      return 1;
   }

   std::cout << "\nAll tests PASSED" << std::endl;
   return 0;
}
