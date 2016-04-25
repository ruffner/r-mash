#include "RRSHServerSession.h"

using namespace std;

RRSHServerSession::RRSHServerSession(int fd, string uFile, string cFile)
: connfd(fd), activeUser("")
{
  loadUsersFromFile(uFile);
  
  loadCommandsFromFile(cFile);

  Rio_readinitb(&rio, fd);
}

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

    if( inLine.compare("") == 0 )
      continue;

    line++;

    for( char c:inLine ){
      if( !isWhite(c) && !next)
	u.name.push_back(c);
      else if( isWhite(c) && !u.name.size() && !next )
	cerr << "error on line " << line << "of users file" << endl;
      else if( isWhite(c) && u.name.size() && !next )
	next = true;
      else if( isWhite(c) && u.name.size() && next )
	continue;
      else if( !isWhite(c) && next )
	u.pass.push_back(c);
      else if( isWhite(c) && next )
	break;
    }

    users.push_back(u);
  }
  
  inFile.close();
}

bool RRSHServerSession::loadCommandsFromFile(string filename)
{
  string inLine;
  ifstream inFile;

  inFile.open(filename.c_str(), ifstream::in);

  if( !inFile.good() ){
    cerr << "error opening user file" << endl;
  }

  while( getline(inFile, inLine) ){

    if( inLine.compare("") == 0 )
      continue;

    commands.push_back(inLine);
  }

  inFile.close();
}

bool RRSHServerSession::awaitAuth()
{
  char nameBuf[RRSH_MAX_CRED_LENGTH+1];
  char passBuf[RRSH_MAX_CRED_LENGTH+1];
 
  Rio_readlineb(&rio, nameBuf, RRSH_MAX_CRED_LENGTH);
  Rio_readlineb(&rio, passBuf, RRSH_MAX_CRED_LENGTH);

  string uname(nameBuf), pass(passBuf);
  uname.pop_back();
  pass.pop_back();

  cout << "got user " << uname << endl;
  cout << "got pass " << pass << endl;
  
  for( int i=0; i<users.size(); i++ ){
    if( users[i].name == uname && users[i].pass == pass ){
      strcpy(nameBuf, RRSH_LOGIN_APPROVED);
      Rio_writen(connfd, nameBuf, MAXLINE);
      activeUser = uname;
      return true;
    }
  }

  strcpy(nameBuf, RRSH_LOGIN_DENIED);
  Rio_writen(connfd, nameBuf, MAXLINE);
      
  return false;
}

void RRSHServerSession::awaitExit()
{
  size_t bytesRead;
  
  while( 42 ){
    char buf[MAXLINE];

    Rio_readinitb(&rio, connfd);
    bytesRead = Rio_readlineb(&rio, buf, MAXLINE);

    if (errno) break;

    struct command * c;

    if( bytesRead == 0 || strlen(buf) <= 0 || strcmp(buf, "") == 0){
      //cout << "zero byte read" << endl;
      break;
    }

    if( strcmp(buf, "exit") == 0 ){
      break;
    }
    
    c = parse_command(buf);
    
    if( !isValidCommand( c->args[0] ) ){
      cout << "User " << getActiveUser() << " blocked from: " << buf << endl;
      char msg[] = "Command Dissallowed\n";
      Rio_writen(connfd, msg, MAXLINE);
    } else {
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

	//close(connfd);

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
	//close(connfd);
	// wait for the child process to exit()
	waitpid(p, &status, 0); 
	
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

  cout << "User " << getActiveUser() << " ended session." << endl;
}

bool RRSHServerSession::isLoggedIn()
{
  return loggedIn;
}

string RRSHServerSession::getActiveUser()
{
  return activeUser;
}

bool RRSHServerSession::isWhite(char c)
{
  return ( c==' ' || c=='\t' || c=='\n' );
}

bool RRSHServerSession::isValidCommand(char *cmd)
{
  for( string s:commands )
    if( strcmp(cmd, s.c_str()) == 0 )
      return true;
  
  return false;
}
