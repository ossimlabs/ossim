#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <ossim/base/ossimString.h>

using namespace std;

#define MAX_BUF_LEN 4096

static char buffer [MAX_BUF_LEN];


void error(const char *msg)
{
   perror(msg);
   exit(0);
}


void connectToServer(char* hostname, char* portname, int& svrsockfd)
{
   // Establish full server address including port:
   struct addrinfo hints;
   memset(&hints, 0, sizeof hints); // make sure the struct is empty
   hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
   hints.ai_canonname = hostname;     // fill in my IP for me
   struct addrinfo *res;
   int failed = getaddrinfo(hostname, portname, &hints, &res);
   if (failed)
      error(gai_strerror(failed));

   // Create socket for this server by walking the linked list of available addresses:
   struct addrinfo *server_info = res;
   while (server_info)
   {
      svrsockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
      if (svrsockfd >= 0)
         break;
      server_info = server_info->ai_next;
   }
   if ((svrsockfd < 0) || (server_info == NULL))
      error("Error opening socket");

   // Connect to the server:portno:
   struct sockaddr_in* serv_addr = (sockaddr_in*) server_info->ai_addr;
   serv_addr->sin_family = AF_INET;
   if (connect(svrsockfd,(struct sockaddr*) serv_addr, server_info->ai_addrlen) < 0)
      error("ERROR connecting");
}


void requestOp(int svrsockfd)
{
   // Fetch message from console:
   cout<<"\nEnter OSSIM command: "<<ends;
   cin.getline(buffer, MAX_BUF_LEN);

   // Write to the socket:
   int n = send(svrsockfd, buffer, strlen(buffer), 0);
   if (n < 0)
      error("ERROR writing to socket");
   while (n < (int) strlen(buffer))
   {
      // Partial send, try remainder of buffer:
      int r = send(svrsockfd, &(buffer[n]), strlen(buffer)-n, 0);
      if (r < 0)
         error("ERROR writing to socket");
      n += r;
   }
}


void receiveText(int svrsockfd)
{
   int n = MAX_BUF_LEN;
   string result;
   while (n == MAX_BUF_LEN)
   {
      memset(buffer, 0, MAX_BUF_LEN);
      n = recv(svrsockfd, buffer, MAX_BUF_LEN, 0);
      if (n < 0)
         error("ERROR reading from socket");
      result.append(buffer);
   }

   printf("Received text:\n-----------------\n%s\n-----------------\n",result.c_str());
}


void receiveError(int svrsockfd)
{
   memset(buffer, 0, MAX_BUF_LEN);
   int n = recv(svrsockfd, buffer, MAX_BUF_LEN, 0);
   if (n < 0)
      error("ERROR reading from socket");

   printf("Received ERROR message:\n-----------------\n%s\n-----------------\n",buffer);
}


void receiveFile(int svrsockfd)
{
   string fname;
   cout << "Save product file to: " <<ends;
   cin.getline(buffer, MAX_BUF_LEN);
   fname = buffer;

   // Open file for writing:
   ofstream fout (fname.c_str());
   if (fout.fail())
      error("ERROR opening output file.");

   memset(buffer, 0, MAX_BUF_LEN);
   int n = MAX_BUF_LEN;
   int numBytes = 0;
   while (n == MAX_BUF_LEN)
   {
      n = recv(svrsockfd, buffer, MAX_BUF_LEN, 0);
      if (n < 0)
         error("ERROR reading from socket");
      fout.write(buffer, n);
      numBytes += n;
      if (fout.fail())
         error("ERROR on file write().");
   }

   cout<<"\nossim-client: Received and wrote "<<numBytes<<" bytes to file: <"<<fname<<">."<<endl;;
   fout.close();
}


int main(int argc, char *argv[])
{

   if (argc < 3)
   {
      cout<<"Usage "<<argv[0]<<" hostname port\n"<<endl;;
      exit(0);
   }

   int svrsockfd = 0;
   int n;
   connectToServer(argv[1], argv[2], svrsockfd);

   while (1)
   {
      requestOp(svrsockfd);

#if 1
      // Read response header from the socket. Can be either:
      // "response_type: text" OR "response_type: file"
      memset(buffer, 0, MAX_BUF_LEN);
      int n = recv(svrsockfd, buffer, 5, 0);
      if (n < 0)
         error("ERROR reading from socket");
      ossimString response (string(buffer, n));

      if (response.contains("TEXT"))
         receiveText(svrsockfd);
      else if (response.contains("FILE"))
         receiveFile(svrsockfd);
      else if (response.contains("ERROR"))
         receiveError(svrsockfd);
      else if (response.contains("ADIOS"))
         break;
      else
      {
         error("Unknown type in response header");
         break;
      }
#else
      n = MAX_BUF_LEN;
      while (n == MAX_BUF_LEN)
      {
         memset(buffer, 0, MAX_BUF_LEN);
         n = recv(svrsockfd, buffer, MAX_BUF_LEN, 0);
         if (n < 0)
            error("ERROR reading from socket");
         printf("%s\n",buffer);
      }
#endif
   }

   cout << "\nossim-client: Closing connection to OSSIM server."<<endl;
   close(svrsockfd);

   return 0;
}
