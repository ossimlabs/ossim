//---
//
// License: MIT
//
// Author: Garrett Potts
//
// Description:
// 
// Class declarations for:
//
// ossimIStream
// ossimOStream
// ossimIOStream
// ossimIOMemoryStream
// ossimIMemoryStream
// ossimOMemoryStream
// ossimIOFStream
// ossimIFStream
// ossimOFStream
//
//---
// $Id$

#ifndef ossimIoStream_HEADER
#define ossimIoStream_HEADER 1

#include <ossim/base/ossimIosFwd.h>

// NOTE: All below includes will go away once deprecated code is replaced.
// drb 04 Nov. 2016


#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimStreamBase.h>
#include <ossim/base/ossimString.h>

#include <istream>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>

namespace ossim
{
   //---
   // Straight from <iosfwd> include.  ossim namespace so we have a common
   // factory stream return that can be derived from.
   //---

   /// Base class for @c char input streams.
   typedef std::basic_istream<char>          istream;

   /// Base class for @c char output streams.
   typedef std::basic_ostream<char>          ostream;
   
   /// Base class for @c char mixed input and output streams.
   typedef std::basic_iostream<char>         iostream;
   
   /// Class for @c char memory buffers.
   typedef std::basic_stringbuf<char>        stringbuf;
   
   /// Class for @c char input memory streams.
   typedef std::basic_istringstream<char>    istringstream;
   
   /// Class for @c char output memory streams.
   typedef std::basic_ostringstream<char>    ostringstream;
   
   /// Class for @c char mixed input and output memory streams.
   typedef std::basic_stringstream<char>     stringstream;
   
   /// Class for @c char file buffers.
   typedef std::basic_filebuf<char>          filebuf;
   
   /// Class for @c char input file streams.
   typedef std::basic_ifstream<char>         ifstream;
   
   /// Class for @c char output file streams.
   typedef std::basic_ofstream<char>         ofstream;
   
   /// Class for @c char mixed input and output file streams.
   typedef std::basic_fstream<char>          fstream;

} // End: namespace ossim


//---
// Depreciated:
//---
class OSSIM_DLL ossimIStream : public ossimStreamBase, public std::basic_istream<char>   
{
public:
   //ossimIStream();
   ossimIStream(std::streambuf* sb);
   virtual ~ossimIStream();
};


class OSSIM_DLL ossimOStream : public ossimStreamBase, public std::basic_ostream<char>
{
public:
   //ossimOStream();
   ossimOStream(std::streambuf* sb);   
   virtual ~ossimOStream();
};

class OSSIM_DLL ossimIOStream : public ossimStreamBase, public std::basic_iostream<char>
{
public:
   //ossimIOStream();
   ossimIOStream(std::streambuf* sb);   
   virtual ~ossimIOStream();
};

class OSSIM_DLL ossimIOMemoryStream : public ossimIOStream
{
public:
   ossimIOMemoryStream();

   virtual ~ossimIOMemoryStream();

   // ??? (drb)
   bool is_open()const;

   // ??? (drb)
   virtual void open(const char* /* protocolString */,
                     int /* openMode */);

   ossimString str();

   // ??? (drb)
   virtual void close();

   // ??? (drb) std::streamsize
   ossim_uint64 size()const;

protected:
   std::stringbuf theBuf;
};

class OSSIM_DLL ossimIMemoryStream : public ossimIStream
{
public:
   
   ossimIMemoryStream(const ossimString& inputBuf);
   
   virtual ~ossimIMemoryStream();
   
   bool is_open()const;
   
   ossim_uint64 size()const;
   
   virtual void open(const char* /* protocolString */,
                     int /* openMode */ );

   virtual void close();
   
   ossimString str();

protected:
   std::stringbuf theBuf;
   
};

class OSSIM_DLL ossimOMemoryStream : public ossimOStream
{
public:
   ossimOMemoryStream();
   virtual ~ossimOMemoryStream();   

   bool is_open()const;

   ossim_uint64 size()const;

   virtual void open(const char* /* protocolString */,
                     int /* openMode */ );

   virtual void close();

   ossimString str();

protected:
   std::stringbuf theBuf;
};

class OSSIM_DLL ossimIOFStream : public ossimStreamBase, public std::basic_fstream<char>
{
public:
   ossimIOFStream();

   ossimIOFStream(const char* name,
                  std::ios_base::openmode mode =
                  std::ios_base::in | std::ios_base::out);

   virtual ~ossimIOFStream();
};

class OSSIM_DLL ossimIFStream : public ossimStreamBase, public std::basic_ifstream<char>
{
public:
   ossimIFStream();
   
   ossimIFStream(const char* file,
                 std::ios_base::openmode mode = std::ios_base::in);

   virtual ~ossimIFStream();

};

class OSSIM_DLL ossimOFStream : public ossimStreamBase, public std::basic_ofstream<char>
{
public:
   ossimOFStream();

   ossimOFStream(const char* name,
                 std::ios_base::openmode mode =
                 std::ios_base::out|std::ios_base::trunc);

   virtual ~ossimOFStream();

};


#ifdef _MSC_VER

class ossimIFStream64 : public std::basic_ifstream<char>
{
public:
   ossimIFStream64(const char* pFilename, 
      std::ios_base::openmode mode = ios_base::in, 
      int prot = ios_base::_Openprot);

   virtual ~ossimIFStream64();
   void seekg64(off_type off, ios_base::seekdir way);

   void seekg64(streampos pos, ios_base::seekdir way);

   static void seekg64(std::istream& str, off_type off, ios_base::seekdir way);
 
   static void seekg64(std::istream& str, std::streampos pos, ios_base::seekdir way);
private:
   FILE* theFile;
};

class ossimOFStream64 : public std::basic_ofstream<char>
{
public:
   ossimOFStream64(const char* pFilename, 
                   std::ios_base::openmode mode = ios_base::out, 
                   int prot = ios_base::_Openprot);
   virtual ~ossimOFStream64();

   ossim_uint64 tellp64();
};

#else

class ossimIFStream64 : public std::basic_ifstream<char>
{
public:
   ossimIFStream64(const char* pFilename, std::ios_base::openmode mode = ios_base::in, long prot = 0666);

   virtual ~ossimIFStream64();

   void seekg64(off_type off, ios_base::seekdir way);

   static void seekg64(std::istream& str, off_type off, ios_base::seekdir way);
};

class ossimOFStream64 : public std::basic_ofstream<char>
{
public:
   ossimOFStream64(const char* pFilename, std::ios_base::openmode mode = ios_base::out, long prot = 0666);

   virtual ~ossimOFStream64();

   ossim_uint64 tellp64();
};

#endif // _MSC_VER

OSSIM_DLL void operator >> (ossimIStream& in,ossimOStream& out);
OSSIM_DLL ossimIOStream& operator >> (ossimIStream& in,ossimIOStream& out);
OSSIM_DLL void operator >> (ossimIOStream& in,ossimOStream& out);
OSSIM_DLL ossimIOStream& operator >> (ossimIOStream& in,ossimIOStream& out);
OSSIM_DLL void operator << (ossimOStream& out, ossimIStream& in);
OSSIM_DLL void operator << (ossimOStream& out, ossimIOStream& in);
OSSIM_DLL ossimIOStream& operator << (ossimIOStream& out, ossimIStream& in);
OSSIM_DLL ossimIOStream& operator << (ossimIOStream& out, ossimIOStream& in);


#endif
