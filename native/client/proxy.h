/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef PROXY_H_
#define PROXY_H_

#include "connection.h"

class Listener : public Point {
public:
	void init();
	void do_recv();
};

class Pipe: public Point {
private:
	Pipe* other;
	bool closing;
public:
	Pipe(Worker* parent, Socket* socket): Point(parent, socket) {
		closing = false;
	};
	void join(Pipe* other);
	void do_recv();
	void do_send();
	void close();
};

class ServerPipe : public Pipe {
public:
	ServerPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
	void init();
};

class ClientPipe : public Pipe {
public:
	ClientPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
	void init();
};



#endif /* PROXY_H_ */
