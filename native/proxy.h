/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef PROXY_H_
#define PROXY_H_

#include "worker.h"

class Listener : public WorkItem {
public:
	Listener(Worker* parent, Socket* socket);
	virtual void init();
	virtual void recv();
	virtual void send();
};

class Pipe: public WorkItem {
private:
	Pipe* other;

public:
	Pipe(Worker* parent, Socket* socket): WorkItem(parent, socket), other(NULL) {};
	void join(Pipe* other);
	virtual void close();
};

class ServerPipe : public Pipe {
public:
	ServerPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
	void init() {}
};

class ClientPipe : public Pipe {
public:
	ClientPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
	void init() {
		this->socket->connect();
	}
};



#endif /* PROXY_H_ */
