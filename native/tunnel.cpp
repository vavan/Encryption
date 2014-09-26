#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "worker.h"

#include "config.h"
#include "proxy.h"
#include "normal_socket.h"
#include "secure_socket.h"


using namespace std;



bool build_config(int argc, char* argv[]) {
	if (argc != 3) {
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

bool running;

void terminationHandler(int sig)
{
	LOG.debugStream() << "MAIN. Got signal:" << sig;
	running = false;
}

int main(int argc, char* argv[]) {
	init_log();
	LOG.noticeStream() << "Start tunnel. Version:" << MAJ_VERSION << "." << MIN_VERSION;

	if (!build_config(argc, argv)) {
		return 1;
	}
	signal(SIGTERM, terminationHandler);

	Worker w = Worker();
//	Listener* listener = new Listener(&w, new NormalSocket(Config::get().server));
//	listener->init();

	SecureSocket ss(Addr("127.0.0.1", 5689));
//	ss.connect();

	ClientPipe cp = ClientPipe(&w, &ss);
	cp.init();

	char data[] = "OK";
//	ss.send(data, 2);
	Buffer b; b.assign(data, data+2);
	cp.push(&b);

	running = true;
	while (running) {
		w.run();
	}

//	delete listener;
	LOG.noticeStream() << "Exit";
	Config::done();
    return 0;
}

