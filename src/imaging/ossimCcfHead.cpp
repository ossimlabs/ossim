//---
//
// License: MIT
// 
// Author:  David Burken
//
// Description:
//
// Contains class definition for CcfHead.
// 
//---
//  $Id$

#include <ossim/imaging/ossimCcfHead.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimErrorCodes.h>
#include <ossim/base/ossimErrorContext.h>
#include <ossim/base/ossimIoStream.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimStreamFactoryRegistry.h>
#include <ossim/base/ossimTrace.h>

#include <sstream>
#include <iostream>
#include <fstream>

static ossimTrace traceDebug("ossimCcfHead:debug");

ossimCcfHead::ossimCcfHead()
   :
      m_connectionString(),
      theNumberOfBands(1),
      thePixelType(OSSIM_UCHAR),
      theFileType(),
      theVersionNumber(0),
      theLinesPerChip(0),
      theSamplesPerChip(0),
      theLineChipsPerChunk(0),
      theSampleChipsPerChunk(0),
      theRectInFullImageStartLine(0),
      theRectInFuleImageStartSample(0),
      theRectInFullImageStopLine(0),
      theRectInFuleImageStopSample(0),
      theNumberOfValidImageVertices(0),
      theValidImageVertices(1),
      theFirstBandHeaderPointer(0),
      theRadiometryString(),
      theBytesPerPixel(0),
      theBytesPerChip(0),
      theBytesPerChunk(0),
      theCompressionType(),
      theNumberOfRLevels(0),
      theOccupiedFlag(1),
      theStartOfData(1),
      theNumberOfLines(1),
      theNumberOfSamples(1),
      theChunksInLineDir(1),
      theChunksInSampleDir(1)
{}

//***************************************************************************
// Public Constructor:
//***************************************************************************
ossimCcfHead::ossimCcfHead(const char* ccf_file)
   :
      m_connectionString(ccf_file),
      theNumberOfBands(1),
      thePixelType(OSSIM_UCHAR),
      theFileType(),
      theVersionNumber(0),
      theLinesPerChip(0),
      theSamplesPerChip(0),
      theLineChipsPerChunk(0),
      theSampleChipsPerChunk(0),
      theRectInFullImageStartLine(0),
      theRectInFuleImageStartSample(0),
      theRectInFullImageStopLine(0),
      theRectInFuleImageStopSample(0),
      theNumberOfValidImageVertices(0),
      theValidImageVertices(1),
      theFirstBandHeaderPointer(0),
      theRadiometryString(""),
      theBytesPerPixel(0),
      theBytesPerChip(0),
      theBytesPerChunk(0),
      theCompressionType(),
      theNumberOfRLevels(0),
      theOccupiedFlag(1),
      theStartOfData(1),
      theNumberOfLines(1),
      theNumberOfSamples(1),
      theChunksInLineDir(1),
      theChunksInSampleDir(1)
{
   static const char MODULE[] = "ossimCcfHead::ossimCcfHead";
   
   if (!parseCcfHeader(ccf_file))
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " ERROR!"
            << "\nError initializing from ccf_file:  " << ccf_file
            << "\nReturning...\n";
      }
   }
}

ossimCcfHead::ossimCcfHead(std::shared_ptr<ossim::istream>& str, 
                           const std::string& connectionString)
{
   static const char MODULE[] = "ossimCcfHead::ossimCcfHead";
   if(!str)
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
   }
   else if (!parseCcfHeader(str, connectionString))
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " ERROR!"
            << "\nError initializing from ccf_file:  " << connectionString
            << "\nReturning...\n";
      }
   }

}
//***************************************************************************
// Destructor:
//***************************************************************************
ossimCcfHead::~ossimCcfHead()
{}

//***************************************************************************
// Public Method:
//***************************************************************************
bool ossimCcfHead::parseCcfHeader(const char* ccf_file)
{
   static const char MODULE[] = "ossimCcfHead::parseCcfHeader";
   bool result = false;

   std::string connectionString = ccf_file;
   std::shared_ptr<ossim::istream> str = ossim::StreamFactoryRegistry::instance()->
      createIstream( connectionString, std::ios_base::in|std::ios_base::binary);
   if ( str )
   {
      result = parseCcfHeader(str, connectionString);
   }
   else
   {
      theErrorStatus = ossimErrorCodes::OSSIM_ERROR;
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " ERROR!"
            << "\nCannot open file:  " << m_connectionString << "\n";
      }
   }
   return result;
}

