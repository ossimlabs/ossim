//---
//
// License: MIT
//
// Description: This class provides capabilities for keywordlists.
//
//---
// $Id$

#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimDirectory.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIoStream.h>

#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRegExp.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimXmlNode.h>
#include <ossim/base/KwlNodeJsonFormatter.h>
#include <ossim/base/KwlNodeXmlFormatter.h>

#include <algorithm>
#include <fstream>
#include <list>
#include <memory>
#include <sstream>
#include <utility>

#include <ossim/base/ossimStreamFactoryRegistry.h>

static ossimTrace traceDebug("ossimKeywordlist:debug");

#ifdef OSSIM_ID_ENABLED
static const bool TRACE = false;
static const char OSSIM_ID[] = "$Id: ossimKeywordlist.cpp 23632 2015-11-19 20:43:06Z dburken $";
#endif

const std::string ossimKeywordlist::NULL_KW = "";

ossimKeywordlist::ossimKeywordlist(const ossimKeywordlist& src)
:m_map(src.m_map),
m_delimiter(src.m_delimiter),
m_preserveKeyValues(src.m_preserveKeyValues),
m_expandEnvVars(src.m_expandEnvVars)
{
}

ossimKeywordlist::ossimKeywordlist(const std::map<std::string, std::string>& keywordMap)
:m_map(keywordMap),
m_delimiter(DEFAULT_DELIMITER),
m_preserveKeyValues(true),
m_expandEnvVars(true)
{

}

ossimKeywordlist::ossimKeywordlist(char delimiter, 
                                   bool expandEnvVars)
:
m_map(),
m_delimiter(delimiter),
m_preserveKeyValues(true),
m_expandEnvVars(expandEnvVars)
{
#ifdef OSSIM_ID_ENABLED
   if (TRACE) ossimNotify(ossimNotifyLevel_DEBUG) << OSSIM_ID << std::endl;
#endif
}

ossimKeywordlist::ossimKeywordlist(const char* file,
                                   char        delimiter,
                                   bool        ignoreBinaryChars,
                                   bool        expandEnvVars)
:
m_map(),
m_delimiter(delimiter),
m_preserveKeyValues(true),
//m_lineContinuationCharacter('\\'),
m_expandEnvVars(expandEnvVars)
{
   ossimFilename in_file(file);
   
   if (!parseFile(in_file, ignoreBinaryChars))
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
   }
}

ossimKeywordlist::ossimKeywordlist(const ossimFilename& file,
                                   char                 delimiter,
                                   bool                 ignoreBinaryChars,
                                   bool                 expandEnvVars)
:
m_map(),
m_delimiter(delimiter),
m_preserveKeyValues(true),
m_expandEnvVars(expandEnvVars)

{
   if (!parseFile(file, ignoreBinaryChars))
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
   }
}

ossimKeywordlist::~ossimKeywordlist()
{
   m_map.clear();
}

void ossimKeywordlist::setExpandEnvVarsFlag( bool flag )
{
   m_expandEnvVars = flag;
}

bool ossimKeywordlist::getExpandEnvVarsFlag( void ) const
{
   return m_expandEnvVars;
}

bool ossimKeywordlist::addFile(const char* file)
{
   ossimFilename in_file(file);
   
   return parseFile(in_file);
}

bool ossimKeywordlist::addFile(const ossimFilename& file)
{
   return parseFile(file);
}

void ossimKeywordlist::addList(const ossimKeywordlist &src, bool overwrite)
{
   KeywordMap::const_iterator i = src.m_map.begin();
   
   while (i != src.m_map.end())
   {
      addPair( (*i).first, (*i).second, overwrite );
      i++;
   }
}

void ossimKeywordlist::add(const ossimKeywordlist& kwl,
                           const char* prefix,
                           bool stripPrefix)
{
   std::map<std::string, std::string>::const_iterator iter = kwl.m_map.begin();
   
   ossimRegExp regExp;
   
   // Check for null prefix.
   std::string tmpPrefix;
   if (prefix) tmpPrefix = prefix;
   
   regExp.compile(("^("+tmpPrefix+")").c_str());
   
   while(iter != kwl.m_map.end())
   {
      ossimString newKey;
      
      if(regExp.find( (*iter).first.c_str()))
      {
         newKey = (*iter).first;
         if(stripPrefix && prefix)
         {
            newKey = newKey.substitute(prefix, "");
            
         }
         
         addPair(newKey.string(), (*iter).second, true);
      }
      ++iter;
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const ossimKeywordlist& kwl,
                           bool overwrite)
{
   std::string p = prefix ? prefix : "";
   std::map<std::string, std::string>::const_iterator iter = kwl.m_map.begin();
   while(iter != kwl.m_map.end())
   {
      std::string k( p + (*iter).first );
      addPair( k, (*iter).second, overwrite );
      ++iter;
   }
}

void ossimKeywordlist::addPair(const std::string& key,
                               const std::string& value,
                               bool               overwrite)
{
   if ( key.size() )
   {
      ossimString v = value;
      if ( m_expandEnvVars == true )
      {
         v = v.expandEnvironmentVariable();
      }
      
      KeywordMap::iterator i = getMapEntry(key);
      
      if (i == m_map.end())
      {
         m_map.insert(std::make_pair(key, v.string()));
      }
      else if (overwrite)
      {
         (*i).second = v.string();
      }
   }
}

void ossimKeywordlist::addPair(const std::string& prefix,
                               const std::string& key,
                               const std::string& value,
                               bool               overwrite)
{
   std::string k(prefix + key);
   addPair(k, value, overwrite);
}

void ossimKeywordlist::add(const char* key,
                           const char* value,
                           bool        overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v(value?value:"");
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           const char* value,
                           bool        overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v(value ? value : "");
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           char        value,
                           bool        overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v(1, value);
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           char        value,
                           bool        overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v(1, value);
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_int16 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           ossim_int16 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_uint16 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           ossim_uint16 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_int32 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char*  prefix,
                           const char*  key,
                           ossim_int32 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_uint32 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char*  prefix,
                           const char*  key,
                           ossim_uint32 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_int64 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char*  prefix,
                           const char*  key,
                           ossim_int64 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_uint64 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char*  prefix,
                           const char*  key,
                           ossim_uint64 value,
                           bool overwrite)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_float32 value,
                           bool overwrite,
                           int precision)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value, precision).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           ossim_float32 value,
                           bool overwrite,
                           int precision)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value, precision).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* key,
                           ossim_float64 value,
                           bool overwrite,
                           int precision)
{
   if ( key )
   {
      std::string k(key);
      std::string v = ossimString::toString(value, precision).string();
      addPair(k, v, overwrite);
   }
}

