#include "RRSHServerSession.h"

using namespace std;

RRSHServerSession::RRSHServerSession(int fd, string uFile, string cFile)
: connfd(fd), activeUser("")
{
  loadUsersFromFile(uFile);
  
  loadCommandsFromFile(cFile);

  // prepare for first read
  Rio_readinitb(&rio, fd);
}

// takes a filename, reads in users and passwords
// separated by whitespace
bool RRSHServerSession::loadUsersFromFile(string filename)
{
  int line = 0;
  string inLine;
  ifstream inFile;
  
  inFile.open(filename.c_str(), ifstream::in);

  if( !inFile.good() ){
    cerr << "error opening user file" << endl;
  }

  while( getline(inFile, inLine) ){
    bool next = false;

    struct user u;

    // disregard blank lines
    if( inLine.compare("") == 0 )
      continue;

    line++;

    for( char c:inLine ){
      // still reading username
      if( !isWhite(c) && !next)
	u.name.push_back(c);

      // havent gotten uname and we get to a blank - error
      else if( isWhite(c) && !u.name.size() && !next )
	cerr << "error on line " << line << "of users file" << endl;

      // have read uname and we find space, founfUname=true
      else if( isWhite(c) && u.name.size() && !next )
	next = true;

      // keep looping while we read white between uname and pass
      else if( isWhite(c) && u.name.size() && next )
	continue;

      // reading password
      else if( !isWhite(c) && next )
	u.pass.push_back(c);
      
      // we've read password and get to blank - read done
      else if( isWhite(c) && next )
	break;
    }

    // add these creds to list
    users.push_back(u);
  }
  
  inFile.close();
}

// takes a filename.
// reads in lines and adds them to command list
bool RRSHServerSession::loadCommandsFromFile(string filename)
{
  string inLine;
  ifstream inFile;

  inFile.open(filename.c_str(), ifstream::in);

  if( !inFile.good() ){
    cerr << "error opening user file" << endl;
  }

  while( getline(inFile, inLine) ){
    
    // ignore blank lines
    if( inLine.compare("") == 0 )
      continue;

    // add command to list
    commands.push_back(inLine);
  }

  inFile.close();
}

// blocks until 2 lines have been read, 41 chars each
// returns true if entered creds are in users list
bool RRSHServerSession::awaitAuth()
{
  char nameBuf[RRSH_MAX_CRED_LENGTH+1];
  char passBuf[RRSH_MAX_CRED_LENGTH+1];
 
  // read 41 chars max for uname and pass
  Rio_readlineb(&rio, nameBuf, RRSH_MAX_CRED_LENGTH);
  Rio_readlineb(&rio, passBuf, RRSH_MAX_CRED_LENGTH);

  string uname(nameBuf), pass(passBuf);

  // delete newlines
  uname.pop_back();
  pass.pop_back();

  cout << "User " << uname << " trying login." << endl;

  for( int i=0; i<users.size(); i++ ){
    if( users[i].name == uname && users[i].pass == pass ){

      // if credentials match, send login approved message
      strcpy(nameBuf, RRSH_LOGIN_APPROVED);
      Rio_writen(connfd, nameBuf, MAXLINE);

      // set active user
      activeUser = uname;
      return true;
    }
  }

  // otherwise send login failed message
  strcpy(nameBuf, RRSH_LOGIN_DENIED);
  Rio_writen(connfd, nameBuf, MAXLINE);
      
  return false;
}

// blocks until either the client disconnects or the
// exit command is received
void RRSHServerSession::awaitExit()
{
  size_t bytesRead;
  
  while( 42 ){
    char buf[MAXLINE];

    Rio_readinitb(&rio, connfd);
    bytesRead = Rio_readlineb(&rio, buf, MAXLINE);

    // crude
    if (errno) break;

    struct command * c;

    // bad read, exit
    if( bytesRead == 0 || strlen(buf) <= 0 || strcmp(buf, "") == 0){
      //cout << "zero byte read" << endl;
      break;
    }

    // exit if were told to
    if( strcmp(buf, "exit") == 0 ){
      break;
    }
    
    c = parse_command(buf);
    
    // if command is not in provided list, send disallowed message
    if( !isValidCommand( c->args[0] ) ){
      cout << "User " << getActiveUser() << " blocked from: " << buf << endl;
      char msg[] = "Command Dissallowed\n";
      Rio_writen(connfd, msg, MAXLINE);
    } 
    
    // otherwise start execution
    else {
      cout << "User " << getActiveUser() << " executing: " << buf << endl;
      
      pid_t p;
      int status;
      // start new process
      p = fork();

      // child
      if( p == 0 ){
	
	// redirect stdin to /dev/null
	nullfd = open( "/dev/null", O_RDONLY );
	dup2(nullfd, 0);


	// redirect out to socket
	exec_out = dup2(connfd, 1);

	// redirect err to socket
	exec_err = dup2(connfd, 2);

	int ret = 0;

	// execute external command
	//	if( (ret = execv(c->args[0], c->args)) < 0 )
	if( (ret = execv(c->args[0], c->args)) < 0 )
	  cerr << "Error: failed to execute '" << buf << "'" << endl;


	// close input
	dup2(0, nullfd);

	// restore socket ouput
	dup2(exec_out, 1);
	dup2(exec_err, 2);

	close(nullfd);
	close(exec_out);
	close(exec_err);

	if( ret < 0 )
	  exit(-1);
	else
	  exit(0);
      }
      // parent process
      else if ( p > 0 ){
	// wait for the child process to exit()
	waitpid(p, &status, 0); 

	// send command completed message
	strcpy(buf, "\nRRSH COMMAND COMPLETED\n");
	Rio_writen(connfd, buf, MAXLINE);

      }
      else {
	// fork failed
	cerr << "fork() failed!" << endl;
      }
    }

    free_command(c);
  }
  
  // for whatever reason
  cout << "User " << getActiveUser() << " ended session." << endl;
}

// helper
bool RRSHServerSession::isLoggedIn()
{
  return loggedIn;
}

// helper
string RRSHServerSession::getActiveUser()
{
  return activeUser;
}

// helper
bool RRSHServerSession::isWhite(char c)
{
  return ( c==' ' || c=='\t' || c=='\n' );
}

// helper
bool RRSHServerSession::isValidCommand(char *cmd)
{
  for( string s:commands )
    if( strcmp(cmd, s.c_str()) == 0 )
      return true;
  
  return false;
}