bool ossimCcfHead::parseCcfHeader(std::shared_ptr<ossim::istream>& str, 
                                  const std::string& connectionString)
{
   static const char MODULE[] = "ossimCcfHead::parseossimCcfHeader";

   bool result = false;

   if ( str )
   {
      str->clear();
      str->seekg(0);
      
      const ossim_uint32 MAX_LEN = 256;
      char tmp[MAX_LEN];

      // Check the first string should be "CCF" else get out...
      // this might hang so I changed it to not use the
      // >> operator unless it is a ccf file.  If it is another
      // file we might not be guranteed a whitespace or \n character
      // will exist and therefore the entrie file could have
      // been read in.
      //
      char type[4];
      str->read(type, 3);
      type[3] = '\0';
      theFileType = type;
      if (theFileType == "CCF")
      {
         m_ccfStr = str;
         m_connectionString = connectionString;
   
         // Check the version number.
         (*m_ccfStr) >> tmp >> theVersionNumber;

         //---
         // Call the appropriate method for the version.  Currently only version 6
         // supported as that was all I had in-house.  Feel free to add your own
         // version reader!
         //---
         switch(theVersionNumber)
         {
            case 5:
            {
               result = parseV5CcfHeader(*m_ccfStr);
               break;
            }
            case 6:
            {
               result = parseV6CcfHeader(*m_ccfStr);
               break;
            }
            default:
            {
               m_ccfStr = 0;
               // Version type not supported...
               if (traceDebug())
               {
                  ossimNotify(ossimNotifyLevel_DEBUG)
                     << MODULE << " ERROR!"
                     << "\nUsupported version:  " << theVersionNumber
                     << "  Returning...\n";
               }
            }
            break;
         }
      }

      if ( result )
      {
         //---
         // Parse the radiometry string.  This will initialize "theNumberOfBands"
         // and the pixel type.
         //---
         parseRadString();
      }
      else
      {
         str->clear();
         str->seekg(0);
      }
   }
   
   return result;
}

