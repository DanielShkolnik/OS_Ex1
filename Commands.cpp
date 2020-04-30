#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#include <time.h>
#include <list>

using namespace std;
string defaultPromptName="smash> ";
string promptName=defaultPromptName;
bool firstCD=true;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
  this->plastPwd= (char**) malloc(sizeof(char*));
  *(this->plastPwd)= nullptr;
  this->jobsList= new JobsList;
  this->isQuit=false;
  this->foregroundCmdLine= nullptr;
  this->isPipeCommand=false;
  this->timeOutList= new TimeOutList;
}

SmallShell::~SmallShell() {
// TODO: add your implementation
  free(*plastPwd);
  free(plastPwd);
  delete jobsList;
  free(foregroundCmdLine);
  delete timeOutList;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line,bool isPipe) {
	// For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  char *args[COMMAND_ARGS_MAX_LENGTH];
  int argNum = _parseCommandLine(cmd_line, args);
  if(argNum==0){
      args[0]=(char*)malloc(strlen(cmd_line)+1);
      strcpy(args[0],cmd_line);
      argNum=1;
  }
  string cmd_s_special = string(cmd_line);
  this->isPipeCommand=false;


  if(strcmp(args[0],"timeout")==0){
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new TimeOutCommand(cmd_line);
  }

  else if(strcmp(args[0],"cp")==0){
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new CopyCommand(cmd_line,isPipe);
  }

  else if(cmd_s_special.find(">")!=string::npos){
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new RedirectionCommand(cmd_line,isPipe);
  }

  else if(cmd_s_special.find("|")!=string::npos){
    this->isPipeCommand=true;
    for (int i = 0; i < argNum; i++) free(args[i]);
    return new PipeCommand(cmd_line,isPipe);
  }

  else if(strcmp(args[0],"chprompt")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new ChangePromptCommand(cmd_line);
  }

  else if (strcmp(args[0],"pwd")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new GetCurrDirCommand(cmd_line);
  }

  else if(strcmp(args[0],"showpid")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new ShowPidCommand(cmd_line);
  }

  else if(strcmp(args[0],"cd")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new ChangeDirCommand(cmd_line,this->plastPwd);
  }

  else if(strcmp(args[0],"jobs")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new JobsCommand(cmd_line,this->jobsList);
  }

   else if(strcmp(args[0],"kill")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new KillCommand(cmd_line,this->jobsList);
   }

   else if(strcmp(args[0],"fg")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new ForegroundCommand(cmd_line,this->jobsList);
   }

   else if(strcmp(args[0],"bg")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new BackgroundCommand(cmd_line,this->jobsList);
   }

   else if(strcmp(args[0],"quit")==0) {
      for (int i = 0; i < argNum; i++) free(args[i]);
      return new QuitCommand(cmd_line,this->jobsList);
   }

   else {
      for (int i = 0; i < argNum; i++) free(args[i]);
      ExternalCommand* cmd= new ExternalCommand(cmd_line,isPipe);
      return cmd;
   }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  this->setForegroundPid(-1);
  Command* cmd = CreateCommand(cmd_line);
  if(cmd!= nullptr) cmd->execute();
  delete cmd;
}


