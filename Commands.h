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
    int pid;
 public:
  Command(const char* cmd_line){
      this->cmd_line =(char*)malloc(strlen(cmd_line)+1);
      strcpy(this->cmd_line,cmd_line);
  };
  virtual ~Command(){
      free(this->cmd_line);
  };
  //Copy Constructor
  Command(const Command& command){
        this->cmd_line=(char*)malloc(sizeof(char)*(strlen(command.cmd_line)+1));
        strcpy(this->cmd_line, command.cmd_line);
        this->pid=command.pid;
  };
  //Operator= Constructor
  Command& operator=(const Command& command){
      this->cmd_line=(char*)malloc(sizeof(char)*(strlen(command.cmd_line)+1));
      strcpy(this->cmd_line, command.cmd_line);
      this->pid=command.pid;
      return *this;
  };

  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
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
      if(argNum==1) return;
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
      bool isStopped;
      time_t timeElapsed;
      int jobPid;
      char* cmd_line;
  public:
      JobEntry(int jobID, bool isStopped, time_t timeElapsed, int jobPid, char* cmd_line):jobID(jobID),
                isStopped(isStopped),timeElapsed(timeElapsed),jobPid(jobPid){
          this->cmd_line=(char*)malloc(sizeof(char)*(strlen(cmd_line)+1));
          strcpy(this->cmd_line,cmd_line);
      };
      void setJobID(int jobID){
          this->jobID=jobID;
      }
      //Copy Constructor
      JobEntry(const JobEntry& jobEntry){
          this->jobID=jobEntry.jobID;
          this->isStopped=jobEntry.isStopped;
          this->timeElapsed=jobEntry.timeElapsed;
          this->jobPid=jobEntry.jobPid;
          this->cmd_line=(char*)malloc(sizeof(char)*(strlen(jobEntry.cmd_line)+1));
          strcpy(this->cmd_line,jobEntry.cmd_line);
      };
      //Operator= Constructor
      JobEntry& operator=(const JobEntry& jobEntry){
          this->jobID=jobEntry.jobID;
          this->isStopped=jobEntry.isStopped;
          this->timeElapsed=jobEntry.timeElapsed;
          this->jobPid=jobEntry.jobPid;
          this->cmd_line=(char*)malloc(sizeof(char)*(strlen(jobEntry.cmd_line)+1));
          strcpy(this->cmd_line,jobEntry.cmd_line);
          return *this;
      };

      int getJobID(){
          return this->jobID;
      }
      void printJob(){
          time_t now =time(nullptr);
          double time=difftime(now, this->timeElapsed);
          std::cout << "[" << this->getJobID() << "] " << this->cmd_line << " : " << this->jobPid
                    << " " << time << " secs";
          if(this->isStopped) std::cout << "(stopped)" << std::endl;
          else std::cout << std::endl;
      }
      void setIsStopped(bool isStopped){
          this->isStopped=isStopped;
      }
      bool getIsStopped(){
          return this->isStopped;
      }
      int getJobPid(){
          return this->jobPid;
      }
      time_t getTimeElapsed(){
          return this->timeElapsed;
      }
      char* getCmdLine(){
          return this->cmd_line;
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
    void addJob(int jobPid, char* cmd_line, bool isStopped = false){
        int jobID;
        if(this->jobsList->empty()) jobID=1;
        else {
            jobID=this->jobsList->back().getJobID()+1;
        }
        this->jobsList->push_back(JobEntry(jobID,isStopped,time(nullptr),jobPid,cmd_line));
    };
    void printJobsList(){
        for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
            it->printJob();
        }
    };
  void removeFinishedJobs(){
      for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end();){
          int commandPid=it->getJobPid();
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

  void killAllJobs(){
      for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
          std::cout << it->getJobPid() << ": " << it->getCmdLine() << std::endl;
          if(kill(it->getJobPid(),SIGKILL)==-1) perror("smash error: kill failed");
      }
  };


  JobEntry* getLastJob(){
      if(this->jobsList->empty()) return nullptr;
      return &(this->jobsList->back());
  };

  JobEntry* getLastStoppedJob(){
      for(std::list<JobEntry>::iterator it=jobsList->end(); it != jobsList->begin();){
        --it;
        if(it->getIsStopped()) return &(*it);
      }
      return nullptr;
  };

    int getJobsListSize(){
        return this->jobsList->size();
    };

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
      char *args[COMMAND_ARGS_MAX_LENGTH];
      //char** args=(char**)malloc(sizeof(char)*COMMAND_MAX_ARGS);
      int argNum=_parseCommandLine(this->cmd_line,args);
      if(argNum!=3){
          std::cout << "smash error: kill: invalid arguments" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          return;
      }
      int sigNum,jobNum;
      try{
          sigNum=abs(stoi(args[1]));
          jobNum=abs(stoi(args[2]));
      }
      catch(const std::invalid_argument&){
          std::cout << "smash error: kill: invalid arguments" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          return;
      }

      JobsList::JobEntry* jobByID=this->jobsList->getJobById(jobNum);
      if(jobByID == nullptr){
          std::cout << "smash error: kill: job-id " << jobNum <<" does not exist" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          return;
      }
      int jobPid=jobByID->getJobPid();
      if(kill(jobPid,sigNum)==-1) perror("smash error: kill failed");

      else {
          if(sigNum==SIGSTOP) jobByID->setIsStopped(true);
          if(sigNum==SIGCONT) jobByID->setIsStopped(false);
          std::cout << "signal number " << sigNum <<" was sent to pid " << jobPid << std::endl;
      }

      for(int i=0; i<argNum; i++) free(args[i]);
  };
};