void ossimKeywordlist::add(const char* prefix,
                           const char* key,
                           ossim_float64 value,
                           bool overwrite,
                           int precision)
{
   if ( key )
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      std::string v = ossimString::toString(value, precision).string();
      addPair(k, v, overwrite);
   }
}

bool ossimKeywordlist::write(const char* file, 
                             const char* comment) const
{
   std::ofstream filename(file);
   if (!filename)
   {
      ossimNotify(ossimNotifyLevel_WARN)
      <<"ossimKeywordlist::write, Error opening file:  "
      << file << std::endl;
      return false;
   }
   
   if ( comment != 0 )
   {
      ossimString commentStr("// ");
      commentStr += comment;
      
      // Write out the input comment to the first line.
      filename << commentStr.c_str() << std::endl;
   }
   
   writeToStream(filename);
   
   filename.close();
   
   return true;
}

ossimString ossimKeywordlist::toString()const
{
   std::ostringstream out;
   writeToStream(out);
   
#if 0
   KeywordMap::const_iterator i;
   ossimString result;
   
   for(i = m_map.begin(); i != m_map.end(); i++)
   {
      result += (*i).first;
      result += delimiter_str().c_str();
      result += " ";
      result += (*i).second;
      result += '\n';
   }
#endif
   return ossimString(out.str());
}

void ossimKeywordlist::toString(ossimString& result)const
{
   std::ostringstream out;
   writeToStream(out);
   
   result = out.str();
#if 0
   KeywordMap::const_iterator i;
   
   for(i = m_map.begin(); i != m_map.end(); i++)
   {
      result += (*i).first;
      result += delimiter_str().c_str();
      result += " ";
      result += (*i).second;
      result += '\n';
   }
#endif
}

void ossimKeywordlist::writeToStream(std::ostream& out) const
{
   KeywordMap::const_iterator i;
   ossimString value;
   ossimRegExp reg("\n|\r| $|^ ");
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      value = (*i).second;
      if(!value.empty())
      {
         if(reg.find(value))
         {
            value = "\"\"\"" + value + "\"\"\"";
         }
      }
      //      value = value.substitute('\n', "\\\n", true);
      out << (*i).first  << delimiter_str().c_str() << "  "
      << value << std::endl;
   }
}

bool ossimKeywordlist::hasKey( const std::string& key ) const
{
   bool result = false;
   KeywordMap::const_iterator i = m_map.find(key);
   if (i != m_map.end())
   {
      result = true;
   }
   return result;
}

const std::string& ossimKeywordlist::findKey(const std::string& key) const
{
   // std::string result;
   KeywordMap::const_iterator i = m_map.find(key);
   if (i != m_map.end())
   {
      // result = (*i).second;
      return (*i).second;
   }
   return ossimKeywordlist::NULL_KW;
}

const std::string& ossimKeywordlist::findKey(const std::string& prefix,
                                             const std::string& key) const
{
   std::string k = prefix+key;
   return findKey(k);
}

const char* ossimKeywordlist::find(const char* key) const
{
   const char* result = 0;
   if (key)
   {
      std::string k = key;
      KeywordMap::const_iterator i = m_map.find( k );
      if (i != m_map.end())
      {
         result = (*i).second.c_str();
      }
   }
   return result;
}

const char* ossimKeywordlist::find(const char* prefix,
                                   const char* key) const
{
   const char* result = 0;
   if (key)
   {
      std::string k;
      if (prefix) k = prefix;
      k += key;
      KeywordMap::const_iterator i = m_map.find( k );
      if (i != m_map.end())
      {
         result = (*i).second.c_str();
      }
   }
   return result;
}

void ossimKeywordlist::remove(const char * key)
{
   ossimString k = key?key:"";
   
   KeywordMap::iterator i = m_map.find(k);
   
   if(i != m_map.end())
   {
      m_map.erase(i);
   }
}

void ossimKeywordlist::remove(const char* prefix, const char * key)
{
   if (key)
   {
      ossimString k;
      if (prefix) k = prefix;
      k += key;
      
      KeywordMap::iterator i = m_map.find(k);
      
      if(i != m_map.end())
      {
         m_map.erase(i);
      }
   }
}

