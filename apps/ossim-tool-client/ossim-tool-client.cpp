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
#include <ossim/util/ossimToolClient.h>

using namespace std;

#define MAX_BUF_LEN 4096

int main(int argc, char *argv[])
{
   char buffer [MAX_BUF_LEN];
   if (argc < 2)
   {
      cout<<"Usage "<<argv[0]<<" host:port\n"<<endl;;
      exit(0);
   }

   int n;

   ossimToolClient otc;
   int svrsockfd = otc.connectToServer(argv[1]);
   if (svrsockfd < 0)
   {
      cout << "\nossim-client: Bad socket file descriptor returned."<<endl;
      exit (-1);
   }

   ossimString command;
   while (1)
   {
      // Fetch message from console:
      cout<<"\nEnter OSSIM command: "<<ends;
      cin.getline(buffer, MAX_BUF_LEN);
      command = buffer;
      if (command.empty() || (command == "q") || (command == "quit"))
      {
         otc.disconnect();
         break;
      }

      // Send command to the OSSIM tool server:
      if (otc.execute(command.chars()))
         cout << "\nossim-client: Execute call successful."<<endl;
      else
         cout << "\nossim-client: Error encountred on execute."<<endl;
   }

   cout << "\nossim-client: Closing connection to OSSIM server."<<endl;
   close(svrsockfd);

   return 0;
}
