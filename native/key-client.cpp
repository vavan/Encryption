#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "connection.h"
#include "security.h"
#include "config.h"
#include "proxy.h"


using namespace std;



void start_child() {
	int fork_rv = fork();
	if (fork_rv == 0)
	{
		char** cmd = Config::get().child;
	    // we're in the child
	    if (execv(cmd[0], cmd+1) == -1) {
	    	LOG << "Unable to start child, errno: " << errno;
	    }
	}
	else if (fork_rv == -1)
	{
	    LOG << "***ERROR: could not fork, returns: " << fork_rv;
	}
	//parent, run normally
}

class LocalPipe : public Pipe {
	Socket* other_socket;
protected:
	//Buffer* recv() {
	void recv() {
		LOG << "Accept local";
		Socket* accepted = this->socket->accept();
		Pipe* sp = new Pipe(this->parent, accepted);
		Pipe* cp = new Pipe(this->parent, this->other_socket);
		sp->join(cp);
		sp->init();
		cp->init();
		this->closed = true;
		//return &recv_buffer;
	}
public:
	LocalPipe(Worker* parent, Socket* socket, Socket* other_socket): Pipe(parent, socket), other_socket(other_socket) {};
	void init() {
		LOG << "Listen on: " << this->socket->addr.str();
		this->socket->listen();

		start_child();
	};
};


class KeyManager: public Point {

	Security sec;
public:
	KeyManager(Worker* parent, Socket* socket): Point(parent, socket) {
	}
	Buffer construct_request() {
		char* msg = sec.pub_key;
		int len = strlen(sec.pub_key);
		Buffer request;
		request.reserve(4096);
		request.assign(msg, msg + len - 1);
		return request;
	}
	void send_public() {
		LOG << "Send public to " << this->socket->addr.str();
		Buffer request = construct_request();

		this->socket->connect();
		this->send(&request);
	}
	void save_shared_secret(string decrypted) {
		const char* final_marker = "-----END RSA PRIVATE KEY-----";
		ofstream out("key.pem");
		int endof = decrypted.find(final_marker);
		decrypted.erase(endof+strlen(final_marker)+1);
		out << decrypted;
	}
	void recv_private(Buffer* replay) {
		LOG << "Receive private size: " << replay->size();
//		int s = replay->size();
//		char* b = (char*)malloc(s);
//		memcpy(b, &replay[0], s);
//		int f = open("encr.data", O_WRONLY);
//		write(f, b, s);
//		close(f);
//		ofstream out("encr.data");
//		out << *replay;

		string decrypted = sec.decrypt(&(*replay)[0], replay->size());
		save_shared_secret(decrypted);

		start_proxy();
	}
	void start_proxy() {
		LOG << "Start proxy";
		this->closed = true;
		LocalPipe* sp = new LocalPipe(this->parent, new Socket(Config::get().server), this->relese_socket());
		sp->init();
	}
	void init() {
		sec.build();
		send_public();
	}
	void on_recv(Buffer* data) {
		recv_private(data);
	}
};

bool build_config(int argc, char* argv[]) {
	if (argc < 4) {
		cerr << "USAG: remoute_addr:port local_listener:port ffmpeg_with_parameters" << endl;
		return false;
	}
	Config::init();

	string r = argv[1];
	string l = argv[2];

	Config::get().server = Addr(l.substr(0, l.find(":")), atoi(l.substr(l.find(":")+1).c_str()));
	Config::get().client = Addr(r.substr(0, r.find(":")), atoi(r.substr(r.find(":")+1).c_str()));
	int child_argc = argc - 3;
	char** child = (char**)malloc(sizeof(char*)*(child_argc + 1));
	memcpy(child, argv+3, sizeof(char*)*child_argc);
	child[child_argc] = NULL;

	Config::get().child = child;

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
	KeyManager* km = new KeyManager(&w, socket);
	km->init();

	while (!w.empty()) {
		w.run();
	}

	LOG << "Exit";
	Config::done();
    return 0;
}

