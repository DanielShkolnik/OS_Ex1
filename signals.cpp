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
        } else{
            if( kill(smash.getForegroundPid(),SIGSTOP)==-1) perror("smash error: kill failed");
        }
        smash.getJobsList()->addJob(smash.getForegroundPid(),smash.getForegroundCmdLine(),true);
        std::cout << "smash: process " << smash.getForegroundPid() << " was stopped" << std::endl;
        smash.setForegroundPid(-1);
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

}

