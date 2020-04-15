#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    std::cout << "smash: got ctrl-Z" << std::endl;
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got ctrl-C" << std::endl;
    int pid=getpid();
    int killValue=kill(pid,SIGKILL);
    std::cout << "smash: process " << pid << " was killed" << std::endl;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

