//**************************************************************************************************
//
//     OSSIM Open Source Geospatial Data Processing Library
//     See top level LICENSE.txt file for license information
//
//**************************************************************************************************

#include <ossim/sockets/ossimToolServer.h>
#include <iostream>
#include <sstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ossim/init/ossimInit.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimException.h>
#include <ossim/base/ossimNotify.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/util/ossimChipProcTool.h>
#include <ossim/util/ossimToolRegistry.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <process.h> /* for getpid() and the exec..() family */
#include <io.h>
#define dup2 _dup2
#define close closesocket
#define pipe(phandles)  _pipe(phandles, 4096, _O_BINARY)
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#endif


#define OWARN ossimNotify(ossimNotifyLevel_WARN)
#define OINFO ossimNotify(ossimNotifyLevel_INFO)
#define MAX_BUF_LEN 4096
#define FORK_PROCESS false
#define _DEBUG_ false

ossimToolServer::ossimToolServer()
:  m_svrsockfd(-1),
   m_clisockfd(-1),
   m_buffer(new char[MAX_BUF_LEN])
{}

ossimToolServer::~ossimToolServer()
{
   close(m_svrsockfd);
}


void ossimToolServer::startListening(const char* portid)
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
      m_clisockfd = accept(m_svrsockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (m_clisockfd < 0)
         error("Error accepting message on port.");

      // Test code:
      char clientname[256];
      char clientport[256];
      getnameinfo((struct sockaddr *) &cli_addr, clilen, clientname, 256, clientport, 256, 0);
      cout<<"ossimToolServer: Got connection from  "<<clientname<<":"<<clientport
            <<" Forking child process..."<<endl;


#if 1
      // Fork process to handle client request:
      if (FORK_PROCESS)
      {
         if (!fork())
         {
            // this is the child process
            close(m_svrsockfd); // child doesn't need the listener

            // Receive request from client:
            bool connected = true;
            while (connected)
               connected = processOssimRequest(cli_addr);

            exit(0); // exit forked process
         }
      }
      else
      {
         bool connected = true;
         while (connected)
            connected = processOssimRequest(cli_addr);
      }
#else
      int n=0;
      while (1)
      {
         // Clear the input m_buffer and read the message sent:
         memset(m_buffer, 0, 256);
         n = recv(m_clisockfd,m_buffer,256,0);
         if (n < 0)
         {
            printf("recv returned -1 from port %d.\n", m_clisockfd);
         }

         // Send acknowledgement back to client:
         printf("Received: %s\n",m_buffer);
         string msg ="server received message: ";
         msg.append(m_buffer, strlen(m_buffer));
         n = send(m_clisockfd, msg.c_str(), msg.size(), 0);
         if (n < 0)
            error("ERROR writing to socket");
      }
#endif
      // Finished serving this client. Close the connection:
      close(m_clisockfd);
   }
}

void ossimToolServer::initSocket(const char* portid)
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
      m_svrsockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
      if (m_svrsockfd >= 0)
         break;
      server_info = server_info->ai_next;
   }
   if ((m_svrsockfd < 0) || (server_info == NULL))
      error("Error opening socket");

   // lose the pesky "Address already in use" error message when requently restarting the daemon:
   int yes=1;
   if (setsockopt(m_svrsockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1)
      error("Error on setsockopt() call");

#ifdef __APPLE__
   int bindResult = ::bind(m_svrsockfd, server_info->ai_addr, server_info->ai_addrlen);
#else
   int bindResult = bind(m_svrsockfd, server_info->ai_addr, server_info->ai_addrlen);
#endif
   // Bind the server's socket to the specified port number:
   if ( bindResult < 0)
      error("Error on binding to socket:port.");

   struct sockaddr_in *server_addr = (sockaddr_in*) &(server_info->ai_addr);

   OINFO<<"ossimToolServer daemon started. Listening on port "<<portid<<". Process ID: "<<getpid()<<"\n"<<endl;
   freeaddrinfo(server_info);

   // Start listening:
   if (listen(m_svrsockfd, 5) == -1)
      error("Error on listen()");

   // Reap all dead processes:
   struct sigaction sa;
   sa.sa_handler = ossimToolServer::sigchld_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGCHLD, &sa, NULL) == -1)
      error("Error on sigaction()");
}

