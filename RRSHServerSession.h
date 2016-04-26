#ifndef RRSH_SERVER_SESSION
#define RRSH_SERVER_SESSION

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

// struct to hold user info
struct user {
  std::string name;
  std::string pass;
};

class RRSHServerSession {
public:
  RRSHServerSession(int fd, std::string uFile, std::string cFile);

  std::string getActiveUser();
  bool loadUsersFromFile(std::string filename);
  bool loadCommandsFromFile(std::string filename);
  bool awaitAuth();
  void awaitExit();
  bool isLoggedIn();

private:
  rio_t rio;
  int connfd;
  int nullfd;
  int exec_out, exec_err;
  
  bool loggedIn;

  std::string activeUser;
  std::vector<struct user> users;
  std::vector<std::string> commands;


  bool isWhite(char c);
  bool isWhite(std::string s);
  bool isValidCommand(char * cmd);
};

#endif
// END DEFINE RRSH_SERVER_SESSION