ossim_uint32 ossimKeywordlist::numberOf(const char* str) const
{
   ossim_uint32 count = 0;
   
   if (str)
   {
      KeywordMap::const_iterator i = m_map.begin();
      
      while (i != m_map.end())
      {
         if ( ossimString((*i).first).contains(str) )
         {
            ++count;
         }
         ++i;
      }
   }
   
   return count;
}

ossim_uint32 ossimKeywordlist::numberOf(const char* prefix,
                                        const char* key) const
{
   if ( key ) // Must have key, sometimes no prefix.
   {
      std::string k(prefix ? (std::string(prefix)+std::string(key)) : key);
      return numberOf(k.c_str());
   }
   return 0;
}

void ossimKeywordlist::clear()
{
   m_map.clear();
}

ossimKeywordlist::KeywordMap::iterator
ossimKeywordlist::getMapEntry(const char* key)
{
   if (key)
   {
      std::string k = key;
      return m_map.find(k);
   }
   else
   {
      return m_map.end();
   }
}

ossimKeywordlist::KeywordMap::iterator
ossimKeywordlist::getMapEntry(const std::string& key)
{
   return m_map.find(key);
}

ossimKeywordlist::KeywordMap::iterator
ossimKeywordlist::getMapEntry(const ossimString& key)
{
   return m_map.find(key.string());
}


//*******************************************************************
// Private Method:
//*******************************************************************
bool ossimKeywordlist::parseFile(const ossimFilename& file,
                                 bool ignoreBinaryChars)
{
   bool result = false;

   std::shared_ptr<ossim::istream> is = ossim::StreamFactoryRegistry::instance()->
      createIstream( file.string() );
   if ( is )
   {
      m_currentlyParsing = file;
      result = parseStream(*is, ignoreBinaryChars);
      is.reset();
   }
   
   return result;
}

bool ossimKeywordlist::parseStream(ossim::istream& is, bool /* ignoreBinaryChars */)
{
   return parseStream(is);
}

bool ossimKeywordlist::parseString(const std::string& inString)
{
   ossim::istringstream in(inString);
   
   return parseStream(in);
}

bool ossimKeywordlist::isValidKeywordlistCharacter(ossim_uint8 c)const
{
   if((c>=0x20&&c<=0x7e))
   {
      return true;
   }
   switch(c)
   {
      case '\n':
      case '\r':
      case '\t':
         return true;
   }
   return false;
}

void ossimKeywordlist::skipWhitespace(ossim::istream& in)const
{
   int c = in.peek();
   while( !in.fail() &&
         ( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ) )
   {
      in.ignore(1);
      c = in.peek();
   }
}

ossimKeywordlist::KeywordlistParseState ossimKeywordlist::readComments(ossimString& sequence, ossim::istream& in)const
{
   KeywordlistParseState result = KeywordlistParseState_FAIL;
   char c = (char)in.peek();
   if(c == '/')
   {
      sequence += (char)in.get();
      c = in.peek();
      if(c == '/')
      {
         result = KeywordlistParseState_OK;
         sequence += c;
         while(!in.bad()&&!in.eof())
         {
            c = (char)in.get();
            if (in.bad() || in.eof())
               break;

            if(!isValidKeywordlistCharacter(c))
            {
               result = KeywordlistParseState_BAD_STREAM;
               break;
            }
            if((c == '\n')|| (c == '\r'))
               break;

            sequence += c;
         }
      }
   }
   return result;
}

ossimKeywordlist::KeywordlistParseState
ossimKeywordlist::readPreprocDirective(ossim::istream& in)
{
   KeywordlistParseState status = KeywordlistParseState_FAIL;

   char c = (char)in.peek();
   while (c == '#')
   {
      // Read the line as one big value:
      ossimString sequence;
      status = readValue(sequence, in);
      if (status)
         break;

      ossimString directive = sequence.before(" ");

      // Check for external KWL include file:
      if (directive == "#include")
      {
         ossimFilename includeFile = sequence.after(" ");
         if (includeFile.empty())
            break; // ignore bogus preproc line
         includeFile.trim("\"");
         includeFile.expandEnvironmentVariable();

         // The filename can be either relative to the current file being parsed or absolute:
         if (includeFile.string()[0] != '/')
            includeFile = m_currentlyParsing.path() + "/" + includeFile;

         // Save the current path in case the new one contains it's own include directive!
         ossimFilename savedCurrentPath = m_currentlyParsing;
         addFile(includeFile); // Quietly ignore any errors loading external KWL.
         m_currentlyParsing = savedCurrentPath;
      }

//      else if (directive == "#add_new_directive_here")
//      {
//         process directive
//      }

      status = KeywordlistParseState_OK;
      break;
   }
   return status;
}

