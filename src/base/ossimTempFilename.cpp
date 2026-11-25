//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Description: This is a tmeporary filename class it will create a temporary
//              file and will also delete it upon destruction
//
//*************************************************************************
// $Id: ossimTempFilename.cpp 15524 2009-09-30 01:02:26Z dburken $
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <random>
#include <string>

#include <ossim/base/ossimTempFilename.h>
#include <ossim/base/ossimEnvironmentUtility.h>
#include <ossim/base/ossimNotify.h>

#if defined(_WIN32)
#  include <direct.h>
#  include <fcntl.h>
#  include <io.h>
#  include <sys/stat.h>
#else
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace
{
   // Step of the sequence used to decorrelate the generated names.
   const std::uint64_t SEQUENCE_STEP = 0x9e3779b97f4a7c15ULL;

   /**
    * @brief Returns a 16 character, zero padded, hexadecimal string.
    */
   std::string toHexString(std::uint64_t value)
   {
      static const char* digits = "0123456789abcdef";
      std::string result(16, '0');
      for (int i = 15; i >= 0; --i)
      {
         result[i] = digits[static_cast<std::size_t>(value & 0xf)];
         value >>= 4;
      }
      return result;
   }

   /**
    * @brief Returns an unpredictable 64 bit value for a temporary file name.
    *
    * The operating system entropy source is used when available.  A process
    * wide counter and a high resolution clock are mixed in so that names are
    * still unique if random_device has a deterministic implementation.
    */
   std::uint64_t nextRandomValue()
   {
      static std::atomic<std::uint64_t> counter(0);

      std::uint64_t value = 0;
      try
      {
         std::random_device device;
         value = (static_cast<std::uint64_t>(device()) << 32) ^
                  static_cast<std::uint64_t>(device());
      }
      catch (...)
      {
         // Fall back to the clock and counter below.
      }

      value ^= static_cast<std::uint64_t>(
         std::chrono::high_resolution_clock::now().time_since_epoch().count());

      const std::uint64_t sequence =
         counter.fetch_add(1, std::memory_order_relaxed) + 1;
      value ^= sequence * SEQUENCE_STEP;

      return value;
   }

   /**
    * @brief Atomically creates an empty file, failing if it already exists.
    *
    * @return true on success, false on error with errno set.
    */
   bool createFileExclusively(const char* pathname)
   {
#if defined(_WIN32)
      const int fd = _open(pathname,
                           _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY,
                           _S_IREAD | _S_IWRITE);
#else
      const int fd = ::open(pathname,
                            O_CREAT | O_EXCL | O_WRONLY,
                            static_cast<mode_t>(0600));
#endif
      if (fd == -1)
      {
         return false;
      }

#if defined(_WIN32)
      _close(fd);
#else
      ::close(fd);
#endif
      return true;
   }

   /**
    * @brief Creates a directory, failing if it already exists.
    *
    * @return true on success, false on error with errno set.
    */
   bool createDirectoryExclusively(const char* pathname)
   {
#if defined(_WIN32)
      return _mkdir(pathname) == 0;
#else
      return ::mkdir(pathname, static_cast<mode_t>(0700)) == 0;
#endif
   }
} // End anonymous namespace.

ossimTempFilename::ossimTempFilename(const ossimString& tempDir,
                                     const ossimString& prefix,
                                     const ossimString& extension,
                                     bool autodelete,
                                     bool useWildcardDelete)
   :theTempDir(tempDir),
    thePrefix(prefix),
    theExtension(extension),
    theAutoDeleteFlag(autodelete),
    theWildCardDeleteFlag(useWildcardDelete)
{
}

ossimTempFilename::~ossimTempFilename()
{
   if(*((ossimFilename*)this) != "")
   {
      
      if(theAutoDeleteFlag)
      {
         if(theWildCardDeleteFlag)
         {
            wildcardRemove(*this + ".*");
         }
         else
         {
            remove();
         }
      }
   }
}

void  ossimTempFilename::generateRandomFile()
{
   generate(false);
}

void ossimTempFilename::generateRandomDir()
{
   generate(true);
}

void ossimTempFilename::generate(bool createAsDirectoryFlag)
{
   ossimString tempDirCopy = theTempDir;

   if(tempDirCopy == "")
   {
      tempDirCopy = ossimEnvironmentUtility::instance()->getEnvironmentVariable("TEMP");
      if(tempDirCopy=="")
      {
         tempDirCopy  = ossimEnvironmentUtility::instance()->getEnvironmentVariable("TMP");
      }
      if(tempDirCopy == "")
      {
         if(ossimFilename("/tmp").exists())
         {
            tempDirCopy = "/tmp";
         }
      }
   }

   // Clear the name so a failure can not be mistaken for a valid file.
   *((ossimFilename*)this) = "";

   const int MAX_ATTEMPTS = 128;
   ossimFilename prefixDir = ossimFilename(tempDirCopy);

   for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
   {
      // The name is random so that the file can not be guessed and the
      // creation below is atomic so that an existing file or a symbolic link
      // is never followed or clobbered.
      std::string name(thePrefix.c_str());
      name += toHexString(nextRandomValue());

      ossimFilename candidate = prefixDir.dirCat(ossimString(name));
      if(theExtension != "")
      {
         candidate = candidate.setExtension(theExtension);
      }

      const bool created = createAsDirectoryFlag
         ? createDirectoryExclusively(candidate.c_str())
         : createFileExclusively(candidate.c_str());

      if (created)
      {
         *((ossimFilename*)this) = candidate;
         return;
      }

      if (errno != EEXIST)
      {
         // Not a name collision: no point in retrying.
         break;
      }
   }

   ossimNotify(ossimNotifyLevel_WARN)
      << "ossimTempFilename::generate: Unable to create a unique temporary "
      << (createAsDirectoryFlag ? "directory" : "file")
      << " in <" << tempDirCopy << ">, errno: " << errno << std::endl;
}
