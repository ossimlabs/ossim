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
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/util/ossimUtilityRegistry.h>
#include <ossim/util/ossimChipProcUtil.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimArgumentParser.h>

using namespace std;

#define OWARN ossimNotify(ossimNotifyLevel_WARN)
#define OINFO ossimNotify(ossimNotifyLevel_INFO)
#define MAX_BUF_LEN 4096

class ossimDaemon
{
public:
   ossimDaemon();
   ~ossimDaemon();
   void startListening(const char* portid);

private:
   void initSocket(const char* portid);
   bool processOssimRequest(int client_fd, struct sockaddr_in& cli_addr);
   bool runCommand(int client_fd, ossimString& command);
   void writeSocket(int clientfd, const char* buf, int bufsize);
   bool sendFile(int clientfd, const ossimFilename& fname);
   void error(const char* msg);

   static void sigchld_handler(int s);

   int svrsockfd;
   char buffer[MAX_BUF_LEN];
};

ossimDaemon::ossimDaemon()
:  svrsockfd(-1)
{}

ossimDaemon::~ossimDaemon()
{
   close(svrsockfd);
}


void ossimDaemon::startListening(const char* portid)
{
   initSocket(portid);

   socklen_t clilen;
   ostringstream xmsg;
   bool status_ok = true;

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


#if 1
      // Fork process to handle client request:
      //if (!fork())
      {
         // this is the child process
         //close(svrsockfd); // child doesn't need the listener

         // Receive request from client:
         bool connected = true;
         while (connected)
            connected = processOssimRequest(clisockfd, cli_addr);

         //exit(0); // exit forked process
      }
#else
      int n=0;
      while (1)
      {
         // Clear the input buffer and read the message sent:
         memset(buffer, 0, 256);
         n = recv(clisockfd,buffer,256,0);
         if (n < 0)
         {
            printf("recv returned -1 from port %d.\n", clisockfd);
         }

         // Send acknowledgement back to client:
         printf("Received: %s\n",buffer);
         string msg ="server received message: ";
         msg.append(buffer, strlen(buffer));
         n = send(clisockfd, msg.c_str(), msg.size(), 0);
         if (n < 0)
            error("ERROR writing to socket");
      }
#endif
      // Finished serving this client. Close the connection:
      close(clisockfd);
   }
}

void ossimDaemon::initSocket(const char* portid)
{
   // Establish full server address including port:
   struct addrinfo hints;
   memset(&hints, 0, sizeof hints); // make sure the struct is empty
   hints.ai_family = AF_INET;     // don't care IPv4 or IPv6
   hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
   hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
   struct addrinfo *res;

   int failed = getaddrinfo(NULL, portid, &hints, &res);
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

   // lose the pesky "Address already in use" error message when requently restarting the daemon:
   int yes=1;
   if (setsockopt(svrsockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1)
      error("Error on setsockopt() call");

   // Bind the server's socket to the specified port number:
   if (bind(svrsockfd, server_info->ai_addr, server_info->ai_addrlen) < 0)
      error("Error on binding to socket:port.");

   struct sockaddr_in *server_addr = (sockaddr_in*) &(server_info->ai_addr);

   OINFO<<"ossimd daemon started. Listening on port "<<portid<<". Process ID: "<<getpid()<<"\n"<<endl;
   freeaddrinfo(server_info);

   // Start listening:
   if (listen(svrsockfd, 5) == -1)
      error("Error on listen()");

   // Reap all dead processes:
   struct sigaction sa;
   sa.sa_handler = ossimDaemon::sigchld_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGCHLD, &sa, NULL) == -1)
      error("Error on sigaction()");
}

void ossimDaemon::sigchld_handler(int s)
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while(waitpid(-1, NULL, WNOHANG) > 0);
   errno = saved_errno;
}

void ossimDaemon::error(const char* msg)
{
   perror(msg);
   exit (1);
}

void ossimDaemon::writeSocket(int clientfd, const char* buf, int bufsize)
{
   int remaining = bufsize;
   int n;

   //int optval = 1;
   //setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

   while (remaining)
   {
      n = send(clientfd, buf, remaining, 0);
      if (n < 0)
         error("ERROR writing to socket");
      remaining -= n;
   }
}