ossimKeywordlist::KeywordlistParseState ossimKeywordlist::readKey(ossimString& sequence, ossim::istream& in)const
{
   KeywordlistParseState result = KeywordlistParseState_FAIL;
   if(!sequence.empty())
   {
      if(*(sequence.begin()+(sequence.size()-1)) == m_delimiter)
      {
         sequence = ossimString(sequence.begin(), sequence.begin() + (sequence.size()-1));
         return KeywordlistParseState_OK;
      }
   }
   // not a comment so read til key delimeter
   while(!in.eof() && in.good())
   {
      ossim_uint8 c = in.get();
      if( isValidKeywordlistCharacter(c) )
      {
         if ( (c == '\n') || (c == '\r') ) 
         {
            // Hit end of line with no delimiter.
            if ( in.peek() == EOF )
            {
               //---
               // Allowing on last line only.
               // Note the empty key will trigger parseStream to return true.
               //---
               sequence.clear();
               result = KeywordlistParseState_OK;
               break;
            }
            else // Line with no delimiter.
            {
               // mal formed input stream for keyword list specification
               result = KeywordlistParseState_BAD_STREAM;
               break;
            }
         }
         else if(c != m_delimiter)
         {
            sequence += (char)c;
         }
         else // at m_delimiter
         {
            result = KeywordlistParseState_OK;
            sequence = sequence.trim();
            break;
         }
      }
      else 
      {
         // mal formed input stream for keyword list specification
         result = KeywordlistParseState_BAD_STREAM;
         break;
      }
   }
   // we never found a delimeter so we are mal formed
   if(!sequence.empty()&&(result!=KeywordlistParseState_OK))
   {
      result = KeywordlistParseState_BAD_STREAM;
   }
   return result;
}

ossimKeywordlist::KeywordlistParseState ossimKeywordlist::readValue(ossimString& sequence, ossim::istream& in)const
{
   KeywordlistParseState result = KeywordlistParseState_OK;
   
   ossim_int32 quoteCount = 0; // mark as not set
   
   // make sure we check for a blank value
   while(!in.eof()&&!in.bad())
   {
      if(in.peek() == ' '||
         in.peek() == '\t')
      {
         in.ignore();
      }
      else if(in.peek() == '\n' ||
              in.peek() == '\r')
      {
         in.ignore();
         return result;
      }
      else 
      {
         break;
      }
   }
   // The ifstream object will end in '�' (character 255 or -1) if the end-of-file indicator 
   // will not be set(e.g \n). In this case, end-of-file conditions would never be detected. 
   // add EOF (which is actually the integer -1 or 255) check here.
   // Reference link http://www.cplusplus.com/forum/general/33821/
   while(!in.eof()&&!in.bad()&&in.peek()!=EOF)
   {
      ossim_uint8 c = in.get();
      if(isValidKeywordlistCharacter(c))
      {
         if(((c == '\n'||c=='\r') && !quoteCount) || in.eof())
         {
            break;
         }
         sequence += (char)c;
         if(sequence.size() >2)
         {
            if(quoteCount < 1)
            {
               //---
               // If string has leading tripple quoted bump the "quoteCount" so
               // we start skipping line breaks, preserving paragraph style strings.
               //---
               if(ossimString(sequence.begin(), sequence.begin()+3) == "\"\"\"")
               {
                  ++quoteCount;
               }
            }
            else // check for ending quotes 
            {
               if(ossimString(sequence.begin() + sequence.size()-3, sequence.end()) == "\"\"\"")
               {
                  ++quoteCount;
               }
            }
         }
         if(quoteCount > 1)
         {
            //---
            // Have leading and trailing tripple quotes. Some tiff writers, e.g. Space
            // Imaging are using four quotes.  Below code strips all quotes from each end.
            //---
            char quote = '"';
            std::string::size_type startPos = sequence.string().find_first_not_of(quote);
            std::string::size_type stopPos  = sequence.string().find_last_not_of(quote);
            if ( ( startPos != std::string::npos ) && (stopPos != std::string::npos) )
            {
               sequence = sequence.string().substr( startPos, stopPos-startPos+1 );
            }
            break;
         }
      }
      else 
      {
         result = KeywordlistParseState_BAD_STREAM;
         break;
      }
   }
   return result;
}

ossimKeywordlist::KeywordlistParseState ossimKeywordlist::readKeyAndValuePair(ossimString& key, ossimString& value, ossim::istream& in)const
{
   ossimKeywordlist::KeywordlistParseState keyState   = readKey(key, in);
   if(keyState & KeywordlistParseState_BAD_STREAM) return keyState;
   ossimKeywordlist::KeywordlistParseState valueState = readValue(value, in);
   return static_cast<ossimKeywordlist::KeywordlistParseState>( (static_cast<int>(keyState) |
                                                                 static_cast<int>(valueState)) );
}

bool ossimKeywordlist::parseStream(ossim::istream& is)
{
   if (!is) // Check stream state.
   {
      return false;
   }
   ossimString key;
   ossimString value;
   ossimString sequence;
   KeywordlistParseState state = KeywordlistParseState_OK;
   while(!is.eof() && !is.bad())
   {
      skipWhitespace(is);
      if(is.eof() || is.bad())
         return true; // we skipped to end so valid keyword list

      state = readPreprocDirective(is);
      if(state & KeywordlistParseState_BAD_STREAM)
         return false;

      // if we failed a preprocessor directive parse then try comment parse.
      if(state == KeywordlistParseState_FAIL)
      {
         state = readComments(sequence, is);
         if(state & KeywordlistParseState_BAD_STREAM)
            return false;
      }

      // if we failed a comment parse then try key value parse.
      if(state == KeywordlistParseState_FAIL)
      {
         key = sequence; // just in case there is a 1 token look ahead residual for a single slash test.
         ossimKeywordlist::KeywordlistParseState testKeyValueState = readKeyAndValuePair(key, value, is);
         if(testKeyValueState == KeywordlistParseState_OK)
         {
            key = key.trim();
            if(key.empty())
               return true;

            if ( m_expandEnvVars == true )
               value = value.expandEnvironmentVariable();
            m_map.insert(std::make_pair(key.string(), value.string()));
         }
         else if(testKeyValueState & KeywordlistParseState_BAD_STREAM)
         {
            return false;
         }
#if 0
         // Commented out to allow an invalid line in keyword list without
         // erroring out, effectively skipping bad line. drb - 01 Sep. 2001
         else
         {
            return false;
         }
#endif
      }
      else if(state & KeywordlistParseState_BAD_STREAM)
      {
         return false;
      }
      sequence = key = value = "";
   }   
   
   return true;
}