class SmallShell {
private:
    // TODO: Add your data members
    char** plastPwd;
    JobsList* jobsList;
    int foregroundPid;
    char* foregroundCmdLine;
    bool isQuit;
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
    char* getForegroundCmdLine(){
        return this->foregroundCmdLine;
    }
    void setForegroundCmdLine(char* foregroundCmdLine){
        free(this->foregroundCmdLine);
        this->foregroundCmdLine=(char*)malloc(sizeof(char)*(strlen(foregroundCmdLine)+1));
        strcpy(this->foregroundCmdLine,foregroundCmdLine);
    };
    void setIsQuit(bool isQuit){
        this->isQuit=isQuit;
    }
    bool getIsQuit(){
        return this->isQuit;
    }
};


class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~ExternalCommand() = default;

    //Copy Constructor
    ExternalCommand(const ExternalCommand& command) = default;
    //Operator= Constructor
    ExternalCommand& operator=(const ExternalCommand& command)= default;

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
            smash.setForegroundCmdLine(this->cmd_line);
            if(!_isBackgroundComamnd(cmd_line)) waitpid(pid,NULL,0 | WUNTRACED);
            else smash.getJobsList()->addJob(this->pid,this->cmd_line,false);
        }
        free(cmd);
    };
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





class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
private:
    JobsList* jobsList;
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line),jobsList(jobsList){};
    virtual ~ForegroundCommand() = default;
    void execute() override{
        bool invalidArg=false;
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(this->cmd_line,args);
        if(argNum==1){
            JobsList::JobEntry* lastJob=this->jobsList->getLastJob();
            if(lastJob == nullptr){
                std::cout << "smash error: fg: jobs list is empty" << std::endl;
                for(int i=0; i<argNum; i++) free(args[i]);

                return;
            }
            this->jobsList->removeJobById(lastJob->getJobID());
            std::cout << lastJob->getCmdLine() << " : " << lastJob->getJobPid() << std::endl;
            SmallShell& smash=SmallShell::getInstance();
            smash.setForegroundPid(lastJob->getJobPid());
            smash.setForegroundCmdLine(lastJob->getCmdLine());
            if(lastJob->getIsStopped()){
                if(kill(lastJob->getJobPid(),SIGCONT)==-1){
                    perror("smash error: kill failed");
                    for(int i=0; i<argNum; i++) free(args[i]);

                    return;
                }
                lastJob->setIsStopped(false);
            }
            waitpid(lastJob->getJobPid(),NULL,0 | WUNTRACED);
        }
        if(argNum==2) {
            int jobID;
            try{
                jobID=abs(stoi(args[1]));
            }
            catch(const std::invalid_argument&){
                invalidArg=true;
            }
            if(!invalidArg){
                JobsList::JobEntry* job=this->jobsList->getJobById(jobID);
                if(job == nullptr){
                    std::cout << "smash error: fg: job-id " << jobID << " does not exist" << std::endl;
                    for(int i=0; i<argNum; i++) free(args[i]);

                    return;
                }
                else{
                    this->jobsList->removeJobById(job->getJobID());
                    std::cout << job->getCmdLine() << " : " << job->getJobPid() << std::endl;
                    SmallShell& smash=SmallShell::getInstance();
                    smash.setForegroundPid(job->getJobPid());
                    smash.setForegroundCmdLine(job->getCmdLine());
                    if(job->getIsStopped()){
                        if(kill(job->getJobPid(),SIGCONT)==-1){
                            perror("smash error: kill failed");
                            for(int i=0; i<argNum; i++) free(args[i]);

                            return;
                        }
                        job->setIsStopped(false);
                    }
                    if(waitpid(job->getJobPid(),NULL,0 | WUNTRACED)==-1){
                        perror("smash error: waitpid failed");
                        for(int i=0; i<argNum; i++) free(args[i]);

                        return;
                    }
                }
            }
        }
        if((argNum!=1 && argNum!=2) || invalidArg){
            std::cout << "smash error: fg: invalid arguments" << std::endl;
        }
        for(int i=0; i<argNum; i++) free(args[i]);

    };
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
private:
    JobsList* jobsList;
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line),jobsList(jobsList){};
    virtual ~BackgroundCommand() = default;
    void execute() override{
        bool invalidArg=false;
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(this->cmd_line,args);
        if(argNum==1){
            JobsList::JobEntry* lastStoppedJob=this->jobsList->getLastStoppedJob();
            if(lastStoppedJob == nullptr){
                std::cout << "smash error: bg: there is no stopped jobs to resume" << std::endl;
                for(int i=0; i<argNum; i++) free(args[i]);

                return;
            }
            std::cout << lastStoppedJob->getCmdLine() << " : " << lastStoppedJob->getJobPid() << std::endl;
            if(kill(lastStoppedJob->getJobPid(),SIGCONT)==-1){
                perror("smash error: kill failed");
                for(int i=0; i<argNum; i++) free(args[i]);

                return;
            }
            lastStoppedJob->setIsStopped(false);
        }
        if(argNum==2) {
            int jobID;
            try{
                jobID=abs(stoi(args[1]));
            }
            catch(const std::invalid_argument&){
                invalidArg=true;
            }
            if(!invalidArg){
                JobsList::JobEntry* stoppedJob=this->jobsList->getJobById(jobID);
                if(stoppedJob == nullptr){
                    std::cout << "smash error: bg: job-id " << jobID << " does not exist" << std::endl;
                    for(int i=0; i<argNum; i++) free(args[i]);

                    return;
                }
                else{
                    if(stoppedJob->getIsStopped()){
                        std::cout << stoppedJob->getCmdLine() << " : " << stoppedJob->getJobPid() << std::endl;
                        if(kill(stoppedJob->getJobPid(),SIGCONT)==-1){
                            perror("smash error: kill failed");
                            for(int i=0; i<argNum; i++) free(args[i]);

                            return;
                        }
                    }
                    else{
                        std::cout << "smash error: bg: job-id "<< stoppedJob->getJobPid() <<" is already running in the background" << std::endl;
                    }
                }
            }
        }
        if((argNum!=1 && argNum!=2) || invalidArg){
            std::cout << "smash error: bg: invalid arguments" << std::endl;
        }
        for(int i=0; i<argNum; i++) free(args[i]);

    };
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    JobsList* jobsList;
public:
    QuitCommand(const char* cmd_line, JobsList* jobsList):BuiltInCommand(cmd_line),jobsList(jobsList){};
    virtual ~QuitCommand() = default;
    void execute() override{
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(this->cmd_line,args);
        if(argNum>=2 && strcmp(args[1],"kill")==0){
            std::cout << "smash: sending SIGKILL signal to "<<  this->jobsList->getJobsListSize() << " jobs:" << std::endl;
            this->jobsList->killAllJobs();
        }
        for(int i=0; i<argNum; i++){
            free(args[i]);
        }

        SmallShell& smash=SmallShell::getInstance();
        smash.setIsQuit(true);
    };
};





#endif //SMASH_COMMAND_H_
