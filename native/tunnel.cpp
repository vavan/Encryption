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


Addr parse_addr(string input) {
	int has_colon = input.find(":");
	if (has_colon > 0) {
		return Addr(input.substr(0, has_colon), atoi(input.substr(has_colon + 1).c_str()));
	} else {
		return Addr("127.0.0.1", atoi(input.c_str()));
	}
}

bool build_config(int argc, char* argv[]) {
	if (argc != 3) {
		cerr << "USAGE: remoute_addr:port local_listener:port" << endl;
		return false;
	}
	Config::init();

	string r = argv[1];
	string l = argv[2];

	Config::get().server = parse_addr(l);
	Config::get().client = parse_addr(r);

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
#if 1
//	Listener* main = new Listener(&w, new NormalSocket(Config::get().server));
	SecureSocket* ss = new SecureSocket(Addr("127.0.0.1", 5689));
	ss->set_security("test.crt","test.key");
	Listener* main = new Listener(&w, ss);

	main->init();
#else
	SecureSocket ss(Addr("127.0.0.1", 5689));
	ClientPipe* main = new ClientPipe(&w, &ss);
	main->init();
	char data[] = "OK";
	Buffer b; b.assign(data, data+2);
	main->push(&b);
#endif
	running = true;
	while (running) {
		w.run();
	}

	delete main;
	LOG.noticeStream() << "Exit";
	Config::done();
    return 0;
}