void ossimKeywordlist::getSortedList(std::vector<ossimString>& prefixValues,
                                     const ossimString &prefixKey)const
{
   ossimString regExpression     =  ossimString("^(") + prefixKey+ "[0-9]+)";
   prefixValues.clear();
   std::vector<ossimString> keys;
   getSubstringKeyList(keys, regExpression);
   ossim_uint32 nKeys = (long)keys.size();

   ossim_uint32 offset = (int)ossimString(prefixKey).size();
   ossim_uint32 idx = 0;
   std::map<ossim_int64, ossimString> numberList;
   for (idx = 0; idx < nKeys; ++idx)
   {
      ossimString numberStr(keys[idx].begin() + offset,
            keys[idx].end());
      numberList.insert(std::make_pair(numberStr.toInt64(), numberStr));
   }
   for(auto& numberKey:numberList)
   {
      prefixValues.push_back(prefixKey+numberKey.second);
   }
}

std::vector<ossimString> ossimKeywordlist::findAllKeysThatContains(const ossimString &searchString)const
{
   KeywordMap::const_iterator i;
   std::vector<ossimString> result;
   
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if( ossimString((*i).first).contains(searchString))
      {
         result.push_back((*i).first);
      }
   }
   
   return result;
}

void ossimKeywordlist::findAllKeysThatMatch( std::vector<ossimString>& result,
                                             const ossimString &regularExpression ) const
{
   KeywordMap::const_iterator i;
   ossimRegExp regExp;
   regExp.compile(regularExpression.c_str());
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if(regExp.find( (*i).first.c_str()))
      {
         result.push_back((*i).first);
      }
   }
}

ossim_uint32  ossimKeywordlist::getNumberOfKeysThatMatch(
   const ossimString &regularExpression ) const
{
   ossim_uint32 result = 0;
   KeywordMap::const_iterator i;
   ossimRegExp regExp;
   regExp.compile(regularExpression.c_str());
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if(regExp.find( (*i).first.c_str()))
      {
         ++result;
      }
   }
   return result;
}

void ossimKeywordlist::extractKeysThatMatch(ossimKeywordlist& kwl,
                                            const ossimString &regularExpression)const
{
   KeywordMap::const_iterator i;
   std::vector<ossimString> result;
   ossimRegExp regExp;
   
   regExp.compile(regularExpression.c_str());
   
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if(regExp.find( (*i).first.c_str()))
      {
         kwl.addPair((*i).first, (*i).second);
      }
   }
}

void ossimKeywordlist::removeKeysThatMatch(const ossimString &regularExpression)
{
   KeywordMap::const_iterator i;
   std::vector<ossimString> result;
   ossimRegExp regExp;
   
   regExp.compile(regularExpression.c_str());
   
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if(regExp.find( (*i).first.c_str()))
      {
         result.push_back((*i).first);
      }
   }
   for(ossim_uint32 i2 = 0; i2 < result.size(); ++i2)
   {
      remove(result[i2]);
   }
}

std::vector<ossimString> ossimKeywordlist::getSubstringKeyList(const ossimString& regularExpression)const
{
   std::vector<ossimString> result;
   getSubstringKeyList(result, regularExpression);
   return result;
}

void ossimKeywordlist::getSubstringKeyList(std::vector<ossimString>& result,
                                           const ossimString& regularExpression)const
{
   KeywordMap::const_iterator i;
   ossimRegExp regExp;
   
   regExp.compile(regularExpression.c_str());
   
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      if(regExp.find( (*i).first.c_str()))
      {
         ossimString value = ossimString((*i).first.begin()+regExp.start(),
                                         (*i).first.begin()+regExp.start()+regExp.end());
         
         if(std::find(result.begin(), result.end(), value) == result.end())
         {
            result.push_back(value);
         }
      }
   }
}

ossim_uint32 ossimKeywordlist::getNumberOfSubstringKeys(const ossimString& regularExpression)const
{
   KeywordMap::const_iterator i;
   std::vector<ossimString> currentList;
   getSubstringKeyList(currentList, regularExpression);
   return (ossim_uint32)currentList.size();
}

void ossimKeywordlist::addPrefixToAll(const ossimString& prefix)
{
   ossimKeywordlist tempKwl = *this;
   
   clear();
   
   KeywordMap::const_iterator values = tempKwl.m_map.begin();
   
   while(values != tempKwl.m_map.end())
   {
      std::string newKey = prefix.string() + (*values).first;
      addPair(newKey, (*values).second, true);
      ++values;
   }
}