bool ossimDaemon::sendFile(int clientfd, const ossimFilename& fname)
{
   ostringstream xmsg;

   // Open the server-side image file:
   ifstream svrfile (fname.chars(), ios::binary|ios::in);
   if (svrfile.fail())
   {
      xmsg<<"ossimd.sendFile() -- Error opening file <"<<fname<<">."<<endl;
      error(xmsg.str().c_str());
   }

   // Determine file size:
   std::streampos fsize = svrfile.tellg();
   svrfile.seekg( 0, std::ios::end );
   fsize = svrfile.tellg() -fsize;
   svrfile.seekg( 0, std::ios::beg );

   // Send file size to the client:
   char response[19];
   sprintf(response, "SIZE: %012d", (int) fsize);
   writeSocket(clientfd, response, strlen(response));
   cout << "Sent to client "<< strlen(response) <<" bytes: \""<<response<<"\""<<endl;

   // Client should acknowledge with "ok_to_send":
   memset(buffer, 0, MAX_BUF_LEN);
   int n = recv(clientfd, buffer, 11, 0);
   if (n < 0)
      error("ossimd: EOF encountered reading from port");
   if (strcmp(buffer, "ok_to_send"))
      return false;
   cout << "Send acknowledged by client."<<endl;

   memset(buffer, 0, MAX_BUF_LEN);

   // Send image in MAX_BUF_LEN byte packets
   n = 0;
   int r = 0;
   while (!svrfile.eof())
   {
      // Read server-side file block:
      svrfile.read(buffer, MAX_BUF_LEN);
      if (svrfile.bad())
         error("ossimd.sendFile() -- Error during file read()");

      n = svrfile.gcount();
      r += n;

      // transmit the block to the client:
      writeSocket(clientfd, buffer, n);
   }

   cout << "Send complete."<<endl;
   svrfile.close();
   return true;
}

bool ossimDaemon::runCommand(int client_fd, ossimString& command)
{
   ostringstream xmsg;
   bool status_ok = false;
   static const char* msg = "\nossimd.runCommand(): ";

   // Intercept test mode:
   if (command.contains("sendfile"))
   {
      ossimFilename fname = command.after("sendfile").trim();
      const char* response = "FILE ";
      writeSocket(client_fd, response, strlen(response));
      sendFile(client_fd, fname);
      return true;
   }

   // Redirect stdout:
   memset(buffer, 0, MAX_BUF_LEN);
   int pipeDesc[2] = {0,0};
   int savedStdout = dup( fileno(stdout) );
   if( pipe( pipeDesc ) == -1 )
      error("Could not redirect stdout (1).");
   setbuf( stdout, NULL );
   dup2( pipeDesc[1], fileno(stdout) );
   fcntl( pipeDesc[0], F_SETFL, O_NONBLOCK );

   // Fetch OSSIM utility for requested operation:
   ossimArgumentParser ap (command);
   ossimString util_name = ap[0];
   ossimUtilityFactoryBase* factory = ossimUtilityRegistry::instance();
   ossimRefPtr<ossimUtility> utility = factory->createUtility(util_name);

   try
   {
      // Perform OSSIM command execution:
      if (!utility.valid())
         cout<<msg<<"Did not understand command <"<<util_name<<">"<<endl;
      else if (!utility->initialize(ap))
         cout<<msg<<"Could not execute command sequence <"<<command<<">."<<endl;
      else if (!utility->execute())
         cout<<msg<<"Error encountered executing\n    <"<<command <<">\nCheck options."<<endl;
      else
         status_ok = true;
   }
   catch (ossimException& x)
   {
      cout << msg << "Caught OSSIM exception: "<<x.what()<<endl;
   }
   catch (exception& x)
   {
      cout << msg << "Caught unknown exception: "<<x.what()<<endl;
   }

   // Stop redirecting stdout:
   dup2( savedStdout, fileno(stdout) );

   int n = MAX_BUF_LEN;
   string full_output;
   while (n == MAX_BUF_LEN)
   {
       n = read(pipeDesc[0], buffer, MAX_BUF_LEN);
       if (n > 0)
          full_output.append(buffer, n);
   }

   if (status_ok)
   {
      if (utility->isChipProcessor())
      {
         const char* response = "FILE ";
         writeSocket(client_fd, response, strlen(response));
         ossimChipProcUtil* ocp = (ossimChipProcUtil*) utility.get();
         ossimFilename prodFilename = ocp->getProductFilename();
         sendFile(client_fd, prodFilename);
      }
      else
      {
         const char* response = "TEXT ";
         writeSocket(client_fd, response, strlen(response));
         writeSocket(client_fd, full_output.c_str(), full_output.size());
         cout << "Sent to client "<< full_output.size() <<" bytes:\n------------------\n"
               <<full_output<<"\n------------------\n"<<endl;
      }
   }
   else
   {
      const char* response = "ERROR";
      writeSocket(client_fd, response, strlen(response));
      writeSocket(client_fd, full_output.c_str(), full_output.size());
      cout << "Sending ERROR to client: <"<<full_output<<">"<<endl;
   }

   return status_ok;
}

bool ossimDaemon::processOssimRequest(int client_fd, struct sockaddr_in& cli_addr)
{
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

      // process request:
      ossimString command (std::string(buffer, strnlen(buffer, sizeof(buffer))));
      command.trim();

      if (command == "goodbye")
      {
         const char* response = "ADIOS";
         writeSocket(client_fd, response, strlen(response));
         return false;
      }

      runCommand(client_fd, command);
      return true;
   }
   return false;
}

int main(int argc, char *argv[])
{

   // Initialize ossim stuff, factories, plugin, etc.
   ossimInit::instance()->initialize(argc, argv);

   const char* DEFAULT_PORT = "ossimd";
   const char* portid = DEFAULT_PORT;
   if (argc > 1)
      portid = argv[1];

   ossimDaemon od;
   od.startListening(portid);

   return 0;
}
