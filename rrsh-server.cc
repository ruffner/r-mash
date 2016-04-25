#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "csapp.h"
#include "rrsh.h"

#include "RRSHServerSession.h"

using namespace std;

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    char buf[MAXLINE], client_hostname[MAXLINE], client_port[MAXLINE];
    rio_t rio;
	
    if (argc != 2) {
      cerr << "usage: " << argv[0] << " <port>" << endl;
      exit(0);
    }

    // listen for incoming connections on the specified port
    listenfd = Open_listenfd(argv[1]);


    while (1) {
      clientlen = sizeof(struct sockaddr_storage); 
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      
      Getnameinfo((SA *) &clientaddr, 
		  clientlen, 
		  client_hostname, 
		  MAXLINE, 
		  client_port, 
		  MAXLINE, 
		  0);

      
      RRSHServerSession session(connfd, 
				string(RRSH_USERS_FILE), 
				string(RRSH_COMMANDS_FILE));
      
      if( session.awaitAuth() ){
	cout << "User " << session.getActiveUser() 
	     << " connected from (" << client_hostname 
	     << ", " << client_port << ")" << endl;
      } else {
	cout << "User login failed" << endl;
	Close(connfd);
	continue;
      }

      session.awaitExit();

      Close(connfd);	
    }
    
    // done listening for connections
    close(listenfd);
    exit(0);
}