void ossimKeywordlist::addPrefixToKeysThatMatch(const ossimString& prefix,
                                                const ossimString& regularExpression)
{
   ossimKeywordlist tempKwl = *this;
   
   clear();
   
   KeywordMap::const_iterator values = tempKwl.m_map.begin();
   ossimRegExp regExp;
   
   regExp.compile(regularExpression.c_str());
   
   while(values != tempKwl.m_map.end())
   {
      std::string newKey = prefix.string()+(*values).first;
      if(regExp.find( (*values).first.c_str()))
      {
         
         addPair(newKey, (*values).second, true);
      }
      else
      {
         addPair((*values).first, (*values).second, true);
      }
      ++values;
   }
}

void ossimKeywordlist::stripPrefixFromAll(const ossimString& regularExpression)
{
   ossimKeywordlist tempKwl = *this;
   
   clear();
   
   KeywordMap::const_iterator values = tempKwl.m_map.begin();
   ossimRegExp regExp;
   
   regExp.compile(regularExpression.c_str());
   
   while(values != tempKwl.m_map.end())
   {
      std::string newKey = (*values).first;
      if(regExp.find( (*values).first.c_str()))
      {
         newKey.erase(newKey.begin()+regExp.start(),
                      newKey.begin()+regExp.start()+regExp.end());
         
         addPair(newKey, (*values).second, true);
      }
      else
      {
         addPair(newKey, (*values).second, true);
      }
      ++values;
   }
}

ossim_uint32 ossimKeywordlist::getSize()const
{
   return (ossim_uint32)m_map.size();
}

const ossimKeywordlist::KeywordMap& ossimKeywordlist::getMap()const
{
   return m_map;
}

ossimKeywordlist::KeywordMap& ossimKeywordlist::getMap()
{
   return m_map;
}

void ossimKeywordlist::change_delimiter(char del)
{
   m_delimiter = del;
}

ossimString ossimKeywordlist::delimiter_str() const
{
   char tmp[2];
   tmp[0] = m_delimiter;
   tmp[1] = '\0';
   return ossimString(tmp);
}

//*******************************************************************
// Public Method:
//*******************************************************************
std::ostream& ossimKeywordlist::print(std::ostream& os) const
{
   writeToStream(os);
#if 0
   KeywordMap::const_iterator i;
   
   for(i = m_map.begin(); i != m_map.end(); ++i)
   {
      os << (*i).first << delimiter_str().c_str() << "  "
      << (*i).second << std::endl;
   }
#endif
   return os;
}

//*******************************************************************
// friend function:
//*******************************************************************
OSSIMDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                        const ossimKeywordlist& kwl)
{
   kwl.print(os);
   
   return os;
}

bool ossimKeywordlist::operator ==(ossimKeywordlist& kwl)const
{
   return (m_map == kwl.m_map);
   /*
   if(this==&kwl) return true;
   std::map<std::string, std::string>::const_iterator iter = m_map.begin();
   
   while(iter != m_map.end())
   {
      const char* value = kwl.find((*iter).first.c_str());
      
      if(ossimString(value) != (*iter).second)
      {
         return false;
      }
      ++iter;
   }
   
   return true;
   */
}

bool ossimKeywordlist::operator !=(ossimKeywordlist& kwl)const
{
   return (m_map != kwl.m_map);
}

ossimKeywordlist&  ossimKeywordlist::downcaseKeywords()
{
   KeywordMap tempMap;
   KeywordMap::iterator iter = m_map.begin();
   
   while(iter != m_map.end())
   {
      ossimString k(iter->first);
      tempMap.insert(std::make_pair(k.downcase().string(), iter->second));
      ++iter;
   }
   m_map = tempMap;
   
   return *this;
}

ossimKeywordlist& ossimKeywordlist::upcaseKeywords()
{
   KeywordMap tempMap;
   KeywordMap::iterator iter = m_map.begin();
   
   while(iter != m_map.end())
   {
      ossimString k(iter->first);
      tempMap.insert(std::make_pair(k.upcase().string(), iter->second));
      ++iter;
   }
   m_map = tempMap;
   
   return *this;
}

ossimKeywordlist& ossimKeywordlist::trimAllValues(const ossimString& valueToTrim)
{
   KeywordMap::iterator iter = m_map.begin();
   
   while(iter != m_map.end())
   {
      iter->second = ossimString(iter->second).trim(valueToTrim).string();
      ++iter;
   }
   
   return *this;
}

ossimKeywordlist ossimKeywordlist::trimAllValues(const ossimString& valueToTrim)const
{
   ossimKeywordlist result(*this);
   result.trimAllValues(valueToTrim);
   return result;
}


//*************************************************************************************************
//! [OLK, Aug/2008]
//! Sets the boolean destination arg depending on value associated with keyword for values = 
//! (yes|no|true|false|1|0). Returns TRUE if keyword found, otherwise false. Also returns false
//! if none of the above permitted values are specified (rtn_val left unchanged in this case).
//*************************************************************************************************
bool ossimKeywordlist::getBoolKeywordValue(bool& rtn_val, 
                                           const char* keyword, 
                                           const char* prefix) const
{
   bool found = true;
   const char* val_str = find(prefix, keyword);
   if (val_str) 
   {
      found = true;
      ossimString yesno (val_str);
      yesno.upcase().trim();
      if ((yesno == "YES") || (yesno == "TRUE") || (yesno == "1"))
         rtn_val = true;
      else if ((yesno == "NO") || (yesno == "FALSE") || (yesno == "0"))
         rtn_val = false;
      else
         found = false;
   }
   else
      found = false;
   
   return found;
}