//***************************************************************************
// Private Method:
//***************************************************************************
bool ossimCcfHead::parseV5CcfHeader(ossim::istream& is)
{
   static const char MODULE[] = "CcfHead::parseV5CcfHeader";

   // Check the stream.
   if ( !is )
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " Bad Stream passed to method!"
            << "\nReturning...\n";
      }
      
      return false;
   }
   
   const ossim_uint32 MAX_LEN = 256;
   char tmp[MAX_LEN];
   
   //---
   // These are all fixed/not used so just skip...
   //---
   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "ccf_maker"

   is >> tmp // "LinesPerChip"
      >> theLinesPerChip
      >> tmp // "SamplesPerChip"
      >> theSamplesPerChip
      >> tmp // "LineChipsPerChunk"
      >> theLineChipsPerChunk
      >> tmp // "SampleChipsPerChunk"   
      >> theSampleChipsPerChunk;

   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "NumberOfBands"
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "RectInFullImageSpace"

   is >> tmp // skip "StartLine"
      >> theRectInFullImageStartLine
      >> tmp // skip "StartSample"
      >> theRectInFuleImageStartSample
      >> tmp // skip "StopLine"
      >> theRectInFullImageStopLine
      >> tmp // skip "StopSample"
      >> theRectInFuleImageStopSample;

   // No valid image vertices in this version.
   theNumberOfValidImageVertices = 0;
   theValidImageVertices[0] = ossimIpt(0,0);
   
   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip blank line

   is >> tmp                         // skip "FirstBandHeaderPointer"
      >> theFirstBandHeaderPointer;  // Offset to band header.

   // Seek to the band header record.
   is.seekg(theFirstBandHeaderPointer, std::ios_base::beg);

   is.getline(tmp, MAX_LEN-1, '\n');  // skip "BAND" line
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "NextBandHeaderPointer" line
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "ChunkMapPointer" line

   is >> tmp;  // skip "Radiometry"

   is.getline(tmp, MAX_LEN-1, '\n'); // Get the radiometry string.
   const char* rad = tmp;
   while ( *rad && (*rad == ' ')) rad++;  // Eat the whitespaces...
   theRadiometryString = rad;

   is >> tmp // skip "BytesPerPixel"
      >> theBytesPerPixel
      >> tmp // skip "BytesPerChip"
      >> theBytesPerChip
      >> tmp // skip "BytesPerChunk"
      >> theBytesPerChunk
      >> tmp // skip "CompressionType"
      >> theCompressionType
      >> tmp // skip "NumberOfRLevels"
      >> theNumberOfRLevels;

   if ((ossim_uint32)theNumberOfRLevels > theOccupiedFlag.size())
   {
      // Resize all the vectors.
      theOccupiedFlag.resize(theNumberOfRLevels);
      theStartOfData.resize(theNumberOfRLevels);
      theNumberOfLines.resize(theNumberOfRLevels);
      theNumberOfSamples.resize(theNumberOfRLevels);
      theChunksInLineDir.resize(theNumberOfRLevels);
      theChunksInSampleDir.resize(theNumberOfRLevels);
   }

   is.read(tmp, 1);                  // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n'); // skip blank line
   
   ossim_uint32 i;
   for (i=0; i<theNumberOfRLevels; i++)
   {
      is.getline(tmp, MAX_LEN-1, '\n'); // skip the Rlevel line

      ossim_uint32 tmp_long;
      
      is >> tmp // skip "Occupied"
         >> tmp_long;

      theOccupiedFlag[i] = tmp_long;

      is >> tmp // skip "StartOfData"
         >> tmp_long;

      theStartOfData[i] = tmp_long;
      
      is >> tmp // skip "NumberOfLines"
         >> tmp_long;

      theNumberOfLines[i] = tmp_long;

      is >> tmp // skip "NumberOfSamples"
         >> tmp_long;

      theNumberOfSamples[i] = tmp_long;

      is >> tmp // skip "ChunksInLineDir"
         >> tmp_long;

      theChunksInLineDir[i] = tmp_long;

      is >> tmp // skip "ChunksInSampleDir"
         >> tmp_long;

      theChunksInSampleDir[i] = tmp_long;

      is.getline(tmp, MAX_LEN-1, '\n'); // skip "LineOffset" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "LineDecimation" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "SampleOffset" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "SampleDecimation" line
      is.read(tmp, 1);                  // Eat the '\n'
      is.getline(tmp, MAX_LEN-1, '\n'); // skip blank line
      is.read(tmp, 1);                  // Eat the '\n'
   }

   // Note:  The caller will close the stream.
   
   return true;
}

