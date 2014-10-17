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
		cerr << "USAGE: <s><local_listener>:port <s><remoute_addr>:port" << endl;
		return false;
	}
	Config::init();
	Config::get().key_file = "key.pem";
	Config::get().crt_file = "cert.pem";

	string inb = argv[1];
	string r = argv[2];

	if (inb[0] == 's') {
		Config::get().inbound_secure = true;
		Config::get().inbound = parse_addr(inb.substr(1, inb.size()));
	} else {
		Config::get().inbound = parse_addr(inb);
	}
	if (r[0] == 's') {
		Config::get().outbound_secure = true;
		Config::get().outbound = parse_addr(r.substr(1, r.size()));
	} else {
		Config::get().outbound = parse_addr(r);
	}

	return true;
}

Worker* w;

void terminationHandler(int sig)
{
	LOG.noticeStream() << "MAIN. Got signal:" << sig;
	w->running = false;
}

int main(int argc, char* argv[]) {
	init_log();
	LOG.noticeStream() << "Start tunnel. Version:" << MAJ_VERSION << "." << MIN_VERSION;

	if (!build_config(argc, argv)) {
		return 1;
	}
	w = new Worker();
	signal(SIGTERM, terminationHandler);


	Socket* s;
	if (Config::get().inbound_secure) {
		s = new SecureSocket(Config::get().inbound);
	} else {
		s = new NormalSocket(Config::get().inbound);
	}
	Listener* main = new Listener(w, s);
	main->init();


	while (w->running) {
		w->run();
	}

	delete w;
	LOG.noticeStream() << "Exit";
	Config::done();
    return 0;
}

