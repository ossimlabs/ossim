//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/sockets/ossimToolClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <sstream>
#ifdef _MSC_VER
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace std;

#define MAX_BUF_LEN 4096
#define _DEBUG_ false

ossimToolClient::ossimToolClient()
:  m_svrsockfd(-1),
   m_buffer(new char[MAX_BUF_LEN])
{
   ossimFilename tmpdir = "/tmp";
#ifdef _MSC_VER
   if (GetTempPath(MAX_BUF_LEN, m_buffer))
      tmpdir = m_buffer;
#endif
   setProductFilePath(tmpdir);
}

ossimToolClient::~ossimToolClient()
{
   disconnect();
}

bool ossimToolClient::disconnect()
{
   bool success = true;

   if (m_svrsockfd < 0)
      return success;

   // Need to send close request to server:
   if (close(m_svrsockfd) < 0)
   {
      error("Error closing socket");
      success = false;
   }

   m_svrsockfd = -1;
   return success;
}


void ossimToolClient::error(const char *msg)
{
   perror(msg);
}


int ossimToolClient::connectToServer(char* hostname, char* portname)
{
   m_svrsockfd = -1;
   while (1)
   {
      // Consider port number in host URL:
      ossimString host (hostname);
      ossimString port;
      if (portname)
         port = portname;
      else if (host.contains(":"))
      {
         port = host.after(":");
         host = host.before(":");
      }

      if (port.empty())
      {
         // Maybe implied port then '/':
         port = host.after("/");
         port = "/" + port;
         host = host.before("/");
      }

      // Establish full server address including port:
      struct addrinfo hints;
      memset(&hints, 0, sizeof hints); // make sure the struct is empty
      hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
      hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
      hints.ai_canonname = host.stringDup();     // fill in my IP for me
      struct addrinfo *res;
      int failed = getaddrinfo(host.stringDup(), port.stringDup(), &hints, &res);
      if (failed)
      {
         error(gai_strerror(failed));
         break;
      }

      // Create socket for this server by walking the linked list of available addresses:
      struct addrinfo *server_info = res;
      while (server_info)
      {
         m_svrsockfd =
               socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
         if (m_svrsockfd >= 0)
            break;
         server_info = server_info->ai_next;
      }
      if ((m_svrsockfd < 0) || (server_info == NULL))
      {
         error("Error opening socket");
         break;
      }

      // Connect to the server:portno:
      struct sockaddr_in* serv_addr = (sockaddr_in*) server_info->ai_addr;
      serv_addr->sin_family = AF_INET;
      if (connect(m_svrsockfd,(struct sockaddr*) serv_addr, server_info->ai_addrlen) < 0)
      {
         error("ERROR connecting");
         disconnect();
      }

      break;
   }
   return m_svrsockfd;
}


bool ossimToolClient::execute(const char* command_spec)
{
   // Clear previous file path of product filename:
   if (!m_prodFilePath.isDir())
      m_prodFilePath =m_prodFilePath.path();

   if (command_spec == NULL)
      return false;

   // Write command to the OSSIM tool server socket:
   if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" sending <"<<command_spec<<">..."<<endl; //TODO REMOVE DEBUG
   int n = send(m_svrsockfd, command_spec, strlen(command_spec), 0);
   if (n < 0)
   {
      error("ERROR writing to socket");
      return false;
   }
   while (n < (int) strlen(command_spec))
   {
      // Partial send, try remainder of buffer:
      if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" sending <"<<&(command_spec[n])<<">..."<<endl; //TODO REMOVE DEBUG
      int r = send(m_svrsockfd, &(command_spec[n]), strlen(command_spec)-n, 0);
      if (r < 0)
      {
         error("ERROR writing to socket");
         return false;
      }
      n += r;
   }

   // Process response from server. First read the response type. Can be either:
   //     "TEXT"  -- only text is streamed
   //     "FILE"  -- a file is treamed
   //     "MIXED" -- Both file and text
   //     "ERROR" -- only text with error message. No product generated.
   //     "ADIOS" -- Server acknowledges disconnect
   bool success = false;
   memset(m_buffer, 0, MAX_BUF_LEN);
   if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" Waiting to recv"<<endl; //TODO REMOVE DEBUG
   n = recv(m_svrsockfd, m_buffer, 5, 0);
   if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" Received <"<<m_buffer<<">"<<endl; //TODO REMOVE DEBUG
   if (n < 0)
   {
      error("ERROR reading from socket");
      return false;
   }
   ossimString response (string(m_buffer, n));
   if (response.contains("TEXT"))
      success = receiveText();
   else if (response.contains("FILE"))
      success = receiveFile();
   else if (response.contains("MIXED"))
      success = receiveText() && receiveFile();
   else if (response.contains("ERROR"))
      receiveText(); // success = false;
   else if (response.contains("ADIOS"))
      error("Received unexpected disconnect message from server.");
   else
      error("Unknown type in response header");

   return success;
}