//***************************************************************************
// Private Method:
//***************************************************************************
bool ossimCcfHead::parseV6CcfHeader(std::istream& is)
{
   static const char MODULE[] = "CcfHead::parseV6CcfHeader";

   // Check the stream.
   if (!is)
   {
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << MODULE << " Bad Stream passed to method!"
            << "\nReturning...\n";
      }

      return false;
   }

   const ossim_uint32 MAX_LEN = 256;
   char tmp[MAX_LEN];
   
   //***
   // These are all fixed/not used so just skip...
   //***
   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "ccf_maker"

   is >> tmp // "LinesPerChip"
      >> theLinesPerChip
      >> tmp // "SamplesPerChip"
      >> theSamplesPerChip
      >> tmp // "LineChipsPerChunk"
      >> theLineChipsPerChunk
      >> tmp // "SampleChipsPerChunk"
      >> theSampleChipsPerChunk;

   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "NumberOfBands"
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "RectInFullImageSpace"

   is >> tmp // skip "StartLine"
      >> theRectInFullImageStartLine
      >> tmp // skip "StartSample"
      >> theRectInFuleImageStartSample
      >> tmp // skip "StopLine"
      >> theRectInFullImageStopLine
      >> tmp // skip "StopSample"
      >> theRectInFuleImageStopSample
      >> tmp // skip "ValidImageVertices"
      >> theNumberOfValidImageVertices;

   // Get the valid image vertices.
   ossim_uint32 i;

   if (theNumberOfValidImageVertices != theValidImageVertices.size())
   {
      theValidImageVertices.resize(theNumberOfValidImageVertices);
   }

   for (i=0; i<theNumberOfValidImageVertices; i++)
   {
      ossimString tmp_dbl;  // Can be doubles in header.
      
      is >> tmp      // skip "Vertex"
         >> tmp      // skip Vertex number
         >> tmp_dbl; // line

      theValidImageVertices[i].y = static_cast<int>(tmp_dbl.toDouble());
      tmp_dbl = "";
      is >> tmp_dbl; // sample

      theValidImageVertices[i].x = static_cast<int>(tmp_dbl.toDouble());
   }

   is.read(tmp, 1);                   // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n');  // skip blank line

   is >> tmp                         // skip "FirstBandHeaderPointer"
      >> theFirstBandHeaderPointer;  // Offset to band header.

   // Seek to the band header record.
   is.seekg(theFirstBandHeaderPointer, std::ios_base::beg);

   is.getline(tmp, MAX_LEN-1, '\n');  // skip "BAND" line
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "NextBandHeaderPointer" line
   is.getline(tmp, MAX_LEN-1, '\n');  // skip "ChunkMapPointer" line

   is >> tmp;  // skip "Radiometry"

   is.getline(tmp, MAX_LEN-1, '\n'); // Get the radiometry string.
   
   const char* rad = tmp;
   while ( *rad && (*rad == ' ')) rad++;  // Eat the whitespaces...
   theRadiometryString = rad;

   is >> tmp // skip "BytesPerPixel"
      >> theBytesPerPixel
      >> tmp // skip "BytesPerChip"
      >> theBytesPerChip
      >> tmp // skip "BytesPerChunk"
      >> theBytesPerChunk
      >> tmp // skip "CompressionType"
      >> theCompressionType
      >> tmp // skip "NumberOfRLevels"
      >> theNumberOfRLevels;

   if (theNumberOfRLevels > theOccupiedFlag.size())
   {
      // Resize all the vectors.
      theOccupiedFlag.resize(theNumberOfRLevels);
      theStartOfData.resize(theNumberOfRLevels);
      theNumberOfLines.resize(theNumberOfRLevels);
      theNumberOfSamples.resize(theNumberOfRLevels);
      theChunksInLineDir.resize(theNumberOfRLevels);
      theChunksInSampleDir.resize(theNumberOfRLevels);
   }

   is.read(tmp, 1);                  // eat the '\n'
   is.getline(tmp, MAX_LEN-1, '\n'); // skip blank line
   
   for (i=0; i<theNumberOfRLevels; i++)
   {
      is.getline(tmp, MAX_LEN-1, '\n'); // skip the Rlevel line

      ossim_uint32 tmp_long;
      
      is >> tmp // skip "Occupied"
         >> tmp_long;

      theOccupiedFlag[i] = tmp_long;

      is >> tmp // skip "StartOfData"
         >> tmp_long;

      theStartOfData[i] = tmp_long;
      
      is >> tmp // skip "NumberOfLines"
         >> tmp_long;

      theNumberOfLines[i] = tmp_long;

      is >> tmp // skip "NumberOfSamples"
         >> tmp_long;

      theNumberOfSamples[i] = tmp_long;

      is >> tmp // skip "ChunksInLineDir"
         >> tmp_long;

      theChunksInLineDir[i] = tmp_long;

      is >> tmp // skip "ChunksInSampleDir"
         >> tmp_long;

      theChunksInSampleDir[i] = tmp_long;      

      is.getline(tmp, MAX_LEN-1, '\n'); // skip "LineOffset" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "LineDecimation" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "SampleOffset" line
      is.getline(tmp, MAX_LEN-1, '\n'); // skip "SampleDecimation" line
      is.read(tmp, 1);                  // Eat the '\n'
      is.getline(tmp, MAX_LEN-1, '\n'); // skip blank line
      is.read(tmp, 1);                  // Eat the '\n'
   }

   // Note:  The caller will close the stream.
   
   return true;
}

