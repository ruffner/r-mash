#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include "csapp.h"
#include "rrsh.h"

using namespace std;

void echo(int fd);

string whiteStrip(char * s);

void read_users(vector<string> & users, vector<string> & pass);
void read_commands(vector<string> & commands);
int handle_login(vector<string> & users, vector<string> & pass, int fd, char * addr, char * port);
int is_valid_command(char * arg, vector<string> & cmds);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    char buf[MAXLINE], client_hostname[MAXLINE], client_port[MAXLINE];
    rio_t rio;
	
    vector<string> users, passwords, commands;


    if (argc != 2) {
      cerr << "usage: " << argv[0] << " <port>" << endl;
      exit(0);
    }


    read_users(users, passwords);
    

    read_commands(commands);


    listenfd = Open_listenfd(argv[1]);

    while (1) {
      struct command *c;
      clientlen = sizeof(struct sockaddr_storage); 
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      
      Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
		  client_port, MAXLINE, 0);
      
      
      // initialize read 
      //Rio_readinitb(&rio, connfd);
      
      // authenticate user
      if( handle_login(users, passwords, connfd, client_hostname, client_port) ){
	printf("Connected to (%s, %s)\n", client_hostname, client_port);
      } else {
	
      }
      
      
      // init and
	// read line
      Rio_readinitb(&rio, connfd);
      Rio_readlineb(&rio, buf, MAXLINE);
      
      
      
      c = parse_command(buf);
      char * cmd = c->args[0];
      
      if( cmd != NULL && 
	  is_valid_command(cmd, commands) &&
	  c->out_redir == NULL &&
	  c->in_redir == NULL){
	
	cout << "user  executing " << cmd << endl;

	for( int i=1; c->args[i] != NULL; i++ ){

	}

      } else {
	cout << "invalid command" << endl;
	
      }

      while( c->args[0] != NULL && 
	     c->args[0] != "exit" && 
	     c->out_redir == NULL &&
	     c->in_redir == NULL){
	
	//cout << "Executing command '" << arg
	
      }
      
      Rio_writen(connfd, buf, strlen(buf));
      
      
      free_command(c);
      
      Close(connfd);	
    }
    
    close(listenfd);
    exit(0);
}


int handle_login(vector<string> & users, vector<string> & pass, int fd, char * host, char * port)
{
  int userIndex;
  char buf[MAXLINE];
  bool vUser=false, vPass=false;
  rio_t rio;

  // init read
  Rio_readinitb(&rio, fd);

  // read in username entry
  int rlen = Rio_readlineb(&rio, buf, MAXLINE);

  if( buf[rlen-1] != '\n' ){
    cerr << "- No newline received on username input!" << endl;
    exit(1);
  }
  buf[ rlen-1 ] = 0;

  // check if in users list
  userIndex = 0;

  for( string s:users ){
    if( s.compare(string(buf)) == 0 ){
      cout << s << " logging in from " << host << " on port " << port << "." << endl;
      vUser = true;
      break;
    }
    userIndex++;
  }

  // invalid user cannot log in
  if( !vUser ){
    cout << "User " << buf << " not in users list!" << endl;
    return 0;
  }

  // init and read in password entry
  Rio_readinitb(&rio, fd);
  rlen = Rio_readlineb(&rio, buf, MAXLINE);

  buf[ rlen-1 ] = 0;

  if( pass[userIndex].compare( string(buf) ) != 0 ){
    cout << users[userIndex] << " denied access." << endl;
    return false;
  } else {
    cout << users[userIndex] << " logged in." << endl;
    return true;
  }
}

int is_valid_command(char * arg, vector<string> & cmds)
{
  for( string c:cmds ){
    if( c.compare(arg) == 0 ){
      return true;
    }
  }

  return false;
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

string whiteStrip(char * s)
{
  string out(s);

  while( out.at(0) == ' ' )
    out = out.erase(0, 1);
  while( out.at(out.size()-1) == ' ' )
    out.pop_back();

  return out;
}
