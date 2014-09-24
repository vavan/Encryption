#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "connection.h"
#include "config.h"
#include "proxy.h"


using namespace std;


bool build_config(int argc, char* argv[]) {
	if (argc < 4) {
		cerr << "USAGE: remoute_addr:port local_listener:port" << endl;
		return false;
	}
	Config::init();

	string r = argv[1];
	string l = argv[2];

	Config::get().server = Addr(l.substr(0, l.find(":")), atoi(l.substr(l.find(":")+1).c_str()));
	Config::get().client = Addr(r.substr(0, r.find(":")), atoi(r.substr(r.find(":")+1).c_str()));

	return true;
}

int main(int argc, char* argv[]) {
	init_log();
	LOG << "Start v:" << MAJ_VERSION << "." << MIN_VERSION;
	if (!build_config(argc, argv)) {
		return 1;
	}

	Socket* socket = new Socket(Config::get().client);

	Worker w = Worker();
	Listener* l = new Listener(&w, socket);
	l->init();

	while (!w.empty()) {
		w.run();
	}

	LOG << "Exit";
	Config::done();
    return 0;
}

