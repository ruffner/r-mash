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
  Rio_readinitb(&rio, clientfd);

  login(clientfd, rio);


  Close(clientfd);
  
  ////
  
  while( getline(cin, inLine) ){
    
    if(inLine.length()) {



    }
    
  }
  

  ////


  while (Fgets(buf, MAXLINE, stdin) != NULL) {
    Rio_writen(clientfd, buf, strlen(buf));
    Rio_readlineb(&rio, buf, MAXLINE);
    Fputs(buf, stdout);
  }
  Close(clientfd); //line:netp:echoclient:close
  exit(0);
}


int login(int fd, rio_t & rio)
{
  char buf[MAXLINE];
  string uname, pass;
    
  cout << "Username: ";


  while( strcmp( buf, RRSH_LOGIN_APPROVED ) != 0 ){
    while( !getline(cin, uname) ||  uname.size() > RRSH_MAX_CRED_LENGTH ){
      cout << "Please enter a valid username: ";
    }
    uname.append("\n");
    strcpy(buf, uname.c_str());
    
    Rio_writen(fd, buf, uname.size());
    
    //Rio_readlineb(&rio, buf, MAXLINE);
    
  
    
    //cout << "received response: " << buf << endl;
    
    /////
    
    cout << "Password: ";
    
    while( !getline(cin, pass) || pass.size() > RRSH_MAX_CRED_LENGTH ){
      cout << "Please enter a valid password: ";
    }
    cout << "sending";
    pass.append("\n");
    strcpy(buf, pass.c_str());
    
    Rio_writen(fd, buf, pass.size());
    
    Rio_readlineb(&rio, buf, MAXLINE);
    
    cout << "received response: " << buf << endl;

  }
  exit(0);
  
}
