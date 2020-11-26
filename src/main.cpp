/*
 * main.cpp
 *
 *  Created on: Oct 19, 2020
 *      Author: amira
 */

#include <iostream>
#include <csignal>
#include "userinterface.h"

using namespace std;

void signalHandler(int signum) {
	exit(signum);
}

int main() {
	std::signal(SIGINT, signalHandler);
	Ui *interface = new Ui();
	return interface->run();
}