bool ossimToolClient::receiveText()
{
   int n = MAX_BUF_LEN;
   m_textResponse.clear();
   while (n == MAX_BUF_LEN)
   {
      memset(m_buffer, 0, MAX_BUF_LEN);
      if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" Waiting to recv"<<endl; //TODO REMOVE DEBUG
      n = recv(m_svrsockfd, m_buffer, MAX_BUF_LEN, 0);
      if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" Received <"<<m_buffer<<">"<<endl; //TODO REMOVE DEBUG
      if (n < 0)
      {
         error("ERROR reading from socket");
         return false;
      }
      m_textResponse.append(m_buffer);
   }
   if (!acknowledgeRcv())
      return false;

   printf("Text response:\n-----------------\n%s\n-----------------\n",m_textResponse.c_str());
   return true;
}


bool ossimToolClient::receiveFile()
{
   ostringstream xmsg;
   cout << "Server requesting file send. " <<endl;

   // Fetch the file size in bytes as the next message "SIZE: GGGMMMKKKBBB", (18 bytes)
   if (!receiveText())
      return false;
   int filesize = m_textResponse.after("SIZE: ").toInt();
   cout << "File size = " <<filesize<<endl;

   // Fetch the filename as next message "NAME: AAAAA...", (256 bytes)
   if (!receiveText())
      return false;
   ossimFilename fileName (m_textResponse.after("NAME: "));
   m_prodFilePath = m_prodFilePath.dirCat(fileName);
   cout << "File name = " <<m_prodFilePath<<endl;

   // Open file for writing:
   ofstream fout (m_prodFilePath.chars());
   if (fout.fail())
   {
      xmsg <<"ERROR opening output file: <"<<m_prodFilePath<<">"<<ends;
      error(xmsg.str().c_str());
      return false;
   }

   memset(m_buffer, 0, MAX_BUF_LEN);
   int n = MAX_BUF_LEN;
   int numBytes = 0;
   while ((n == MAX_BUF_LEN) && (numBytes < filesize))
   {
      n = recv(m_svrsockfd, m_buffer, MAX_BUF_LEN, 0);
      if (n < 0)
      {
         error("ERROR reading from socket");
         return false;
      }
      fout.write(m_buffer, n);
      if (fout.fail())
      {
         error("ERROR on file write().");
         return false;
      }
      numBytes += n;
   }

   if (!acknowledgeRcv())
      return false;

   cout<<"\nossim-client: Received and wrote "<<numBytes<<" bytes to: <"<<m_prodFilePath<<">."<<endl;;
   fout.close();

   return true;
}


bool ossimToolClient::acknowledgeRcv()
{
   if (_DEBUG_) cout<<"ossimToolClient:"<<__LINE__<<" sending \"ok_to_send\"..."<<endl; //TODO REMOVE DEBUG
   int r = send(m_svrsockfd, "ok_to_send", 11, 0);
   if (r < 0)
   {
      error("ERROR writing to socket");
      return false;
   }
   if (_DEBUG_) cout << "Receive acknowledged."<<endl;
   return true;
}


bool ossimToolClient::setProductFilePath(const char* filepath)
{
   if (filepath == NULL)
      m_prodFilePath = "./";
   else
      m_prodFilePath = filepath;

   ostringstream xmsg;
   if (!m_prodFilePath.exists() && !m_prodFilePath.createDirectory())
   {
      xmsg <<"ERROR creating output directory: <"<<m_prodFilePath<<">"<<ends;
      error(xmsg.str().c_str());
   }
   else if (!m_prodFilePath.isWriteable())
   {
      xmsg <<"ERROR output directory: <"<<m_prodFilePath<<"> is not writable."<<ends;
      error(xmsg.str().c_str());
   }
   return true;
}
