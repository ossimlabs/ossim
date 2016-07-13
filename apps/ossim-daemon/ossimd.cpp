//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <iostream>
#include <sstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimArgumentParser.h>

using namespace std;

#define OWARN ossimNotify(ossimNotifyLevel_WARN)
#define OINFO ossimNotify(ossimNotifyLevel_INFO)
#define MAX_BUF_LEN 4096

void sigchld_handler(int s)
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while(waitpid(-1, NULL, WNOHANG) > 0);
   errno = saved_errno;
}

bool sendImage(int clientfd, const ossimFilename& fname)
{
   ostringstream xmsg;

   // Open the server-side image file:
   ifstream svrfile (fname.chars());
   if (svrfile.fail())
   {
      xmsg<<"ossimd.sendImage() -- Error opening file <"<<fname<<">."<<endl;
      throw (ossimException(xmsg.str()));
   }

   char buffer[MAX_BUF_LEN];
   memset(buffer, 0, MAX_BUF_LEN);

   // Send image in MAX_BUF_LEN byte packets
   while (!svrfile.eof())
   {
      // Read server-side file block:
      svrfile.read(buffer, MAX_BUF_LEN);
      if (svrfile.fail())
      {
         xmsg<<"ossimd.sendImage() -- Error during file read() for <"<<fname<<">."<<endl;
         throw (ossimException(xmsg.str()));
      }

      // transmit the block to the client:
      int blocksize = svrfile.gcount();
      while (blocksize)
      {
         int n = write(clientfd, buffer, blocksize);
         if (n < 0)
         {
            xmsg<<"ossimd.sendImage() -- Error during socket write()."<<endl;
            throw (ossimException(xmsg.str()));
         }
         blocksize -= n;
      }
   }

   svrfile.close();
   return true;
}

bool runCommand(ossimString& command)
{
   bool status_ok = false;
   static const char* msg = "\nossimd -- runCommand(): ";

   ossimArgumentParser ap (command);
   ossimString util_name = ap[0];

   try
   {
      ossimUtilityFactoryBase* factory = ossimUtilityRegistry::instance();
      ossimRefPtr<ossimUtility> utility = factory->createUtility(util_name);

      if (!utility.valid())
         cout<<msg<<"Did not understand command <"<<util_name<<">"<<endl;
      else if (!utility->initialize(ap))
         cout<<msg<<"Could not execute command sequence <"<<command<<">."<<endl;
      else if (!utility->execute())
         cout<<msg<<"Error encountered executing\n    <"<<command <<">\nCheck options."<<endl;
      else
         status_ok = true;
   }
   catch (ossimException x)
   {
      cout << "ossimd -- Caught OSSIM exception: "<<x.what()<<endl;
   }

   return status_ok;
}

void error(const char* msg)
{
   perror(msg);
   exit (1);
}

void processOssimRequest(int client_fd, struct sockaddr_in& cli_addr)
{
   char buffer[MAX_BUF_LEN];
   char dst[INET6_ADDRSTRLEN];

   // TEST CODE:
   cout << "\nprocessOssimRequest() -- Process ID: "<<getpid()<<endl;
   cout <<   "                  Parent process ID: "<<getppid()<<endl;

   // Clear the input buffer and read the message sent:
   memset(buffer, 0, MAX_BUF_LEN);
   int n = recv(client_fd, buffer, MAX_BUF_LEN, 0);
   if (n < 0)
      error("ossimd: EOF encountered reading from port");

   // If message received, acknowledge back:
   if (n != 0)
   {
      // Log the message received:
      void* addr = &(cli_addr.sin_addr);
      if (!inet_ntop(AF_INET, addr, dst, INET6_ADDRSTRLEN))
         error("ossimd: Error returned from inet_ntop(). ");
      cout << "\nossimd received message from: "<<dst<<"\n---------------\n"<<buffer
            <<"\n---------------\n"<< endl;

//      dup2(client_fd, 0);
//      dup2(client_fd, 1);
//      dup2(client_fd, 2);

      // Send acknowledgement back to client:
//      n = send(client_fd,"ossimd received request",24,0);
//      if (n < 0)
//         error("ERROR writing to socket");

      // process request:
      ossimString command (std::string(buffer, strnlen(buffer, sizeof(buffer))));
      command.trim();

      runCommand(command);
   }

   // Finished serving this client. Close the connection:
   close(client_fd);
}

int main(int argc, char *argv[])
{
   socklen_t clilen;
   ostringstream xmsg;

   bool status_ok = true;

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(argc, argv);

   // Establish full server address including port:
   struct addrinfo hints;
   memset(&hints, 0, sizeof hints); // make sure the struct is empty
   hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
   hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
   struct addrinfo *res;
   int failed = getaddrinfo(NULL, "ossimd", &hints, &res);
   if (failed)
      error(gai_strerror(failed));

   // Create socket for this server by walking the linked list of available addresses:
   struct addrinfo *server_info = res;
   int svrsockfd = 0;
   while (server_info)
   {
      svrsockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
      if (svrsockfd >= 0)
         break;
      server_info = server_info->ai_next;
   }
   if ((svrsockfd < 0) || (server_info == NULL))
      error("Error opening socket");

   // lose the pesky "Address already in use" error message when requently restarting the daemon:
   int yes=1;
   if (setsockopt(svrsockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1)
      error("Error on setsockopt() call");

   // Bind the server's socket to the specified port number:
   if (bind(svrsockfd, server_info->ai_addr, server_info->ai_addrlen) < 0)
      error("Error on binding to socket:port.");

   struct sockaddr_in *server_addr = (sockaddr_in*) &(server_info->ai_addr);

   OINFO<<"ossimd daemon started. Process ID: "<<getpid()<<"\n"<<endl;
   freeaddrinfo(server_info);

   // Start listening:
   if (listen(svrsockfd, 5) == -1)
      error("Error on listen()");

   // Reap all dead processes:
   struct sigaction sa;
   sa.sa_handler = sigchld_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGCHLD, &sa, NULL) == -1)
      error("Error on sigaction()");

   // Loop forever to listen for OSSIM requests:
   OINFO<<"Waiting for connections...\n"<<endl;
   while (1)
   {
      // Message received at server socket, establish connection to client's socket:
      struct sockaddr_in cli_addr;
      clilen = sizeof(cli_addr);
      int clisockfd = accept(svrsockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (clisockfd < 0)
         error("Error accepting message on port.");

      // Test code:
      char clientname[256];
      char clientport[256];
      getnameinfo((struct sockaddr *) &cli_addr, clilen, clientname, 256, clientport, 256, 0);
      cout<<"ossimd: Got connection from  "<<clientname<<":"<<clientport
            <<" Forking child process..."<<endl;

      // Fork process to handle client request:
      //if (!fork())
      {
         // this is the child process
         //close(svrsockfd); // child doesn't need the listener

         // Receive request from client:
         processOssimRequest(clisockfd, cli_addr);

         //exit(0); // exit forked process
      }
   }

   freeaddrinfo(server_info);
   close(svrsockfd);
   return 0;
}