//***************************************************************************
// Public Method:
//***************************************************************************
std::ostream& ossimCcfHead::print(std::ostream& out) const
{
   static const char MODULE[] = "ossimCcfHead::print";

   out << MODULE
       << "\ntheErrorStatus:                 " << theErrorStatus
       << "\ntheCcfFile:                     " << m_connectionString
       << "\ntheNumberOfBands:               " << theNumberOfBands
       << "\nthePixelType:                   "
       << (ossimScalarTypeLut::instance()->getEntryString(thePixelType))
       << "\ntheFileType:                    " << theFileType
       << "\ntheVersionNumber:               " << theVersionNumber
       << "\ntheLinesPerChip:                " << theLinesPerChip
       << "\ntheSamplesPerChip:              " << theSamplesPerChip
       << "\ntheLineChipsPerChunk:           " << theLineChipsPerChunk
       << "\ntheSampleChipsPerChunk:         " << theSampleChipsPerChunk
       << "\ntheRectInFullImageStartLine:    " << theRectInFullImageStartLine
       << "\ntheRectInFuleImageStartSample:  " << theRectInFuleImageStartSample
       << "\ntheRectInFullImageStopLine:     " << theRectInFullImageStopLine
       << "\ntheRectInFuleImageStopSample:   " << theRectInFuleImageStopSample
       << "\ntheNumberOfValidImageVertices:  "
       << theNumberOfValidImageVertices;
   
   ossim_uint32 i;
   
   for (i=0; i<theNumberOfValidImageVertices; i++)
   {
      out << "\ntheValidImageVertices[" << i << "]:  "
          << theValidImageVertices[i];
   }
   
   out << "\ntheFirstBandHeaderPointer:  " << theFirstBandHeaderPointer
       << "\ntheRadiometryString:        " << theRadiometryString
       << "\ntheBytesPerPixel:           " << theBytesPerPixel
       << "\ntheBytestPerChip:           " << theBytesPerChip
       << "\ntheBytesPerChunk:           " << theBytesPerChunk
       << "\ntheCompressionType:         " << theCompressionType
       << "\ntheNumberOfRLevels:         " << theNumberOfRLevels;
   
   for (i=0; i<theNumberOfRLevels; i++)
   {
      out << "\ntheOccupiedFlag[" << i << "]:       " << theOccupiedFlag[i]
          << "\ntheStartOfData[" << i << "]:        " << theStartOfData[i]
          << "\ntheNumberOfLines[" << i << "]:      " << theNumberOfLines[i]
          << "\ntheNumberOfSamples[" << i << "]:    " << theNumberOfSamples[i]
          << "\ntheChunksInLineDir[" << i << "]:    " << theChunksInLineDir[i]
          << "\ntheChunksInSampleDir[" << i << "]:  "
          << theChunksInSampleDir[i];
   }
   
   out << std::endl;

   return ossimErrorStatusInterface::print(out);
}

//***************************************************************************
// Public Method:
//***************************************************************************
ossim_uint32 ossimCcfHead::numberOfLines(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::numberOfLines";

   if (reduced_res_level > highestReducedResSet() )
   {
       ossimNotify(ossimNotifyLevel_WARN)
          << MODULE << " ERROR!"
          << "\nInvalid reduced res level:  " << reduced_res_level
          << "\nHighest reduced res level available:  "
          << highestReducedResSet() << std::endl;   
       return 0;
   }

   return theNumberOfLines[reduced_res_level];
}

//***************************************************************************
// Public Method:
//***************************************************************************
ossim_uint32 ossimCcfHead::numberOfSamples(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::numberOfSamples";

   if (reduced_res_level > highestReducedResSet() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE << " ERROR!"
         << "\nInvalid reduced res level:  " << reduced_res_level
         << "\nHighest reduced res level available:  "
         << highestReducedResSet() << std::endl;   
      return 0;
   }
   
   return theNumberOfSamples[reduced_res_level];
}

