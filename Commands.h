#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <list>
#include <sys/wait.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

using namespace std;
extern string defaultPromptName;
extern string promptName;
extern bool firstCD;

int _parseCommandLine(const char* cmd_line, char** args);
bool _isBackgroundComamnd(const char* cmd_line);
void _removeBackgroundSign(char* cmd_line);

class Command {
// TODO: Add your data members
protected:
    char* cmd_line;
    time_t timeElapsed;
    int pid;
 public:
  Command(const char* cmd_line){
      this->cmd_line =(char*)malloc(strlen(cmd_line)+1);
      strcpy(this->cmd_line,cmd_line);
      this->timeElapsed = time(nullptr);
  };
  virtual ~Command(){
      free(this->cmd_line);
  };
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
  time_t getTimeElapsed(){
      return this->timeElapsed;
  }
  const char*getCMD(){
      return this->cmd_line;
  }
  int getPID(){
      return this->pid;
  }
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~BuiltInCommand()= default;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
    virtual ~ChangePromptCommand()= default;
    void execute() override{
      char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
      int argNum=_parseCommandLine(this->cmd_line,args);
      if (argNum > 1 ){
        promptName=args[1];
        promptName+="> ";
      }
      else if(argNum == 1){
        promptName=defaultPromptName;
      }
      for(int i=0; i<argNum; i++) free(args[i]);
      free(args);
    }
};


class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    char **plastPwd;
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd):BuiltInCommand(cmd_line),plastPwd(plastPwd){};
  virtual ~ChangeDirCommand() = default;
  void execute() override{
      char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
      int argNum=_parseCommandLine(this->cmd_line,args);
      if(argNum>2){
          std::cout << "smash error: cd: too many arguments" << endl;
          return;
      }
      if(strcmp(args[1],"-")==0){
          if(firstCD) std::cout << "smash error: cd: OLDPWD not set" << endl;
          else {
              char* temp=get_current_dir_name();
              int chdirError=chdir( *plastPwd);
              if(!firstCD) free( *plastPwd);
              *plastPwd=(char*)malloc(strlen(temp)+1);
              strcpy(*plastPwd,temp);
              if(chdirError==-1) perror("smash error: chdir failed");
          }
          return;
      }
      char* oldPath=get_current_dir_name();
      if (oldPath== nullptr) perror("smash error: get_current_dir_name failed");
      if(!firstCD) free(*plastPwd);
      *plastPwd=(char*)malloc(strlen(oldPath)+1);
      strcpy(*plastPwd,oldPath);
      int chdirError=chdir(args[1]);
      if(chdirError==-1) perror("smash error: chdir failed");

      if(firstCD) firstCD=false;
      for(int i=0; i<argNum; i++) free(args[i]);
      free(args);
  }
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() = default;
  void execute() override{
      char* path=get_current_dir_name();
      if (path== nullptr) perror("smash error: get_current_dir_name failed");
      else std::cout << path << endl;
  };
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() = default;
  void execute() override{
      std::cout << "smash pid is: " << getpid() << endl;
  };
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class CommandsHistory {
 protected:
  class CommandHistoryEntry {
	  // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  CommandsHistory();
  ~CommandsHistory() {}
  void addRecord(const char* cmd_line);
  void printHistory();
};

class HistoryCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history);
  virtual ~HistoryCommand() {}
  void execute() override;
};

class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
  private:
      int jobID;
      Command* command;
      bool isStopped;
  public:
      JobEntry(int jobID, Command* command, bool isStopped):jobID(jobID),command(command),isStopped(isStopped){};
      void setJobID(int jobID){
          this->jobID=jobID;
      }
      int getJobID(){
          return this->jobID;
      }
      void printJob(){
          time_t now =time(nullptr);
          double time=difftime(now, this->command->getTimeElapsed());
          std::cout << "[" << this->getJobID() << "] " << this->command->getCMD() << " : " << this->command->getPID()
                    << " " << time << " secs";
          if(this->isStopped) std::cout << "(stopped)" << std::endl;
          else std::cout << std::endl;
      }
      Command* getCommand(){
          return this->command;
      }

  };

 // TODO: Add your data members
