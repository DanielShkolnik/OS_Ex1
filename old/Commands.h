#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <unistd.h>
#include <string.h>
#include <fstream>
#include <time.h>
#include <list>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

using namespace std;
extern string defaultPromptName;
extern string promptName;
extern bool firstCD;

class Command;
class JobsList;
class JobEntry;


int _parseCommandLine(const char* cmd_line, char** args, bool backSlash);
bool _isBackgroundComamnd(const char* cmd_line);
void _removeBackgroundSign(char* cmd_line);


class Command {
 protected:
    const char* cmd_line;
    time_t timeElapsed;
    int pid;
    JobsList* jobsList;
 public:
  Command(const char* cmd_line, JobsList* jobsList){
      this->cmd_line = cmd_line;
      this->timeElapsed = time(nullptr);
      this->jobsList = jobsList;
  };
  virtual ~Command()= default;
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
  BuiltInCommand(const char* cmd_line, JobsList* jobsList):Command(cmd_line,jobsList){};
  virtual ~BuiltInCommand() = default;
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
    ChangeDirCommand(const char *cmd_line,char** plastPwd, JobsList* jobsList) : BuiltInCommand(cmd_line,jobsList),plastPwd(plastPwd){};
  virtual ~ChangeDirCommand() = default;
  void execute() override{
      char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
      int argNum=_parseCommandLine(this->cmd_line,args,false);
      if(argNum>2){
          std::cout << "smash error: cd: too many arguments" << endl;
          return;
      }
      if(strcmp(args[1],"-")==0){
          if(firstCD) std::cout << "smash error: cd: OLDPWD not set" << endl;
          else {
              int chdirError=chdir(*plastPwd);
              if(chdirError==-1) perror("smash error: chdir failed");
          }
          return;
      }
      /*
      if(strcmp(args[1],"..")==0){
          char** argsPath=(char**)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
          int argPathNum=_parseCommandLine(args[1],argsPath,true);

      }
       */
      char* oldPath=get_current_dir_name();
      if (oldPath== nullptr) perror("smash error: get_current_dir_name failed");
      if(!firstCD) free(*plastPwd);
      *plastPwd=(char*)malloc(strlen(oldPath)+1);
      strcpy(*plastPwd,oldPath);
      int chdirError=chdir(args[1]);
      if(chdirError==-1) perror("smash error: chdir failed");

      if(firstCD) firstCD=false;
      for(int i=0; i<=argNum; i++) free(args[i]);
      free(args);
  }
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line,jobsList){};
  virtual ~GetCurrDirCommand()= default;
  void execute() override{
      char* path=get_current_dir_name();
      if (path== nullptr) perror("smash error: get_current_dir_name failed");
      else std::cout << path << endl;
  }
};


class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line,jobsList){};
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
        double time=difftime(this->command->getTimeElapsed(), now);
        std::cout << "[" << this->getJobID() << "]" << this->command->getCMD() << ":" << this->command->getPID()
                  << " " << time << " secs";
        if(this->isStopped) std::cout << "(stopped)" << std::endl;
        else std::cout << std::endl;
    }
};

class JobsList {
 // TODO: Add your data members
private:
    std::list<JobEntry>* jobsList;
 public:
  JobsList():jobsList(new std::list<JobEntry>){};
  ~JobsList(){
      delete jobsList;
  };
  void addJob(Command* cmd, bool isStopped){
      int jobID;
      if(this->jobsList->empty()) jobID=1;
      else {
          jobID=this->jobsList->back().getJobID()+1;
      }
      this->jobsList->push_back(JobEntry(jobID,cmd,isStopped));
  };
  void printJobsList(){
      for(JobEntry job : *(this->jobsList)){
          std::cout << "loop" << std::endl;
          job.printJob();
      }
  };
  void killAllJobs();
  void removeFinishedJobs(){

  };
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line,jobsList){
      std::cout << "constructor" << std::endl;
  };
  virtual ~JobsCommand() = default;
  void execute() override{
      this->jobsList->printJobsList();
      std::cout << "printJobsList" << std::endl;
  }
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
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
  void execute() override {};
};


// TODO: should it really inhirit from BuiltInCommand ?
class CopyCommand : public BuiltInCommand {
 public:
  CopyCommand(const char* cmd_line);
  virtual ~CopyCommand() {}
  void execute() override{

  };
};

class ChangePromptCommand : public BuiltInCommand {
public:
    ChangePromptCommand(const char* cmd_line):BuiltInCommand(cmd_line,jobsList){};
    virtual ~ChangePromptCommand()= default;
    void execute() override{
        char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
        int argNum=_parseCommandLine(this->cmd_line,args,false);
        if (argNum > 1 ){
            promptName=args[1];
            promptName+="> ";
        }
        else if(argNum == 1){
            promptName=defaultPromptName;
        }
        for(int i=0; i<=argNum; i++) free(args[i]);
        free(args);
    }
};

// TODO: add more classes if needed 
// maybe chprompt , timeout ?

class SmallShell {
 private:
  // TODO: Add your data members
  char** plastPwd;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line, char** plastPwd, JobsList* jobsList);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line, char** plastPwd, JobsList* jobsList);
  // TODO: add extra methods as needed
};




class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line, JobsList* jobsList):Command(cmd_line,jobsList){};
    virtual ~ExternalCommand() = default;
    void execute() override{
        char* cmd=(char*)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd,this->cmd_line);
        if(_isBackgroundComamnd(this->cmd_line)){
            //Add to Job list
            //_removeBackgroundSign(cmd);
        }
        pid_t pid=fork();
        if(pid==-1) perror("smash error: fork failed");
        //Child:
        if(pid==0){
            char* argv[] = {(char*)"/bin/bash", (char*)"-c", cmd, NULL};
            execv(argv[0], argv);
        }
            //Parent:
        else{
            this->pid=pid;
            if(_isBackgroundComamnd(this->cmd_line)){
                this->jobsList->addJob(this,false);
            }
            waitpid(pid,NULL,0);
        }
        free(cmd);
    }
};








#endif //SMASH_COMMAND_H_
