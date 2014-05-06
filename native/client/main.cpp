#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include "connection.h"
#include "security.h"
#include "config.h"
#include "proxy.h"


using namespace std;





//void test() {
//	Config::init();
//	Config::get().server = Addr("0.0.0.0", 8080);
//	Config::get().client = Addr("0.0.0.0", 1935);
//
//	Security sec = Security();
//	sec.build();
//
//	Socket s = Socket(Addr("127.0.0.1", 8080));
//	s.connect();
//
//	int hi_size = strlen(hi) + 2 + strlen(sec.pub_key);
//	char* hi_msg = (char*)malloc(hi_size);
//
//	cout << "Pub:" << sec.pub_key << endl;
//
//	create_hi(hi_msg, sec.pub_key);
//	int sent = s.send(hi_msg, hi_size-1);
//
//	cout << "Sent: " << sent << endl;
//
//	char* buffer = (char *)malloc(16384);
//	int recv = s.recv(buffer, 16384);
//
//	cout << "Recv: " << recv << endl;
//
//	char ack[] = "Heroyam Slava!";
//	sent = s.send(ack, strlen(ack));
//
//	cout << "Sent2: " << sent << endl;
//
//
//	free(hi_msg);
//	sec._free();
//    return 0;
//
//}

const char hi[] = "Slava Ukraini!";
const char by[] = "Heroyam Slava!";

class KeyManager: public Point {

	Security sec;
	int state;
	enum STATE {
		IDLE,
		REQUEST,
		ACK,
		DONE
	};
public:
	KeyManager(Worker* parent, Socket* socket): Point(parent, socket) {
		state = IDLE;
	};

	char* create_hi(char *out, char* pub_key)
	{
		strcpy(out, hi);
		int i = strlen(hi);

		int size = strlen(pub_key)-1;
		out[i++] = (char)(size>>8);
		out[i++] = (char)(size & 0xFF);

		strcpy(out+i, pub_key);
		return out;
	}

	void init() {
		sec.build();
		int hi_size = strlen(hi) + 2 + strlen(sec.pub_key);
		char* hi_msg = (char*)malloc(hi_size);
		create_hi(hi_msg, sec.pub_key);
		Buffer request;
		request.assign(hi_msg, hi_msg + hi_size - 1);

		this->socket->connect();
		this->send(request);
		state = REQUEST;
	};

	void do_recv() {
		switch (state) {
		case REQUEST: {
			//TODO decrypt the key
			Buffer& replay = this->recv();
			state = ACK;
			break;
		}
		case ACK: {
			//TODO change to one sequence
			Buffer ack;
			ack.assign(by, by + strlen(by));
			this->send(ack);
			state = DONE;
			break;
		}
		case DONE: {
			//handle disconnect, handle closure upon do_send
			this->closed = true;
		}
		}
	}
};

int main(void) {
	Config::init();
	Config::get().server = Addr("0.0.0.0", 8080);
	Config::get().client = Addr("127.0.0.1", 8080);

	Socket* socket = new Socket(Config::get().client);

	Worker w = Worker();
	KeyManager* km = new KeyManager(&w, socket);
	km->init();

	while (!w.empty()) {
		w.run();
	}

	cout << "KUKU";
    return 0;
}