bool ossimKeywordlist::isSpecialXmlCharacters(const ossimString& value)const
{
   for(ossimString::const_iterator it = value.begin(); it != value.end();++it)
   {
      switch(*it)
      {
         case '&':
         case '<':
         case '>':
         case '"':
         case '\'':
         {
            return true;
         }
         default:
         {
            break;
         }
      }
      
   }
   return false;
}

bool ossimKeywordlist::isValidTag(const std::string& value)const
{
   std::string::const_iterator textChars = value.begin();
   bool result = true;
   if(!isalpha(*(textChars) ))
   {
      result = false;
   }
   else if(!value.empty())
   {
      for(++textChars;textChars!=value.end();++textChars)
      {
         bool test = isalnum(*textChars) || 
                     (*textChars == '-') ||
                     (*textChars == '_') ||
                     (*textChars == '.');
         if (!test)
         {
            result = false;
            break;
         }
      }
   }
   else
   {
      result = false;
   }
   
   return result;
}

void ossimKeywordlist::replaceSpecialCharacters(ossimString& value)const
{
   ossimString::iterator iter = value.begin();
   
   while(iter!=value.end())
   {
      if(!(isdigit(*iter) ||
           isalpha(*iter)||
           (*iter=='/')))
      {
         *iter = '_';
      }
      ++iter;
   }
}

void ossimKeywordlist::toXML(std::ostream& out, const std::string& rootTag)const
{

   std::shared_ptr<ossim::KwlNodeXmlFormatter> formatter =
       std::make_shared<ossim::KwlNodeXmlFormatter>(*this, rootTag);
   ossim::KwlNodeFormatter *baseFormatter = formatter.get();
   ossim::KwlNodeFormatter::FormatHints hints(3, ossim::KwlNodeFormatter::FormatHints::FORMAT_HINTS_PRETTY_PRINT_FLAG);
   baseFormatter->write(out, hints);

#if 0
   std::string rootTagStr = rootTag;
   if (!isValidTag(rootTagStr))
   {
      rootTagStr = "info";
   }

   ossimRefPtr<ossimXmlNode> metadata = new ossimXmlNode;
   metadata->setTag("metadata");
   ossimKeywordlist::KeywordMap::const_iterator iter = m_map.begin();
   while(iter != m_map.end())
   {
      ossimString path = iter->first;
      bool outputValue = true;
      ossimString value = iter->second;
      if(path.contains("unformatted_tag_data"))
      {
         ossimString temp = value.trim();
         if(ossimString(temp.begin(), temp.begin()+5) == "<?xml")
         {
            value = "XML not converted";
            outputValue = false;
         }
      }
      
      if(!path.empty())
      {
         bool tagOk = true;
         path = path.substitute(".", "/", true);
         replaceSpecialCharacters(path);
         std::vector<ossimString> splitValues;
         path.split(splitValues,"/");
         if(splitValues.size())
         {
            splitValues[splitValues.size()-1] = splitValues[splitValues.size()-1].downcase();
            ossim_uint32 idx = 0;
            for(idx = 0; ((idx < splitValues.size()-1)&&tagOk);++idx)
            {
               if(!isValidTag(splitValues[idx]))
               {
                  tagOk = false;
               }
               
               splitValues[idx] = splitValues[idx].upcase();
            }
         }
         if(tagOk)
         {
            path.join(splitValues, "/");
            ossimRefPtr<ossimXmlNode> node = metadata->addNode(path.c_str(), value);
            if(isSpecialXmlCharacters(value))
            {
               node->setCDataFlag(true);
            }
         }
      }
      ++iter;
   }
   
   if( 1 ) // tmp dbr !m_includeMetadataTagName)
   {
      out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
          << "<" << rootTagStr << ">\n";
      
      const ossimXmlNode::ChildListType& children = metadata->getChildNodes();
      ossimXmlNode::ChildListType::const_iterator iter = children.begin();
      while(iter != children.end())
      {
         out << *(iter->get());
         ++iter;
      }
      out << "\n</" << rootTagStr << ">" << std::endl;
   }
   else 
   {
      out << *(metadata.get()) << std::endl;
   }  
#endif 
}

