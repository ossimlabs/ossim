//*****************************************************************************
// FILE: ossimElevCellHandler.cc
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains implementation of class ossimElevCellHandler. This is the base
//   class for all DEM file readers including DTED. Each file shall have its
//   corresponding ossimElevCellHandler. When more than one file is accessed,
//   an ossimElevCombiner is used which owns multiple instances of this class.
//
// SOFTWARE HISTORY:
//>
//   19Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
// $Id: ossimElevCellHandler.cpp 23117 2015-01-29 22:33:13Z okramer $

#include <ossim/elevation/ossimElevCellHandler.h>
#include <ossim/base/ossimKeyword.h>
#include <ossim/base/ossimGpt.h>

RTTI_DEF1(ossimElevCellHandler, "ossimElevCellHandler" , ossimElevSource)

//***
// Define Trace flags for use within this file:
//***
#include <ossim/base/ossimTrace.h>
static ossimTrace traceExec  ("ossimElevCellHandler:exec");
static ossimTrace traceDebug ("ossimElevCellHandler:debug");

static const ossimKeyword DEM_FILENAME_KW ("dem_filename",
                                           "Name of DEM file to load.");
static const ossimIpt ZERO_SIZE_IPT (0, 0);

using namespace std;

ossimElevCellHandler::ossimElevCellHandler ()
   : ossimElevSource(),
     theFilename(),
     theMeanSpacing(0.0),
     theAbsLE90(0.0),
     theAbsCE90(0.0)
{
}

ossimElevCellHandler::ossimElevCellHandler (const ossimElevCellHandler& src)
   : ossimElevSource(src),
     theFilename    (src.theFilename),
     theMeanSpacing (src.theMeanSpacing),
     theAbsLE90     (src.theAbsLE90),
     theAbsCE90     (src.theAbsCE90)
{
}

ossimElevCellHandler::ossimElevCellHandler(const char* filename)
   : ossimElevSource(),
     theFilename(filename),
     theMeanSpacing(0.0),
     theAbsLE90(0.0),
     theAbsCE90(0.0)
{}

ossimElevCellHandler::~ossimElevCellHandler()
{}

const ossimFilename& ossimElevCellHandler::getFilename() const
{
   return theFilename;
}

double ossimElevCellHandler::getMeanSpacingMeters() const
{
   return theMeanSpacing;
}

bool ossimElevCellHandler::getAccuracyInfo(ossimElevationAccuracyInfo& info,
                                           const ossimGpt& /* gpt*/ ) const
{
  info.m_confidenceLevel = .9;
  info.m_absoluteLE = theAbsLE90;
  info.m_absoluteCE = theAbsCE90;

  return info.hasValidAbsoluteError();
}

bool ossimElevCellHandler::canConnectMyInputTo(
   ossim_int32 /* inputIndex */,
   const ossimConnectableObject* /* object */) const
{         
   return false;
}

std::ostream& ossimElevCellHandler::print(ostream& out) const
{
   out << "\nossimElevCellHandler @ "<< (ossim_uint64) this
         << "\n theFilename = "<< theFilename
         << "\n theMeanSpacing = "<< theMeanSpacing
         << "\n theAbsLE90 = "<< theAbsLE90
         << "\n theAbsCE90 = "<< theAbsCE90;
         //<< ossimElevSource::print(out);
   return out;
}

