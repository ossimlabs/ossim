#include <ossim/base/ossimHttpResponse.h>

RTTI_DEF1(ossimHttpResponse, "ossimHttpResponse", ossimWebResponse);

void ossimHttpResponse::convertHeaderStreamToKeywordlist()
{
   m_headerKwl.clear();
   ossimByteStreamBuffer buf(m_headerBuffer);
   
   std::istream in(&buf);
   in.seekg(0); // make sure we are at the begining
   // skip method type
   //
   std::string statusLine;
   std::getline(in, statusLine);
   m_statusLine = statusLine;
   
   m_headerKwl.parseStream(in);
   std::vector<ossimString> statusLineArray;
   m_statusLine.split(statusLineArray, " ");
   if(statusLineArray.size() > 1)
   {
      m_statusCode = statusLineArray[1].toUInt32();
   }
   else
   {
      m_statusCode = 0;
   }
}

ossimString ossimHttpResponse::getHeaderValue(const ossimString& headerName)const
{
   ossimString result = m_headerKwl.find(headerName);

   return result;
}

ossim_int64 ossimHttpResponse::getContentLength()const
{
   ossim_float64 result = -1;
   ossimString contentLength = m_headerKwl.find("Content-Length");
   
   if(!contentLength.empty())
   {
      result = contentLength.toInt64();
   }

   return result;
}
