//---
// License: MIT
// Description: Forward ossim stream declarations.
// $Id$
//---

#ifndef ossimIosFwd_H
#define ossimIosFwd_H 1

#include <iosfwd>

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

#endif /* #ifndef ossimIosFwd_H */
