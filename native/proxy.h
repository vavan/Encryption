/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef PROXY_H_
#define PROXY_H_

#include "connection.h"


class Listener : public WorkItem {
public:
	Listener(Worker* parent, Socket* socket);
	virtual void init();
	virtual void recv();
	virtual void send();
};

class Pipe: public BufferedPoint {
private:
	Pipe* other;
	bool closing;

public:
	Pipe(Worker* parent, Socket* socket): BufferedPoint(parent, socket), other(NULL), closing(false) {};
	void join(Pipe* other);
	virtual void on_send();
	virtual void on_close();
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
