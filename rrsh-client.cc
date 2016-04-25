#include <iostream>
#include <string>
#include <cstdlib>
#include "csapp.h"
#include "rrsh.h"

using namespace std;

int login(int fd, rio_t & rio);

int main(int argc, char **argv) 
{
  int clientfd;
  char *host, *port, buf[MAXLINE];
  rio_t rio;
  string inLine;
  
  if (argc != 3) {
    cerr << "usage: " << argv[0] << " <host> <port>" << endl;
    exit(0);
  }
  host = argv[1];
  port = argv[2];
  
  clientfd = Open_clientfd(host, port);

  // handle user login
  login(clientfd, rio);

  cout << RRSH_PROMPT;

  while( getline(cin, inLine) ){
    Rio_readinitb(&rio, clientfd);

    if(inLine.length()) {
      if(inLine.compare("exit") == 0) break;
      strcpy( buf, inLine.c_str() );
      Rio_writen( clientfd, buf, MAXLINE );
      
      int nb = 1;

      do {
	nb = Rio_readlineb( &rio, buf, MAXLINE );
	if( strcmp(buf, "RRSH COMMAND COMPLETED\n") != 0 )
	  cout << buf;
      } while( strcmp(buf, "RRSH COMMAND COMPLETED\n") != 0 && nb &&
	       strcmp(buf, "Command Dissallowed\n") != 0);

    }
    
    cout << RRSH_PROMPT;
  }

  Close(clientfd);
  exit(0);
}


int login(int fd, rio_t & rio)
{
  char buf[MAXLINE];
  string uname, pass;

  Rio_readinitb(&rio, fd);  

  while( strcmp( buf, RRSH_LOGIN_APPROVED ) != 0 &&
	 strcmp( buf, RRSH_LOGIN_DENIED ) != 0){
    
    cout << "Username: ";

    while( !getline(cin, uname) || 
	   uname.size() > RRSH_MAX_CRED_LENGTH ||
	   !uname.size() ){
      cout << "Username: ";
    }
    uname.append("\n");
    strcpy(buf, uname.c_str());
    
    Rio_writen(fd, buf, uname.size());
    
    cout << "Password: ";
    
    while( !getline(cin, pass) || pass.size() > RRSH_MAX_CRED_LENGTH ){
      cout << "Please enter a valid password: ";
    }
    pass.append("\n");
    strcpy(buf, pass.c_str());
    
    Rio_writen(fd, buf, pass.size());
    Rio_readlineb(&rio, buf, MAXLINE);
    buf[strlen(buf)-1] = '\n';
  }
  
  cout << buf << endl;
 
  if( strcmp( buf, RRSH_LOGIN_DENIED ) == 0 ){
    exit(1);
  }
}