void ossimToolServer::sigchld_handler(int s)
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while(waitpid(-1, NULL, WNOHANG) > 0);
   errno = saved_errno;
}

void ossimToolServer::error(const char* msg)
{
   perror(msg);
   exit (1);
}

void ossimToolServer::writeSocket(const char* buf, int bufsize)
{
   int remaining = bufsize;
   int n;

   //int optval = 1;
   //setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

   while (remaining)
   {
      n = send(m_clisockfd, buf, remaining, 0);
      if (n < 0)
         error("ERROR writing to socket");
      remaining -= n;
   }
}

bool ossimToolServer::sendFile(const ossimFilename& fname)
{
   ostringstream xmsg;

   // Open the server-side image file:
   ifstream svrfile (fname.chars(), ios::binary|ios::in);
   if (svrfile.fail())
   {
      xmsg<<"ossimToolServer.sendFile() -- Error opening file <"<<fname<<">."<<endl;
      error(xmsg.str().c_str());
   }

   // Determine file size:
   std::streampos fsize = svrfile.tellg();
   svrfile.seekg( 0, std::ios::end );
   fsize = svrfile.tellg() -fsize;
   svrfile.seekg( 0, std::ios::beg );

   // Send file size to the client:
   char size_response[19];
   sprintf(size_response, "SIZE: %012d", (int) fsize);
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" sending <"<<size_response<<">"<<endl; //TODO REMOVE DEBUG
   writeSocket(size_response, strlen(size_response));
   if (!acknowledgeRcvd())
      return false;

   // Send file name to the client:
   char name_response[256];
   memset(name_response, 0, 256);
   sprintf(name_response, "NAME: %s", fname.file().chars());
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" sending <"<<name_response<<">"<<endl; //TODO REMOVE DEBUG
   writeSocket(name_response, strlen(name_response));
   if (!acknowledgeRcvd())
      return false;

   memset(m_buffer, 0, MAX_BUF_LEN);

   // Send image in MAX_BUF_LEN byte packets
   int n = 0;
   int r = 0;
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" sending binary data..."<<endl; //TODO REMOVE DEBUG
   while (!svrfile.eof())
   {
      // Read server-side file block:
      svrfile.read(m_buffer, MAX_BUF_LEN);
      if (svrfile.bad())
         error("ossimToolServer.sendFile() -- Error during file read()");

      n = svrfile.gcount();
      r += n;

      // transmit the block to the client:
      writeSocket(m_buffer, n);
   }
   if (!acknowledgeRcvd())
      return false;

   cout << "Send complete."<<endl;
   svrfile.close();
   return true;
}
bool ossimToolServer::acknowledgeRcvd()
{
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" Waiting to recv"<<endl; //TODO REMOVE DEBUG
   int n = recv(m_clisockfd, m_buffer, 11, 0);
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" Received <"<<m_buffer<<">"<<endl; //TODO REMOVE DEBUG
   if (n < 0)
      error("ossimToolServer: EOF encountered reading from port");
   if (strcmp(m_buffer, "ok_to_send"))
      return false;
   if (_DEBUG_) cout << "Send acknowledged by client."<<endl;
   return true;
}

