#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

using namespace std;
extern string promptName;

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction new_action;
    new_action.sa_handler=alarmHandler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;
    if(sigaction (SIGALRM, &new_action, NULL) < 0){
        perror("smash error: failed to set alarm handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    smash.setIsQuit(false);

    while(smash.getIsQuit()== false) {
    std::cout << promptName; // TODO: change this (why?)
    std::string cmd_line;
    std::getline(std::cin, cmd_line);
    smash.getJobsList()->removeFinishedJobs();
    smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}