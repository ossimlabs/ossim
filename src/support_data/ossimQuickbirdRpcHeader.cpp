//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id: ossimQuickbirdRpcHeader.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <ossim/support_data/ossimQuickbirdRpcHeader.h>
#include <ossim/base/ossimRegExp.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <ossim/base/ossimXmlDocument.h>

using namespace std;

std::ostream& operator << (std::ostream& out,
		      const ossimQuickbirdRpcHeader& data)
{
   out << "theSatId      = " << data.theSatId << std::endl
       << "theBandId     = " << data.theBandId << std::endl
       << "theSpecId     = " << data.theSpecId << std::endl
       << "theErrBias    = " << data.theErrBias << std::endl
       << "theLineOffset = " << data.theLineOffset << std::endl
       << "theSampOffset = " << data.theSampOffset << std::endl
       << "theLatOffset  = " << data.theLatOffset << std::endl
       << "theLonOffset  = " << data.theLonOffset << std::endl
       << "theHeightOffset  = " << data.theHeightOffset << std::endl
       << "theLineScale  = " << data.theLineScale << std::endl
       << "theSampScale  = " << data.theSampScale << std::endl
       << "theLatScale  = " << data.theLatScale << std::endl
       << "theLonScale  = " << data.theLonScale << std::endl
       << "theHeightScale  = " << data.theHeightScale << std::endl;
   
   out << "lineNumCoef = " << std::endl;
   std::copy(data.theLineNumCoeff.begin(),
             data.theLineNumCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "lineDenCoef = " << std::endl;
   std::copy(data.theLineDenCoeff.begin(),
             data.theLineDenCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "sampNumCoef = " << std::endl;
   std::copy(data.theSampNumCoeff.begin(),
             data.theSampNumCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   out << "sampDenCoef = " << std::endl;
   std::copy(data.theSampDenCoeff.begin(),
             data.theSampDenCoeff.end(),
             std::ostream_iterator<double>(out, "\n"));
   
   return out;
}


ossimQuickbirdRpcHeader::ossimQuickbirdRpcHeader()
:  theErrBias(0),
   theErrRand(0),
   theLineOffset(0),
   theSampOffset(0),
   theLatOffset(0),
   theLonOffset(0),
   theHeightOffset(0),
   theLineScale(0),
   theSampScale(0),
   theLatScale(0),
   theLonScale(0),
   theHeightScale(0)
{
}

bool ossimQuickbirdRpcHeader::open(const ossimFilename& file)
{
   theFilename = file;

   if (theFilename.ext().upcase() == "XML")
      return parseXml();

   std::ifstream in(file.c_str(), std::ios::in|std::ios::binary);
   
   char test[64];
   
   in.read((char*)test, 63);
   test[63] = '\0';
   in.seekg(0);
   ossimString line = test;
   line = line.upcase();
   
   // std::cout << "HERE: parse" << std::endl;

   if(parseNameValue(line))
   {      

      theErrorStatus = ossimErrorCodes::OSSIM_OK;
      getline(in, line);
      while((in)&&(theErrorStatus == ossimErrorCodes::OSSIM_OK))
      {
         line = line.upcase();
         if(line.contains("LINENUMCOEF"))
         {
            if(!readCoeff(in, theLineNumCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("LINEDENCOEF"))
         {
            if(!readCoeff(in, theLineDenCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("SAMPNUMCOEF"))
         {
            if(!readCoeff(in, theSampNumCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(line.contains("SAMPDENCOEF"))
         {
            if(!readCoeff(in, theSampDenCoeff))
            {
               setErrorStatus();
               break;
            }
         }
         else if(!parseNameValue(line))
         {
            setErrorStatus();
            break;
         }
         getline(in,
                 line);
      }
   }
   else
   {
      setErrorStatus();
   }
   return (theErrorStatus == ossimErrorCodes::OSSIM_OK);
}

bool ossimQuickbirdRpcHeader::parseXml ()
{
   ossimXmlDocument document;
   if (!document.openFile(theFilename))
      return false;

   ossimRefPtr<ossimXmlNode> root = document.getRoot();
   ossimRefPtr<ossimXmlNode> rpcNode = root->findFirstNode("RPB");
   if (!rpcNode)
      return false;

   ossimString dataString;
   bool success = false; // Assume we're gonna screw this up...
   while (1)
   {
      theSatId = rpcNode->getChildTextValue("SATID");
      if (theSatId.empty())
         break;

      theBandId = rpcNode->getChildTextValue("BANDID");
      if (theBandId.empty())
         break;

      theSpecId = rpcNode->getChildTextValue("SPECID");
      if (theSpecId.empty())
         break;

      rpcNode = rpcNode->findFirstNode("IMAGE");
      if (!rpcNode)
         break;

      dataString = rpcNode->getChildTextValue("ERRBIAS");
      if (dataString.empty())
         break;
      theErrBias = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("ERRRAND");
      if (dataString.empty())
         break;
      theErrRand = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("LINEOFFSET");
      if (dataString.empty())
         break;
      theLineOffset = dataString.toInt();

      dataString = rpcNode->getChildTextValue("SAMPOFFSET");
      if (dataString.empty())
         break;
      theSampOffset = dataString.toInt();

      dataString = rpcNode->getChildTextValue("LATOFFSET");
      if (dataString.empty())
         break;
      theLatOffset = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("LONGOFFSET");
      if (dataString.empty())
         break;
      theLonOffset = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("HEIGHTOFFSET");
      if (dataString.empty())
         break;
      theHeightOffset = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("LINESCALE");
      if (dataString.empty())
         break;
      theLineScale = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("SAMPSCALE");
      if (dataString.empty())
         break;
      theSampScale = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("LATSCALE");
      if (dataString.empty())
         break;
      theLatScale = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("LONGSCALE");
      if (dataString.empty())
         break;
      theLonScale = dataString.toDouble();

      dataString = rpcNode->getChildTextValue("HEIGHTSCALE");
      if (dataString.empty())
         break;
      theHeightScale = dataString.toDouble();

      vector<ossimString> ln, ld, sn, sd;
      rpcNode->getChildTextValue("LINENUMCOEFList/LINENUMCOEF").split(ln, " ", true);
      rpcNode->getChildTextValue("LINEDENCOEFList/LINEDENCOEF").split(ld, " ", true);
      rpcNode->getChildTextValue("SAMPNUMCOEFList/SAMPNUMCOEF").split(sn, " ", true);
      rpcNode->getChildTextValue("SAMPDENCOEFList/SAMPDENCOEF").split(sd, " ", true);
      if ( (ln.size() != 20) || (ld.size() != 20) || (sn.size() != 20) || (sd.size() != 20))
         break;

      for (int i=0; i<20; ++i)
      {
         theLineNumCoeff.emplace_back(ln[i].toDouble());
         theLineDenCoeff.emplace_back(ld[i].toDouble());
         theSampNumCoeff.emplace_back(sn[i].toDouble());
         theSampDenCoeff.emplace_back(sd[i].toDouble());
      }
      success = true; // Well waddaya know!
   }
   return success;
}

bool ossimQuickbirdRpcHeader::readCoeff(std::istream& in,
					std::vector<double>& coeff)
{
   coeff.clear();
   bool done = false;
   ossimString line;
   while(!in.eof()&&!in.bad()&&!done)
   {
      getline(in,
	      line);
      line.trim();
      line.trim(',');
      if(line.contains(");"))
      {
         done = true;
         line.trim(';');
         line.trim(')');
      }
      coeff.push_back(line.toDouble());
   }
   return done;
}
bool ossimQuickbirdRpcHeader::isGlobal() const
{
	ossimRegExp regex("R[0-9]*C[0-9]*");

	return !regex.find(theFilename.c_str());
}

bool ossimQuickbirdRpcHeader::parseNameValue(const ossimString& line)
{
   bool result = true;
   ossimString lineCopy = line;
   
   if(lineCopy.contains("SATID"))
   {
      theSatId = lineCopy.after("\"");
      theSatId = theSatId.before("\"");
   }
   else if(lineCopy.contains("BANDID"))
   {
      theBandId = lineCopy.after("\"");
      theBandId = theBandId.before("\"");
   }
   else if(lineCopy.contains("SPECID"))
   {
      theSpecId = lineCopy.after("\"");
      theSpecId = theSpecId.before("\"");
   }
   else if(lineCopy.contains("BEGIN_GROUP"))
   {
   }
   else if(lineCopy.contains("ERRBIAS"))
   {
      lineCopy = lineCopy.after("=");
      theErrBias = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("ERRRAND"))
   {
      lineCopy = lineCopy.after("=");
      theErrRand = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LINEOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLineOffset = lineCopy.before(";").toInt();
   }
   else if(lineCopy.contains("SAMPOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theSampOffset = lineCopy.before(";").toInt();
   }
   else if(lineCopy.contains("LATOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLatOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LONGOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theLonOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("HEIGHTOFFSET"))
   {
      lineCopy = lineCopy.after("=");
      theHeightOffset = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LINESCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLineScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("SAMPSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theSampScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LATSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLatScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("LONGSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theLonScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("HEIGHTSCALE"))
   {
      lineCopy = lineCopy.after("=");
      theHeightScale = lineCopy.before(";").toDouble();
   }
   else if(lineCopy.contains("END_GROUP"))
   {
   }
   else if(lineCopy.contains("END"))
   {
   }
   // SkySat or BlackSky images - START
   else if(lineCopy.contains("HEIGHT_OFF"))
   {
      lineCopy = lineCopy.after(":");
      theHeightOffset = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("HEIGHT_SCALE"))
   {
      lineCopy = lineCopy.after(":");
      theHeightScale = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LAT_OFF"))
   {
      lineCopy = lineCopy.after(":");
      theLatOffset = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LAT_SCALE"))
   {
      lineCopy = lineCopy.after(":");
      theLatScale = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LINE_OFF"))
   {
      lineCopy = lineCopy.after(":");
      theLineOffset = lineCopy.before("\n").toInt();
   }
   else if(lineCopy.contains("LINE_SCALE"))
   {
      lineCopy = lineCopy.after(":");
      theLineScale = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LONG_OFF"))
   {
      lineCopy = lineCopy.after(":");
      theLonOffset = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LONG_SCALE"))
   {
      lineCopy = lineCopy.after(":");
      theLonScale = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("SAMP_OFF"))
   {
      lineCopy = lineCopy.after(":");
      theSampOffset = lineCopy.before("\n").toInt();
   }
   else if(lineCopy.contains("SAMP_SCALE"))
   {
      lineCopy = lineCopy.after(":");
      theSampScale = lineCopy.before("\n").toDouble();
   }
   else if(lineCopy.contains("LINE_DEN_COEFF"))
   {
      ossimString label = lineCopy.before(":");
      int index = label.explode("_").back().toInt() - 1;
      double coeff = lineCopy.after(":").before("\n").toDouble();

      // std::cout << "HERE: " << label.substr(0, label.find_last_of('_')) << "(" << index << ")" << "=" << coeff << std::endl;

      //theLineDenCoeff[index] = coeff;
      theLineDenCoeff.push_back(coeff);

   }
   else if(lineCopy.contains("LINE_NUM_COEFF"))
   {
      ossimString label = lineCopy.before(":");
      int index = label.explode("_").back().toInt() - 1;
      double coeff = lineCopy.after(":").before("\n").toDouble();

      // std::cout << "HERE: " << label.substr(0, label.find_last_of('_')) << "(" << index << ")" << "=" << coeff << std::endl;

      // theLineNumCoeff[index] = coeff;
      theLineNumCoeff.push_back(coeff);
   }
   else if(lineCopy.contains("SAMP_DEN_COEFF"))
   {
      ossimString label = lineCopy.before(":");
      int index = label.explode("_").back().toInt() - 1;
      double coeff = lineCopy.after(":").before("\n").toDouble();

      // std::cout << "HERE: " << label.substr(0, label.find_last_of('_')) << "(" << index << ")" << "=" << coeff << std::endl;

      // theSampDenCoeff[index] = coeff;
      theSampDenCoeff.push_back(coeff);
   }
   else if(lineCopy.contains("SAMP_NUM_COEFF"))
   {
      ossimString label = lineCopy.before(":");
      int index = label.explode("_").back().toInt() - 1;
      double coeff = lineCopy.after(":").before("\n").toDouble();

      // std::cout << "HERE: " << label.substr(0, label.find_last_of('_')) << "(" << index << ")" << "=" << coeff << std::endl;

      // theSampNumCoeff[index] = coeff;
      theSampNumCoeff.push_back(coeff);
   }
   // SkySat or BlackSky images - END
   else
   {
      result = false;
   }

   return result;
}
