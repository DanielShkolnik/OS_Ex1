#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    std::cout << "smash: got ctrl-Z" << std::endl;
    SmallShell& smash=SmallShell::getInstance();
    if(smash.getForegroundPid()!=-1){
        if(smash.getIsPipeCommand()){
            if( kill(-(smash.getForegroundPid()),SIGSTOP)==-1) perror("smash error: kill failed");
            smash.getJobsList()->addJob(smash.getForegroundPid(),smash.getForegroundCmdLine(),true,true,smash.getForegroundJobID());
        } else{
            if( kill(smash.getForegroundPid(),SIGSTOP)==-1) perror("smash error: kill failed");
            smash.getJobsList()->addJob(smash.getForegroundPid(),smash.getForegroundCmdLine(),true,false,smash.getForegroundJobID());
        }
        std::cout << "smash: process " << smash.getForegroundPid() << " was stopped" << std::endl;
        smash.setForegroundPid(-1);
        smash.setForegroundJobID(-1);
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got ctrl-C" << std::endl;
    SmallShell& smash=SmallShell::getInstance();

    if(smash.getForegroundPid()!=-1){
        if(smash.getIsPipeCommand()){
            if( kill(-(smash.getForegroundPid()),SIGKILL)==-1) perror("smash error: kill failed");
        } else{
            if( kill(smash.getForegroundPid(),SIGKILL)==-1) perror("smash error: kill failed");
        }
        std::cout << "smash: process " << smash.getForegroundPid() << " was killed" << std::endl;
        smash.setForegroundPid(-1);
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got an alarm" << std::endl;
    SmallShell& smash=SmallShell::getInstance();
    smash.getJobsList()->removeFinishedJobs();
    char cmd[COMMAND_ARGS_MAX_LENGTH];
    smash.getTimeOutList()->removeFinishedTimeOutAlarm();
    int pid=smash.getTimeOutList()->getAlarmedTimeOutParamsAndDelete(cmd);
    if(pid!=-1 && waitpid(pid,NULL,WNOHANG)!=-1){
        if(kill(-pid,SIGKILL)==-1) perror("smash error: kill failed");
        smash.getTimeOutList()->sortTimeOutList();
        std::cout << "smash: " << cmd << " timed out!" << std::endl;
    }
    if(!smash.getTimeOutList()->isEmptyTimeOutList()){
        time_t now=time(nullptr);
        double time = smash.getTimeOutList()->getFirstTimeOutAlarm().getDuration()-difftime(now,smash.getTimeOutList()->getFirstTimeOutAlarm().getTimeBegin());
        alarm(round(time));
    }
}