private:
    std::list<JobEntry>* jobsList;
 public:
    JobsList():jobsList(new std::list<JobEntry>){};
    ~JobsList(){
        delete jobsList;
    };
    void addJob(Command* cmd, bool isStopped = false){
        int jobID;
        if(this->jobsList->empty()) jobID=1;
        else {
            jobID=this->jobsList->back().getJobID()+1;
        }
        this->jobsList->push_back(JobEntry(jobID,cmd,isStopped));
    };
    void printJobsList(){
        for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
            it->printJob();
        }
    };
  void removeFinishedJobs(){
      for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end();){
          int commandPid=it->getCommand()->getPID();
          if(waitpid(commandPid,NULL,WNOHANG)==commandPid){
              it=this->jobsList->erase(it);
              if(it==this->jobsList->end()) break;
          }
          else {
              ++it;
          }
      }
  };
  JobEntry* getJobById(int jobId){
      for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
          if(it->getJobID()==jobId) return &(*it);
      }
      return nullptr;
  };
  void removeJobById(int jobId){
      for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
          if(it->getJobID()==jobId){
              this->jobsList->erase(it);
              break;
          }
      }
  };

  void killAllJobs();
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
private:
    JobsList* jobsList;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line),jobsList(jobsList){};
  virtual ~JobsCommand() = default;
    void execute() override{
        this->jobsList->printJobsList();
    }
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
private:
    JobsList* jobsList;
 public:
  KillCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line),jobsList(jobsList){};
  virtual ~KillCommand() = default;
  void execute() override{
      char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
      int argNum=_parseCommandLine(this->cmd_line,args);
      char signal[200];
      strcpy(signal,args[1]);
      //int sigNum=stoi(signal);
      //std::cout << "test stoi: " << sigNum << std::endl;

      for(int i=0; i<argNum; i++){
          std::cout << "test: " << i << std::endl;
          free(args[i]);
      }
      free(args);

  };
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};


// TODO: should it really inhirit from BuiltInCommand ?
class CopyCommand : public BuiltInCommand {
 public:
  CopyCommand(const char* cmd_line);
  virtual ~CopyCommand() {}
  void execute() override;
};

// TODO: add more classes if needed 
// maybe chprompt , timeout ?

class SmallShell {
 private:
  // TODO: Add your data members
  char** plastPwd;
  JobsList* jobsList;
  int foregroundPid;
  Command* foregroundCommand;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  int getForegroundPid(){
      return this->foregroundPid;
  }
  void setForegroundPid(int foregroundPid){
      this->foregroundPid=foregroundPid;
  }
  JobsList* getJobsList(){
      return this->jobsList;
  }
  Command* getForegroundCommand(){
      return this->foregroundCommand;
  }
  void setForegroundCommand(Command* foregroundCommand){
      this->foregroundCommand=foregroundCommand;
  };
};


class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~ExternalCommand() = default;
    void execute() override{
        char* cmd=(char*)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd,this->cmd_line);
        if(_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        pid_t pid=fork();
        if(pid==-1) perror("smash error: fork failed");
        //Child:
        if(pid==0){
            char* argv[] = {(char*)"/bin/bash", (char*)"-c", cmd, NULL};
            setpgrp();
            execv(argv[0], argv);
            perror("smash error: execv failed");
        }
        //Parent:
        else{
            this->pid=pid;
            SmallShell& smash=SmallShell::getInstance();
            smash.setForegroundPid(pid);
            smash.setForegroundCommand(this);
            if(!_isBackgroundComamnd(cmd_line)) waitpid(pid,NULL,0 | WUNTRACED);
        }
        free(cmd);
    };
};

#endif //SMASH_COMMAND_H_
