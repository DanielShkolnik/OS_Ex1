#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <list>
#include <sys/wait.h>

#include <cmath>
#include <fcntl.h>

#define MAX_PATH (260)


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
        char* cmd=(char*)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd,this->cmd_line);
        if(_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(cmd,args);
        if (argNum > 1 ){
            promptName=args[1];
            promptName+="> ";
        }
        else if(argNum == 1){
            promptName=defaultPromptName;
        }
        for(int i=0; i<argNum; i++) free(args[i]);
        free(cmd);
    }
};




class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
private:
    char **plastPwd;
public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd):BuiltInCommand(cmd_line),plastPwd(plastPwd){};
  virtual ~ChangeDirCommand() = default;
  void execute() override{
      char* cmd=(char*)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
      strcpy(cmd,this->cmd_line);
      if(_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
      char *args[COMMAND_ARGS_MAX_LENGTH];
      int argNum=_parseCommandLine(cmd,args);
      if(argNum>2){
          std::cerr << "smash error: cd: too many arguments" << endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      if(argNum==1){
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      if(strcmp(args[1],"-")==0){
          if(firstCD) std::cerr << "smash error: cd: OLDPWD not set" << endl;
          else {
              char* temp=get_current_dir_name();
              int chdirError=chdir( *plastPwd);
              if(!firstCD) free( *plastPwd);
              *plastPwd=(char*)malloc(strlen(temp)+1);
              strcpy(*plastPwd,temp);
              free(temp);
              if(chdirError==-1) perror("smash error: chdir failed");
          }
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      char* oldPath=get_current_dir_name();
      if (oldPath== nullptr) perror("smash error: get_current_dir_name failed");

      int chdirError=chdir(args[1]);
      if(chdirError==-1){
          perror("smash error: chdir failed");
      }
      else{
          if(!firstCD) free(*plastPwd);
          *plastPwd=(char*)malloc(strlen(oldPath)+1);
          strcpy(*plastPwd,oldPath);
      }
      if(firstCD) firstCD=false;
      for(int i=0; i<argNum; i++) free(args[i]);
      free(cmd);
      free(oldPath);

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
      free(path);
  };
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() = default;
  void execute() override{
      std::cout << "smash pid is " << getpid() << endl;
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
      bool isPipeCommand;

  public:
      JobEntry(int jobID, bool isStopped, time_t timeElapsed, int jobPid,char* cmd_line,bool isPipeCommand=false, bool isTimeOutCommand=false):
              jobID(jobID),isStopped(isStopped),timeElapsed(timeElapsed),
              jobPid(jobPid),isPipeCommand(isPipeCommand){
          this->cmd_line=(char*)malloc(sizeof(char)*(strlen(cmd_line)+1));
          strcpy(this->cmd_line,cmd_line);
      };
      ~JobEntry(){
          free(this->cmd_line);
      }

      void setJobID(int jobID){
          this->jobID=jobID;
      }
      //Copy Constructor
      JobEntry(const JobEntry& jobEntry){
          this->jobID=jobEntry.jobID;
          this->isStopped=jobEntry.isStopped;
          this->timeElapsed=jobEntry.timeElapsed;
          this->jobPid=jobEntry.jobPid;
          this->isPipeCommand=jobEntry.isPipeCommand;
          this->cmd_line=(char*)malloc(sizeof(char)*(strlen(jobEntry.cmd_line)+1));
          strcpy(this->cmd_line,jobEntry.cmd_line);
      };
      //Operator= Constructor
      JobEntry& operator=(const JobEntry& jobEntry){
          this->jobID=jobEntry.jobID;
          this->isStopped=jobEntry.isStopped;
          this->timeElapsed=jobEntry.timeElapsed;
          this->jobPid=jobEntry.jobPid;
          this->isPipeCommand=jobEntry.isPipeCommand;
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
          if(this->isStopped) std::cout << " (stopped)" << std::endl;
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
      bool getIsPipeCommand(){
          return this->isPipeCommand;
      }

      bool operator<(const JobEntry& jobEntry){
          if (this->jobID<jobEntry.jobID) return true;
          else return false;
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
    void addJob(int jobPid, char* cmd_line, bool isStopped = false, bool isPipeCommand=false, int foregroundJobID=-1){
        int jobID;
        if(foregroundJobID!=-1){
            jobID=foregroundJobID;
        }
        else if(this->jobsList->empty()) jobID=1;
        else {
            this->jobsList->sort();
            jobID=this->jobsList->back().getJobID()+1;
        }
        this->jobsList->push_back(JobEntry(jobID,isStopped,time(nullptr),jobPid,cmd_line,isPipeCommand));
    };
    void printJobsList(){
        this->jobsList->sort();
        for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end(); ++it){
            it->printJob();
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
          if(it->getIsPipeCommand()){
              if(kill(-(it->getJobPid()),SIGKILL)==-1) perror("smash error: kill failed");
          }
          else{
              if(kill(it->getJobPid(),SIGKILL)==-1) perror("smash error: kill failed");
          }
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

    std::list<JobEntry>* getJobsList(){
        return this->jobsList;
    }

    void removeFinishedJobs(){
        for(std::list<JobEntry>::iterator it=jobsList->begin(); it != jobsList->end();){
            int commandPid=it->getJobPid();
            //if(it->getIsPipeCommand()) commandPid=-commandPid;
            if(waitpid(commandPid,NULL,WNOHANG)==commandPid){
                it=this->jobsList->erase(it);
                if(it==this->jobsList->end()) break;
            }
            else {
                ++it;
            }
        }
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
      char* cmd=(char*)malloc(sizeof(char)*COMMAND_ARGS_MAX_LENGTH);
      strcpy(cmd,this->cmd_line);
      if(_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
      char *args[COMMAND_ARGS_MAX_LENGTH];
      int argNum=_parseCommandLine(cmd,args);
      if(argNum!=3){
          std::cerr << "smash error: kill: invalid arguments" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      int sigNum,jobNum;
      try{
          sigNum=abs(stoi(args[1]));
          jobNum=stoi(args[2]);
      }
      catch(const std::invalid_argument&){
          std::cerr << "smash error: kill: invalid arguments" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      string sigNumStr=args[1];
      if(sigNum<1 || sigNum>31 || sigNumStr.find("-")!=0){
          std::cerr << "smash error: kill: invalid arguments" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }

      JobsList::JobEntry* jobByID=this->jobsList->getJobById(jobNum);
      if(jobByID == nullptr){
          std::cerr << "smash error: kill: job-id " << jobNum <<" does not exist" << std::endl;
          for(int i=0; i<argNum; i++) free(args[i]);
          free(cmd);
          return;
      }
      int jobPid=jobByID->getJobPid();
      if(jobByID->getIsPipeCommand()) {
          if (kill(-jobPid, sigNum) == -1) perror("smash error: kill failed");
      }
      else {
          if (kill(jobPid, sigNum) == -1) perror("smash error: kill failed");
      }

      if(sigNum==SIGSTOP) jobByID->setIsStopped(true);
      if(sigNum==SIGCONT) jobByID->setIsStopped(false);
      std::cout << "signal number " << sigNum <<" was sent to pid " << jobPid << std::endl;

      for(int i=0; i<argNum; i++) free(args[i]);
      free(cmd);
  };
};



class TimeOutList;

class SmallShell {
private:
    // TODO: Add your data members
    char** plastPwd;
    JobsList* jobsList;
    int foregroundPid;
    char* foregroundCmdLine;
    bool isQuit;
    bool isPipeCommand;
    int foregroundJobID;
    TimeOutList* timeOutList;
    SmallShell();
public:
    Command *CreateCommand(const char* cmd_line, bool isPipe=false);
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
    bool getIsPipeCommand(){
        return this->isPipeCommand;
    }
    void setIsPipeCommand(bool isPipeCommand){
        this->isPipeCommand=isPipeCommand;
    }

    int getForegroundJobID(){
        return this->foregroundJobID;
    }

    void setForegroundJobID(int foregroundJobID){
        this->foregroundJobID=foregroundJobID;
    }

    TimeOutList* getTimeOutList(){
        return this->timeOutList;
    }

};



class ExternalCommand : public Command {
private:
    bool isPipe;
public:
    ExternalCommand(const char* cmd_line, bool isPipe=false):Command(cmd_line),isPipe(isPipe){};
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
            if(!(this->isPipe)) setpgrp();
            char* argv[] = {(char*)"/bin/bash", (char*)"-c", cmd, NULL};
            execv(argv[0], argv);
            perror("smash error: execv failed");
        }
            //Parent:
        else{
            this->pid=pid;
            SmallShell& smash=SmallShell::getInstance();
            if(!_isBackgroundComamnd(cmd_line)){
                smash.setForegroundPid(pid);
                smash.setForegroundCmdLine(this->cmd_line);
                smash.setForegroundJobID(-1);
                waitpid(pid,NULL,0 | WUNTRACED);
                smash.setForegroundPid(-1);
            }
            else smash.getJobsList()->addJob(this->pid,this->cmd_line,false);
        }
        free(cmd);
    };
};





// TODO: should it really inhirit from BuiltInCommand ?
class CopyCommand : public BuiltInCommand {
private:
    bool isPipe;
public:
    CopyCommand(const char *cmd_line, bool isPipe=false):BuiltInCommand(cmd_line),isPipe(isPipe){};

    virtual ~CopyCommand() = default;

    void execute() override {
        char *cmd = (char *) malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd, this->cmd_line);
        if (_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum = _parseCommandLine(cmd, args);
        if (argNum < 3) {
            std::cerr << "smash error: cp: invalid arguments" << std::endl;
            for (int i = 0; i < argNum; i++) free(args[i]);
            free(cmd);
            return;
        }


        SmallShell &smash = SmallShell::getInstance();
        pid_t pid = fork();
        if (pid == -1) perror("smash error: fork failed");
        //Child:
        if (pid == 0) {
            if(!(this->isPipe)) setpgrp();
            char buffer[1024];
            int files[2];
            ssize_t count;

            files[0] = open(args[1], O_RDONLY);
            if (files[0] == -1) {
                perror("smash error: open failed");
                for (int i = 0; i < argNum; i++) free(args[i]);
                free(cmd);
                smash.setIsQuit(true);
                return;
            }

            char realFile1[MAX_PATH];
            char realFile2[MAX_PATH];
            realpath(args[1], realFile1);
            realpath(args[2], realFile2);

            if(strcmp(realFile1,realFile2)==0){
                std::cout << "smash: "<<  args[1] << " was copied to " << args[2] << std::endl;
                for (int i = 0; i < argNum; i++) free(args[i]);
                free(cmd);
                smash.setIsQuit(true);
                close(files[0]);
                return;
            }



            files[1] = open(args[2], O_CREAT | O_RDWR | O_TRUNC, 0666);
            if (files[1] == -1) /* Check if file opened (permissions problems ...) */
            {
                perror("smash error: open failed");
                close(files[0]);
                for (int i = 0; i < argNum; i++) free(args[i]);
                free(cmd);
                smash.setIsQuit(true);
                return;
            }

            while ((count = read(files[0], buffer, sizeof(buffer))) != 0)
                if (write(files[1], buffer, count) == -1) {
                    perror("smash error: write failed");
                    break;
                }
            close(files[0]);
            close(files[1]);
            std::cout << "smash: "<<  args[1] << " was copied to " << args[2] << std::endl;
            for (int i = 0; i < argNum; i++) free(args[i]);
            free(cmd);
            smash.setIsQuit(true);
            return;
        }

            //Parent:
        else {
            this->pid = pid;
            SmallShell &smash = SmallShell::getInstance();
            if (!_isBackgroundComamnd(cmd_line)) {
                smash.setForegroundPid(pid);
                smash.setForegroundCmdLine(this->cmd_line);
                smash.setForegroundJobID(-1);
                waitpid(pid, NULL, 0 | WUNTRACED);
                smash.setForegroundPid(-1);
            } else smash.getJobsList()->addJob(this->pid, this->cmd_line, false);
        }

        for (int i = 0; i < argNum; i++) free(args[i]);
        free(cmd);

    }
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
        char *cmd = (char *) malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd, this->cmd_line);
        if (_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        bool invalidArg=false;
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(cmd,args);
        if(argNum==1){
            JobsList::JobEntry* lastJob=this->jobsList->getLastJob();
            if(lastJob == nullptr){
                std::cerr << "smash error: fg: jobs list is empty" << std::endl;
                for(int i=0; i<argNum; i++) free(args[i]);
                free(cmd);
                return;
            }

            std::cout << lastJob->getCmdLine() << " : " << lastJob->getJobPid() << std::endl;
            SmallShell& smash=SmallShell::getInstance();
            smash.setForegroundPid(lastJob->getJobPid());
            smash.setForegroundCmdLine(lastJob->getCmdLine());
            smash.setForegroundJobID(lastJob->getJobID());
            if(lastJob->getIsPipeCommand()) smash.setIsPipeCommand(true);

            if(lastJob->getIsStopped()){
                if(lastJob->getIsPipeCommand()){
                    if(kill(-(lastJob->getJobPid()),SIGCONT)==-1){
                        perror("smash error: kill failed");
                        for(int i=0; i<argNum; i++) free(args[i]);
                        free(cmd);
                        return;
                    }
                }
                else{
                    if(kill(lastJob->getJobPid(),SIGCONT)==-1){
                        perror("smash error: kill failed");
                        for(int i=0; i<argNum; i++) free(args[i]);
                        free(cmd);
                        return;
                    }
                }
                lastJob->setIsStopped(false);
            }
            int lastJobPid=lastJob->getJobPid();
            this->jobsList->removeJobById(lastJob->getJobID());
            if(waitpid(lastJobPid,NULL,0 | WUNTRACED)==-1){
                perror("smash error: waitpid failed");
                for(int i=0; i<argNum; i++) free(args[i]);
                free(cmd);
                return;
            }
            smash.setForegroundPid(-1);
        }
        if(argNum==2) {
            int jobID;
            try{
                jobID=stoi(args[1]);
            }
            catch(const std::invalid_argument&){
                invalidArg=true;
            }
            if(!invalidArg){
                JobsList::JobEntry* job=this->jobsList->getJobById(jobID);
                if(job == nullptr){
                    std::cerr << "smash error: fg: job-id " << jobID << " does not exist" << std::endl;
                    for(int i=0; i<argNum; i++) free(args[i]);
                    free(cmd);
                    return;
                }
                else{

                    std::cout << job->getCmdLine() << " : " << job->getJobPid() << std::endl;
                    SmallShell& smash=SmallShell::getInstance();
                    smash.setForegroundPid(job->getJobPid());
                    smash.setForegroundCmdLine(job->getCmdLine());
                    smash.setForegroundJobID(job->getJobID());
                    if(job->getIsPipeCommand()) smash.setIsPipeCommand(true);
                    if(job->getIsStopped()){
                        if(job->getIsPipeCommand()){
                            if(kill(-(job->getJobPid()),SIGCONT)==-1){
                                perror("smash error: kill failed");
                                for(int i=0; i<argNum; i++) free(args[i]);
                                free(cmd);
                                return;
                            }
                        }
                        else{
                            if(kill(job->getJobPid(),SIGCONT)==-1){
                                perror("smash error: kill failed");
                                for(int i=0; i<argNum; i++) free(args[i]);
                                free(cmd);
                                return;
                            }
                        }
                        job->setIsStopped(false);
                    }
                    int jobPid=job->getJobPid();
                    this->jobsList->removeJobById(job->getJobID());
                    if(waitpid(jobPid,NULL,0 | WUNTRACED)==-1){
                        perror("smash error: waitpid failed");
                        for(int i=0; i<argNum; i++) free(args[i]);
                        free(cmd);
                        return;
                    }
                    smash.setForegroundPid(-1);
                }
            }
        }
        if((argNum!=1 && argNum!=2) || invalidArg){
            std::cerr << "smash error: fg: invalid arguments" << std::endl;
        }
        for(int i=0; i<argNum; i++) free(args[i]);
        free(cmd);

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
        char *cmd = (char *) malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd, this->cmd_line);
        if (_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        bool invalidArg=false;
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(cmd,args);
        if(argNum==1){
            JobsList::JobEntry* lastStoppedJob=this->jobsList->getLastStoppedJob();
            if(lastStoppedJob == nullptr){
                std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
                for(int i=0; i<argNum; i++) free(args[i]);
                free(cmd);
                return;
            }
            std::cout << lastStoppedJob->getCmdLine() << " : " << lastStoppedJob->getJobPid() << std::endl;

            if(lastStoppedJob->getIsPipeCommand()){
                if(kill(-(lastStoppedJob->getJobPid()),SIGCONT)==-1){
                    perror("smash error: kill failed");
                    for(int i=0; i<argNum; i++) free(args[i]);
                    free(cmd);
                    return;
                }
            }
            else{
                if(kill(lastStoppedJob->getJobPid(),SIGCONT)==-1){
                    perror("smash error: kill failed");
                    for(int i=0; i<argNum; i++) free(args[i]);
                    free(cmd);
                    return;
                }
            }
            lastStoppedJob->setIsStopped(false);
        }
        if(argNum==2) {
            int jobID;
            try{
                jobID=stoi(args[1]);
            }
            catch(const std::invalid_argument&){
                invalidArg=true;
            }
            if(!invalidArg){
                JobsList::JobEntry* stoppedJob=this->jobsList->getJobById(jobID);
                if(stoppedJob == nullptr){
                    std::cerr << "smash error: bg: job-id " << jobID << " does not exist" << std::endl;
                    for(int i=0; i<argNum; i++) free(args[i]);
                    free(cmd);
                    return;
                }
                else{
                    if(stoppedJob->getIsStopped()){
                        std::cout << stoppedJob->getCmdLine() << " : " << stoppedJob->getJobPid() << std::endl;
                        if(stoppedJob->getIsPipeCommand()){
                            if(kill(-(stoppedJob->getJobPid()),SIGCONT)==-1){
                                perror("smash error: kill failed");
                                for(int i=0; i<argNum; i++) free(args[i]);
                                free(cmd);
                                return;
                            }
                            stoppedJob->setIsStopped(false);
                        }
                        else{
                            if(kill(stoppedJob->getJobPid(),SIGCONT)==-1){
                                perror("smash error: kill failed");
                                for(int i=0; i<argNum; i++) free(args[i]);
                                free(cmd);
                                return;
                            }
                            stoppedJob->setIsStopped(false);
                        }
                    }
                    else{
                        std::cerr << "smash error: bg: job-id "<< stoppedJob->getJobID() <<" is already running in the background" << std::endl;
                    }
                }
            }
        }
        if((argNum!=1 && argNum!=2) || invalidArg){
            std::cerr << "smash error: bg: invalid arguments" << std::endl;
        }
        for(int i=0; i<argNum; i++) free(args[i]);
        free(cmd);

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
        char *cmd = (char *) malloc(sizeof(char) * COMMAND_ARGS_MAX_LENGTH);
        strcpy(cmd, this->cmd_line);
        if (_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmd);
        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum=_parseCommandLine(cmd,args);
        bool isKill=false;
        for(int i=1; i<argNum; i++){
            if(strcmp(args[i],"kill")==0) isKill=true;
        }
        if(argNum>=2 && isKill){
            std::cout << "smash: sending SIGKILL signal to "<<  this->jobsList->getJobsListSize() << " jobs:" << std::endl;
            this->jobsList->killAllJobs();
        }
        for(int i=0; i<argNum; i++){
            free(args[i]);
        }
        free(cmd);

        SmallShell& smash=SmallShell::getInstance();
        smash.setIsQuit(true);
        smash.getJobsList()->removeFinishedJobs();
    };
};


class PipeCommand : public Command {
    // TODO: Add your data members
private:
    bool isPipe;
public:
    PipeCommand(const char* cmd_line,bool isPipe):Command(cmd_line),isPipe(isPipe){};
    virtual ~PipeCommand() = default;
    void execute() override{
        string cmd_s = string(cmd_line);
        bool isStdError = false, isBackground=false;
        if(cmd_s.find("|&")!=string::npos) isStdError=true;
        char *argsCommand1[COMMAND_ARGS_MAX_LENGTH];
        char *argsCommand2[COMMAND_ARGS_MAX_LENGTH];
        int argsNumCommand1,argsNumCommand2;

        int pos=cmd_s.find("|");
        string command1Cmd=cmd_s.substr(0,pos);
        string command2Cmd;
        if(isStdError) command2Cmd=cmd_s.substr(pos+2);
        else command2Cmd=cmd_s.substr(pos+1);
        char command1CmdCStr[command1Cmd.size()];
        strcpy(command1CmdCStr,command1Cmd.c_str());
        char command2CmdCStr[command2Cmd.size()];
        strcpy(command2CmdCStr,command2Cmd.c_str());
        argsNumCommand1=_parseCommandLine(command1CmdCStr,argsCommand1);
        argsNumCommand2=_parseCommandLine(command2CmdCStr,argsCommand2);
        if(_isBackgroundComamnd(command1CmdCStr)) _removeBackgroundSign(command1CmdCStr);
        if(_isBackgroundComamnd(command2CmdCStr)){
            _removeBackgroundSign(command2CmdCStr);
            isBackground=true;
        }
        SmallShell& smash=SmallShell::getInstance();
        pid_t smashPid=getpid();

        //Check if |& Command
        if(cmd_s.find("|&")!=string::npos) isStdError=true;

        pid_t pidChildren=fork();
        if(pidChildren==-1) perror("smash error: fork failed");
        //Children:
        if(pidChildren==0){
            if(!isPipe) setpgrp();
            int fd[2];
            pipe(fd);
            pid_t pidChild1=fork();
            if(pidChild1==-1) perror("smash error: fork failed");
            //Child1:
            if(pidChild1==0){
                if(isStdError)dup2(fd[1],2);
                else dup2(fd[1],1);
                close(fd[0]);
                close(fd[1]);
                Command* cmd1=smash.CreateCommand(command1CmdCStr,true);
                if(strcmp(argsCommand1[0],"showpid")==0) std::cout << "smash pid is " << smashPid << endl;
                else cmd1->execute();
                delete cmd1;
                smash.setIsQuit(true);
                for(int i=0; i<argsNumCommand1; i++) free(argsCommand1[i]);
                for(int i=0; i<argsNumCommand2; i++) free(argsCommand2[i]);
                return;
                //exit(0);
            }
            //Parent:
            pid_t pidChild2=fork();
            if(pidChild2==-1) perror("smash error: fork failed");
            //Child2:
            if(pidChild2==0){
                dup2(fd[0],0);
                close(fd[0]);
                close(fd[1]);
                Command* cmd2=smash.CreateCommand(command2CmdCStr,true);
                if(strcmp(argsCommand2[0],"showpid")==0) std::cout << "smash pid is " << smashPid << endl;
                else cmd2->execute();
                delete cmd2;
                smash.setIsQuit(true);
                for(int i=0; i<argsNumCommand1; i++) free(argsCommand1[i]);
                for(int i=0; i<argsNumCommand2; i++) free(argsCommand2[i]);
                return;
                //exit(0);
            }
            //Parent:
            close(fd[0]);
            close(fd[1]);
            waitpid(pidChild1,NULL,0);
            waitpid(pidChild2,NULL,0);
            smash.setIsQuit(true);
            for(int i=0; i<argsNumCommand1; i++) free(argsCommand1[i]);
            for(int i=0; i<argsNumCommand2; i++) free(argsCommand2[i]);
            return;
            //exit(0);
        }
        //Grandparent:
        else{
            this->pid=pidChildren;
            if(!isBackground){
                smash.setForegroundPid(pid);
                smash.setForegroundCmdLine(this->cmd_line);
                smash.setForegroundJobID(-1);
                waitpid(pid,NULL,WUNTRACED);
                smash.setForegroundPid(-1);
            }
            else{
                smash.getJobsList()->addJob(this->pid,this->cmd_line,false,true);
            }
        }

        for(int i=0; i<argsNumCommand1; i++) free(argsCommand1[i]);
        for(int i=0; i<argsNumCommand2; i++) free(argsCommand2[i]);

    };
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
private:
    bool isPipe;
public:
    explicit RedirectionCommand(const char* cmd_line, bool isPipe=false):Command(cmd_line),isPipe(isPipe){};
    virtual ~RedirectionCommand() = default;
    void execute() override{
        string cmd_s = string(cmd_line);
        char *argsPath[COMMAND_ARGS_MAX_LENGTH];
        char *argsCommand[COMMAND_ARGS_MAX_LENGTH];
        int argNumPath,argsNumCommand;
        bool isAppend=true;
        //Check if > Command
        if(cmd_s.find(">>")==string::npos) isAppend=false;

        int pos=cmd_s.find(">");
        string commandCmd=cmd_s.substr(0,pos);
        string filePath;
        if(isAppend) filePath=cmd_s.substr(pos+2);
        else filePath=cmd_s.substr(pos+1);
        char commandCmdCStr[commandCmd.size()];
        strcpy(commandCmdCStr,commandCmd.c_str());
        char filePathCStr[filePath.size()];
        strcpy(filePathCStr,filePath.c_str());
        argNumPath=_parseCommandLine(filePathCStr,argsPath);
        argsNumCommand=_parseCommandLine(commandCmdCStr,argsCommand);
        if(_isBackgroundComamnd(commandCmdCStr)) _removeBackgroundSign(commandCmdCStr);
        bool isBackground=false;
        if(_isBackgroundComamnd(filePathCStr)){
            _removeBackgroundSign(filePathCStr);
            isBackground=true;
        }
        pid_t pid=fork();
        if(pid==-1) perror("smash error: fork failed");
        //Child:
        if(pid==0){
            SmallShell& smash=SmallShell::getInstance();
            if(!(this->isPipe)) setpgrp();
            if(close(1)==-1) perror("smash error: close failed");
            int file;
            if(isAppend) file=open(argsPath[0],O_CREAT | O_RDWR | O_APPEND ,0666);
            else file=open(argsPath[0],O_CREAT | O_RDWR | O_TRUNC ,0666);
            if(file==-1){
                perror("smash error: open failed");
                for(int i=0; i<argNumPath; i++) free(argsPath[i]);
                for(int i=0; i<argsNumCommand; i++) free(argsCommand[i]);
                smash.setIsQuit(true);
                return;
            }
            Command* cmd;
            if(this->isPipe) cmd=smash.CreateCommand(commandCmdCStr,true);
            else cmd=smash.CreateCommand(commandCmdCStr);
            if(strcmp(argsCommand[0],"showpid")==0) std::cout << "smash pid is " << getppid() << endl;
            else cmd->execute();
            delete cmd;
            smash.setIsQuit(true);
            close(file);
            for(int i=0; i<argNumPath; i++) free(argsPath[i]);
            for(int i=0; i<argsNumCommand; i++) free(argsCommand[i]);
            return;
            //exit(0);
        }
        //Parent:
        else{
            this->pid=pid;
            SmallShell& smash=SmallShell::getInstance();
            if(!isBackground){
                smash.setForegroundPid(pid);
                smash.setForegroundCmdLine(this->cmd_line);
                waitpid(pid,NULL,0 | WUNTRACED);
                smash.setForegroundPid(-1);
            }
            else smash.getJobsList()->addJob(this->pid,this->cmd_line,false);
        }

        for(int i=0; i<argNumPath; i++) free(argsPath[i]);
        for(int i=0; i<argsNumCommand; i++) free(argsCommand[i]);
    };
    //void prepare() override;
    //void cleanup() override;
};




//********************************************************************************************************************************************************
class TimeOutList {
public:
    class TimeOutEntry {
        // TODO: Add your data members
    private:
        int commandPid;
        time_t timeBegin;
        int duration;
        char *cmd_line;

    public:
        TimeOutEntry(int commandPid, int duration, char *cmd_line) : commandPid(commandPid), duration(duration) {
            this->cmd_line = (char *) malloc(sizeof(char) * (strlen(cmd_line) + 1));
            strcpy(this->cmd_line, cmd_line);
            this->timeBegin = time(nullptr);
        };
        ~TimeOutEntry(){
            free(this->cmd_line);
        }

        //Copy Constructor
        TimeOutEntry(const TimeOutEntry &timeOutEntry) {
            this->commandPid = timeOutEntry.commandPid;
            this->timeBegin = timeOutEntry.timeBegin;
            this->duration = timeOutEntry.duration;
            this->cmd_line = (char *) malloc(sizeof(char) * (strlen(timeOutEntry.cmd_line) + 1));
            strcpy(this->cmd_line, timeOutEntry.cmd_line);
        };

        //Operator= Constructor
        TimeOutEntry &operator=(const TimeOutEntry &timeOutEntry) {
            this->commandPid = timeOutEntry.commandPid;
            this->timeBegin = timeOutEntry.timeBegin;
            this->duration = timeOutEntry.duration;
            this->cmd_line = (char *) malloc(sizeof(char) * (strlen(timeOutEntry.cmd_line) + 1));
            strcpy(this->cmd_line, timeOutEntry.cmd_line);
            return *this;
        };

        int getcommandPid() {
            return this->commandPid;
        }

        time_t getTimeBegin() {
            return this->timeBegin;
        }

        char *getCmdLine() {
            return this->cmd_line;
        }

        int getDuration() {
            return this->duration;
        }

        bool operator<(const TimeOutEntry &timeOutEntry) {
            time_t now = time(nullptr);
            double time1 = this->duration - difftime(now, this->timeBegin);
            double time2 = timeOutEntry.duration - difftime(now, timeOutEntry.timeBegin);
            if (time1 < time2) return true;
            return false;
        }

    };


    // TODO: Add your data members
private:
    std::list<TimeOutEntry> *timeOutList;
public:
    TimeOutList() : timeOutList(new std::list<TimeOutEntry>) {};

    ~TimeOutList() {
        delete timeOutList;
    };

    void addTimeOut(int commandPid, time_t timeBegin, int duration, char *cmd_line) {
        this->timeOutList->push_back(TimeOutEntry(commandPid, duration, cmd_line));
        this->timeOutList->sort();
        time_t now = time(nullptr);
        double time = this->timeOutList->front().getDuration()-difftime(now,this->timeOutList->front().getTimeBegin());
        alarm(round(time));
    };

    void removeFinishedTimeOutAlarm() {
        for(std::list<TimeOutEntry>::iterator it=timeOutList->begin(); it != timeOutList->end();){
            int commandPid=it->getcommandPid();
            if(waitpid(commandPid,NULL,WNOHANG)==-1){
                it=this->timeOutList->erase(it);
                if(it==this->timeOutList->end()) break;
            }
            else {
                it++;
            }
        }
    }

    bool isEmptyTimeOutList(){
        return this->timeOutList->empty();
    }

    void sortTimeOutList(){
        this->timeOutList->sort();
    }

    int getAlarmedTimeOutParamsAndDelete(char* cmd){
        time_t now = time(nullptr);
        for(std::list<TimeOutEntry>::iterator it=timeOutList->begin(); it != timeOutList->end(); it++){
            double time = it->getDuration()-difftime(now,it->getTimeBegin());
            if(round(time)<=0){
                strcpy(cmd,it->getCmdLine());
                int pid=it->getcommandPid();
                this->timeOutList->erase(it);
                return pid;
            }
        }
        return -1;
    }

    TimeOutEntry& getFirstTimeOutAlarm(){
        return this->timeOutList->front();
    }

};

class TimeOutCommand : public Command {
public:
    TimeOutCommand(const char* cmd_line):Command(cmd_line){};
    virtual ~TimeOutCommand() = default;

    //Copy Constructor
    TimeOutCommand(const TimeOutCommand& command) = default;
    //Operator= Constructor
    TimeOutCommand& operator=(const TimeOutCommand& command)= default;

    void execute() override{
        char cmdNoBackground[COMMAND_ARGS_MAX_LENGTH];
        strcpy(cmdNoBackground,this->cmd_line);
        if(_isBackgroundComamnd(cmd_line)) _removeBackgroundSign(cmdNoBackground);

        char *args[COMMAND_ARGS_MAX_LENGTH];
        int argNum = _parseCommandLine(cmdNoBackground, args);
        if(argNum<3 || stoi(args[1])<0){
            std::cerr << "smash error: timeout: invalid arguments" << std::endl;
            for (int i = 0; i < argNum; i++) free(args[i]);
            return;
        }
        string commandCmd;
        for (int i = 2; i < argNum; i++){
            commandCmd.append(args[i]);
            if(i!=argNum-1) commandCmd.append(" ");
        }

        char commandCmdCStr[commandCmd.size()];
        strcpy(commandCmdCStr,commandCmd.c_str());

        pid_t pid=fork();
        if(pid==-1) perror("smash error: fork failed");
        //Child:
        if(pid==0){
            setpgrp();
            SmallShell& smash=SmallShell::getInstance();
            Command* cmd=smash.CreateCommand(commandCmdCStr,true);
            if(strcmp(args[2],"showpid")==0) std::cout << "smash pid is " << getppid() << endl;
            else cmd->execute();
            delete cmd;
            smash.setIsQuit(true);
            for (int i = 0; i < argNum; i++) free(args[i]);
            return;
        }
            //Parent:
        else{
            this->pid=pid;
            int duration=stoi(args[1]);
            SmallShell& smash=SmallShell::getInstance();
            smash.getTimeOutList()->addTimeOut(this->pid,time(nullptr),duration,this->cmd_line);
                if(!_isBackgroundComamnd(cmd_line)){
                    smash.setIsPipeCommand(true);
                    smash.setForegroundPid(pid);
                    smash.setForegroundCmdLine(this->cmd_line);
                    smash.setForegroundJobID(-1);
                    waitpid(pid,NULL,0 | WUNTRACED);
                    smash.setForegroundPid(-1);
                }
                else smash.getJobsList()->addJob(this->pid,this->cmd_line,false,true);
        }
        for (int i = 0; i < argNum; i++) free(args[i]);
    };
};

//********************************************************************************************************************************************************







#endif //SMASH_COMMAND_H_
