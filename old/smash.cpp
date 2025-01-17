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

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    char** plastPwd = (char **) malloc(sizeof(char *));
    *plastPwd = nullptr;
    JobsList* jobsList=new JobsList;
    while(true) {
        std::cout << promptName;
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str(),plastPwd,jobsList);
    }
    free(*plastPwd);
    free(plastPwd);
    return 0;
}