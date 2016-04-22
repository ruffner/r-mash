#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include "csapp.h"
#include "rrsh.h"

using namespace std;

void echo(int fd);

void read_users(vector<string> & users, vector<string> & pass);
void read_commands(vector<string> & commands);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];

    vector<string> users, passwords, commands;


    if (argc != 2) {
      cerr << "usage: " << argv[0] << " <port>" << endl;
      exit(0);
    }


    read_users(users, passwords);
    

    read_commands(commands);


    exit(0);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
	clientlen = sizeof(struct sockaddr_storage); 
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
	
	//c = parse_command(inLine.c_str());


      //handle_command(c);
      //free_command(c);


	//echo(connfd);
	Close(connfd);
    }
    exit(0);
}

void read_users(vector<string> & users, vector<string> & pass) 
{
  int line = 0;
  ifstream inFile;
  string inLine;
  
  inFile.open(RRSH_USERS_FILE, ifstream::in);

  if( !inFile.good() ){
    cerr << "INIT: error reading user file " << RRSH_USERS_FILE <<   endl;
    exit(1);
  }

  while( getline(inFile, inLine) ){
    bool fName=false, fPass=false;
    char c;

    if( inLine.compare("") == 0 ) continue;

    line++;
    
    users.push_back("");
    pass.push_back("");

    for( int i=0; i < inLine.size(); i++ ){
      c = inLine[i];

      if( fName ){
	if( pass[line-1].size() ){
	  if( c != ' ' && c != '\t' ){
	    if( fPass ){
	      cerr << "Syntax error on line " << line << " of " << RRSH_USERS_FILE << endl;
	      exit(1);
	    } else {
	      pass[line-1].append(1, c);
	    }
	  } else {
	    pass[line-1].append(1, c);
	    cerr << "Ignoring trailing whitespace on line " << line << " in " << RRSH_USERS_FILE << endl;
	    fPass = true;
	    continue;
	  }
	} else {
	  if( c != ' '  && c != '\t' ){
	    pass[line-1].append(1, c);
	  } else {
	    continue;
	  }
	}

      } else {
	if( users[line-1].size() ){
	  if( c != ' '  && c != '\t' ){
	    users[line-1].append(1, c);
	  } else {
	    fName = true;
	    continue;
	  }
	} else {
	  if( c != ' ' && c != '\t' ){
	    users[line-1].append(1, c);
	  } else {
	    cerr << "Ignoring leading whitespace on line " << line << "in " << RRSH_USERS_FILE << endl;
	    continue;
	  }
	}	
      } 
    }

    if(!users[line-1].size() || !pass[line-1].size() ){
      cerr << "Error on line " << line << " in " << RRSH_USERS_FILE << endl;
      exit(1);
    }
  }

  inFile.close();
}

void read_commands(vector<string> & commands)
{
  int line = 0;
  ifstream inFile;
  string inLine;
  
  inFile.open(RRSH_COMMANDS_FILE, ifstream::in);

  if( !inFile.good() ){
    cerr << "INIT: error reading user file " << RRSH_COMMANDS_FILE << endl;
    exit(1);
  }

  while( getline(inFile, inLine) ){
    bool fCmd = true;
    
    line++;
    if( inLine.compare("") == 0 ) continue;
    
    for( int i=0; i<inLine.size(); i++ ){
      if( inLine[i] == ' ' || inLine[i] == '\t' ){
	cerr << "Error on line " << line << " of " << RRSH_COMMANDS_FILE << endl;
	fCmd = false;
	break;
      } 
    }

    if( fCmd )
      commands.push_back(inLine);
  }

  inFile.close();
}
