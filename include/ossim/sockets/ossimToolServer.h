//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#ifndef ossimToolServer_HEADER
#define ossimToolServer_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFilename.h>

/**
 * Utility class provides the server interface to ossimTool-derived functionality via TCP sockets
 * Results are returned either as streamed text (for non-image responses such as image info) or
 * streamed binary file representing imagery or vector products. Clients interfacing to this class
 * should know the commands available (or execute the command "help" and view the text response).
 * @see ossimToolClient for concrete client implementation.
 */
class OSSIM_DLL ossimToolServer
{
public:
   ossimToolServer();
   ~ossimToolServer();
   void startListening(const char* portid);

private:
   void initSocket(const char* portid);
   bool processOssimRequest(struct sockaddr_in& cli_addr);
   bool runCommand(ossimString& command);
   void writeSocket(const char* buf, int bufsize);
   bool sendFile(const ossimFilename& fname);
   void error(const char* msg);
   bool acknowledgeRcvd();

   static void sigchld_handler(int s);

   int m_svrsockfd;
   int m_clisockfd;
   char* m_buffer;
};


#endif