//***************************************************************************
// Public Method:
//***************************************************************************
ossim_uint32 ossimCcfHead::chunksInLineDir(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::chunksInLineDir";

   if (reduced_res_level > highestReducedResSet() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE << " ERROR!"
         << "\nInvalid reduced res level:  " << reduced_res_level
         << "\nHighest reduced res level available:  "
         << highestReducedResSet() << std::endl;   
      return 0;
   }
   
   return theChunksInLineDir[reduced_res_level];
}

//***************************************************************************
// Public Method:
//***************************************************************************
ossim_uint32 ossimCcfHead::chunksInSampleDir(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::chunksInSampleDir";

   if (reduced_res_level > highestReducedResSet() )
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE << " ERROR!"
         << "\nInvalid reduced res level:  " << reduced_res_level
         << "\nHighest reduced res level available:  "
         << highestReducedResSet() << std::endl;   
      return 0;
   }
   
   return theChunksInSampleDir[reduced_res_level];
}


//***************************************************************************
// Public Method:
//***************************************************************************
std::streampos ossimCcfHead::startOfData(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::startOfData";

   if (reduced_res_level > highestReducedResSet() )
   {
      ossimNotify(ossimNotifyLevel_WARN) << MODULE << " ERROR!"
           << "\nInvalid reduced res level:  " << reduced_res_level
           << "\nHighest reduced res level available:  "
           << highestReducedResSet() << std::endl;   
      return 0;
   }
   
   return theStartOfData[reduced_res_level];
}

//***************************************************************************
// Public Method:
//***************************************************************************
ossimIrect ossimCcfHead::imageRect(ossim_uint32 reduced_res_level) const
{
   static const char MODULE[] = "ossimCcfHead::imageRect";
   
   if (reduced_res_level > highestReducedResSet() )
   {
      ossimNotify(ossimNotifyLevel_WARN) << MODULE << " ERROR!"
           << "\nInvalid reduced res level:  " << reduced_res_level
           << "\nHighest reduced res level available:  "
           << highestReducedResSet() << std::endl;
      return ossimIrect(0,0,0,0);
   }

   return ossimIrect(0,
                     0,
                     numberOfSamples(reduced_res_level) - 1,
                     numberOfLines(reduced_res_level)   - 1);
}

//***************************************************************************
// Public Method:
//***************************************************************************
void ossimCcfHead::parseRadString()
{
   static const char MODULE[] = "ossimCcfHead::parseRadString";
   
   if (theRadiometryString.contains("RadiomNBand"))
   {
      //***
      // Complex string, must parse. Should look like:
      // "RadiomNBand (3 3 Linear8 )"
      //***
      char tmp[80];
      std::istringstream is(theRadiometryString);

      is >> tmp;               // Skip the "RadiomNBand" string.
      is.get(tmp, 3);          // Eat the space and the '('.
      is >> theNumberOfBands;  // Get the number of bands.
   }

   if (theRadiometryString.contains("8"))
   {
      thePixelType = OSSIM_UCHAR;
   }
   else if(theRadiometryString.contains("16"))
   {
      thePixelType = OSSIM_USHORT16;
   }
   else if (theRadiometryString.contains("11"))
   {
      thePixelType = OSSIM_USHORT11;
   }
   else if (theRadiometryString.contains("12"))
   {
      thePixelType = OSSIM_USHORT12;
   }
   else if (theRadiometryString.contains("13"))
   {
      thePixelType = OSSIM_USHORT13;
   }
   else if (theRadiometryString.contains("14"))
   {
      thePixelType = OSSIM_USHORT14;
   }
   else if (theRadiometryString.contains("15"))
   {
      thePixelType = OSSIM_USHORT15;
   }
   else
   {
      ossimNotify(ossimNotifyLevel_WARN)
         << MODULE << " Unknown radiometry!"
         << "\ntheRadiometryString:  " << theRadiometryString << std::endl;
   }

   if (traceDebug())
   {
       ossimNotify(ossimNotifyLevel_DEBUG)
          << MODULE
          << "\ntheRadiometryString:  " << theRadiometryString
          << "\ntheNumberOfBands:     " << theNumberOfBands
          << "\nthePixelType:         "
          << (ossimScalarTypeLut::instance()->getEntryString(thePixelType))
          << "\n";
   }
}
