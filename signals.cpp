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
        std::cout << "IsPipeCommand: " << smash.getIsPipeCommand() << std::endl;
        if(smash.getIsPipeCommand()){
            std::cout << "PipeCommand1: " << smash.getPipeCommand1() << std::endl;
            std::cout << "PipeCommand2: " << smash.getPipeCommand2() << std::endl;
            if(kill(smash.getPipeCommand1(),SIGSTOP)==-1) perror("smash error: kill failed");
            if(kill(smash.getPipeCommand2(),SIGSTOP)==-1) perror("smash error: kill failed");
        }
        if( kill(smash.getForegroundPid(),SIGSTOP)==-1) perror("smash error: kill failed");
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
            if(kill(smash.getPipeCommand1(),SIGKILL)==-1) perror("smash error: kill failed");
            if(kill(smash.getPipeCommand2(),SIGKILL)==-1) perror("smash error: kill failed");
        }
        if( kill(smash.getForegroundPid(),SIGKILL)==-1) perror("smash error: kill failed");
        std::cout << "smash: process " << smash.getForegroundPid() << " was killed" << std::endl;
        smash.setForegroundPid(-1);
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation

}