void ossimKeywordlist::toJSON(std::ostream& out, const std::string& rootTag)const
{
   std::shared_ptr<ossim::KwlNodeJsonFormatter> formatter =
       std::make_shared<ossim::KwlNodeJsonFormatter>(*this, rootTag);
   ossim::KwlNodeFormatter *baseFormatter = formatter.get();
   ossim::KwlNodeFormatter::FormatHints hints(3, ossim::KwlNodeFormatter::FormatHints::FORMAT_HINTS_PRETTY_PRINT_FLAG);
   baseFormatter->write(out, hints);

#if 0
   const std::string C   = ": "; // colon
   const std::string DQ  = "\""; // double Quote
   const std::string LB  = "{"; // left bracket
   const std::string CNL = ",\n"; // coma, new line
   const std::string NL  = "\n"; // new line   
   const std::string RB  = "}"; // left bracket
   const std::string S   = " ";  // space
   ossim_uint32 nameCount = 0;
   ossimString lastObject;
   ossim_uint32 indentCount = 3;
   ossim_uint32 indentOffset = 3;
   
   bool stringify = true;
   bool closeObject = false;
   std::vector<ossimString> objectStack;

   // Opening bracket:
   out << LB;
   
   if( rootTag.size() )
   {
      if ( stringify )
      {
         indentOffset = indentCount;
         std::string indent(indentCount, ' ');
         out << NL << indent;
      }
      else
      {
         out << NL;
      }
      // Note: not adding rootTag to object stack
      out << DQ << rootTag << DQ << C << LB << NL;
   }

   ossimKeywordlist::KeywordMap::const_iterator iter = m_map.begin();
   while(iter != m_map.end())
   {
      bool outputValue  = true;
      ossimString key   = iter->first;
      ossimString value = iter->second;
      value = value.trim(); // remove spaces

#if 0
      if(key.contains("unformatted_tag_data"))
      {
         ossimString temp = value.trim();
         if(ossimString(temp.begin(), temp.begin()+5) == "<?xml")
         {
            value = "data not converted";
            outputValue = false;
         }
      }
#endif
      
      if ( outputValue && key.size() )
      {
         std::vector<ossimString> keys;
         key.split(keys,".");
         
         if ( keys.size() )
         {
            // The last key is the name so grab it now and pop it off the stack:
            ossimString name = keys[keys.size()-1];
            keys.pop_back();

            bool sameObject = isSame( keys, objectStack );
            if ( !sameObject && keys.size() )
            {
               for ( ossim_uint32 i = 0; i < keys.size(); ++i )
               {
                  if ( i < objectStack.size() )
                  {
                     if ( keys[i] == objectStack[i] )
                     {
                        // On stack already. Nothng to do. Go to next key.
                        continue;
                     }
                     else
                     {
                        // Different object:
                        while ( i <  objectStack.size() )
                        {
                           // Write bracket, then pop:
                           if ( stringify )
                           {
                              std::string indent(indentOffset+(indentCount*objectStack.size()), ' ');
                              out << NL << indent << RB;
                           }
                           else
                           {
                              out << NL << RB;
                           }
                           objectStack.pop_back();
                           nameCount = 0;
                           closeObject = true;
                           if ( objectStack.size() )
                           {
                              lastObject = objectStack[objectStack.size()-1];
                           }
                           else
                           {
                              lastObject.clear();
                           }
                        }
                     }
                  }
                  
                  //---
                  // New object:
                  // If we had written a key:value for previos object, do a
                  // newline and zero it out.
                  //---
                  if ( nameCount )
                  {
                     out << CNL;
                     nameCount = 0;
                  }
                        
                  objectStack.push_back( keys[i] );

                  if ( closeObject )
                  {
                     out << CNL;
                     closeObject = false;
                  }

                  if ( stringify )
                  {
                     std::string indent(indentOffset+(indentCount*objectStack.size()), ' ');
                     out << indent;
                  }
                  out << DQ << keys[i] << DQ << C << LB << NL;
               }

               // Final check if keys shrunk, pop objects off the object stack.
               while ( keys.size() <  objectStack.size() )
               {
                  // Write bracket then pop:
                  if ( stringify )
                  {
                     std::string indent(indentOffset+(indentCount*objectStack.size()), ' ');
                     out << NL << indent << RB;
                  }
                  else
                  {
                     out << NL << RB;
                  }
                  objectStack.pop_back();
                  nameCount = 0;
                  closeObject = true;
                  if ( objectStack.size() )
                  {
                     lastObject = objectStack[objectStack.size()-1];
                  }
                  else
                  {
                     lastObject.clear();
                  }
               }
            }

            if ( objectStack.size() )
            {       
               if ( lastObject == objectStack[objectStack.size()-1] )
               {
                  out << CNL;
                  closeObject = false;
               }
            }
            else if ( nameCount ) // No objects loaded on the stack.
            {
               out << CNL;
            }  

            // Output "key": "value"
            if ( stringify )
            {
               std::string indent(indentOffset+indentCount*(objectStack.size()+1), ' ');
               out << indent;
            }
            out << DQ << name << DQ << C << DQ << value << DQ;

            if ( objectStack.size() )
            {
               lastObject = objectStack[objectStack.size()-1];
            }
            else
            {
               lastObject.clear();
            }
            
            ++nameCount;
         }
      }
      ++iter;
      
   } // Matches: while(iter != m_map.end())

   // Close out brackets:
   ossim_uint32 stackSize = objectStack.size();
   if ( stackSize )
   {
      for ( ossim_uint32 i = stackSize; i > 0; --i )
      {
         if ( stringify )
         {
            std::string indent(indentOffset+indentCount*i, ' ');
            out << NL << indent << RB;
         }
         else
         {
            out << NL << RB;
         }
      }
   }

   if( rootTag.size() )
   {
      if ( stringify )
      {
         std::string indent(indentCount, ' ');
         out << NL << indent << RB;
      }
      else
      {
         out << NL << RB;
      }
   }
   
   // Closing bracket, newline with flush:
   out << NL << RB << std::endl;
   #endif
}

bool ossimKeywordlist::isSame( const std::vector<ossimString>& a,
                               const std::vector<ossimString>& b ) const
{
   bool result = true;
   if ( a.size() == b.size() )
   {
      for ( ossim_uint32 i = 0; i < a.size(); ++i )
      {
         if ( a[i] != b[i] )
         {
            result = false;
            break;
         }
      }
   }
   else
   {
      result = false;
   }
   return result;
}