bool ossimToolServer::runCommand(ossimString& command)
{
   ostringstream xmsg;
   bool status_ok = false;
   static const char* msg = "\nossimToolServer.runCommand(): ";

   // Intercept test mode:
   if (command == "sendfile")
   {
      ossimFilename fname = command.after("sendfile").trim();
      const char* response = "FILE ";
      writeSocket(response, strlen(response));
      sendFile(fname);
      return true;
   }

   // Redirect stdout:
   memset(m_buffer, 0, MAX_BUF_LEN);
   int pipeDesc[2] = {0,0};
   int savedStdout = dup( fileno(stdout) );
   if( pipe( pipeDesc ) == -1 )
      error("Could not redirect stdout (1).");
   setbuf( stdout, NULL );
   dup2( pipeDesc[1], fileno(stdout) );

#ifdef _MSC_VER
   u_long iMode = 1;
   ioctlsocket(pipeDesc[0], FIONBIO, &iMode);
#else
   fcntl( pipeDesc[0], F_SETFL, O_NONBLOCK );
#endif

   ossimToolFactoryBase* factory = ossimToolRegistry::instance();
   ossimRefPtr<ossimTool> utility = 0;

   // Intercept help request:
   ossimString c1 = command.before(" ");
   ossimString c2 = command.after(" ");
   while (1)
   {
      if (c1 == "help")
      {
         if (!c2.empty())
         {
            command = c2 + " --help";
         }
         else
         {
            map<string, string> capabilities;
            factory->getCapabilities(capabilities);
            map<string, string>::iterator iter = capabilities.begin();
            cout<<"\nAvailable commands:\n"<<endl;
            for (;iter != capabilities.end(); ++iter)
               cout<<"  "<<iter->first<<" -- "<<iter->second<<endl;
            cout<<"\nUse option \"--help\" with above commands to get detailed tool command."<<endl;
            status_ok = true;
            break;
         }
      }
      // Fetch OSSIM utility for requested operation:
      ossimArgumentParser ap (command);
      ossimString util_name = ap[0];
      utility = factory->createTool(util_name);

      try
      {
         // Perform OSSIM command execution:
         if (!utility.valid())
            cout<<msg<<"Did not understand command <"<<util_name<<">"<<endl;
         else if (!utility->initialize(ap))
            cout<<msg<<"Could not execute command sequence <"<<command<<">."<<endl;
         else if (!utility->helpRequested() && !utility->execute())
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

      break;
   }

   // Stop redirecting stdout and copy the output stream buffer to local memory:
   dup2( savedStdout, fileno(stdout) );
   int n = MAX_BUF_LEN;
   string full_output;
   while (n == MAX_BUF_LEN)
   {
       n = read(pipeDesc[0], m_buffer, MAX_BUF_LEN);
       if (n > 0)
          full_output.append(m_buffer, n);
   }

   if (status_ok)
   {
      if (utility.valid() && !utility->helpRequested() && utility->isChipProcessor())
      {
         const char* response = "FILE ";
         writeSocket(response, strlen(response));
         ossimChipProcTool* ocp = (ossimChipProcTool*) utility.get();
         ossimFilename prodFilename = ocp->getProductFilename();
         sendFile(prodFilename);
      }
      else
      {
         const char* response = "TEXT ";
         writeSocket(response, strlen(response));
         writeSocket(full_output.c_str(), full_output.size());
         if (!acknowledgeRcvd())
            error("ERROR receiving acknowledge from client.");
      }
   }
   else
   {
      const char* response = "ERROR";
      writeSocket(response, strlen(response));
      writeSocket(full_output.c_str(), full_output.size());
      cout << "Sending ERROR to client and closing connection: <"<<full_output<<">"<<endl;
      close(m_clisockfd);
   }

   return status_ok;
}

bool ossimToolServer::processOssimRequest(struct sockaddr_in& cli_addr)
{
   char dst[INET6_ADDRSTRLEN];

   // TEST CODE:
   cout << "\nprocessOssimRequest() -- Process ID: "<<getpid()<<endl;
   cout <<   "                  Parent process ID: "<<getppid()<<endl;

   // Clear the input m_buffer and read the message sent:
   memset(m_buffer, 0, MAX_BUF_LEN);
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" Waiting to recv"<<endl; //TODO REMOVE DEBUG
   int n = recv(m_clisockfd, m_buffer, MAX_BUF_LEN, 0);
   if (_DEBUG_) cout<<"ossimToolServer:"<<__LINE__<<" Received <"<<m_buffer<<">"<<endl; //TODO REMOVE DEBUG
   if (n < 0)
      error("ossimToolServer: EOF encountered reading from port");

   // If message received, acknowledge back:
   if (n != 0)
   {
      // Log the message received:
      void* addr = &(cli_addr.sin_addr);
      if (!inet_ntop(AF_INET, addr, dst, INET6_ADDRSTRLEN))
         error("ossimToolServer: Error returned from inet_ntop(). ");
      cout << "\nossimToolServer: received message from: "<<dst<<"\n---------------\n"<<m_buffer
            <<"\n---------------\n"<< endl;

      // process request:
      ossimString command (m_buffer);
      command.trim();

      if (command == "goodbye")
      {
         close(m_clisockfd);
         return true;
      }

      runCommand(command);
      return true;
   }
   return false;
}
