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
	virtual bool is_sending();
};

class Pipe: public BufferedPoint {
private:
	Pipe* other;
	bool closing;

	virtual Buffer* get_buffer();
public:
	Pipe(Worker* parent, Socket* socket): BufferedPoint(parent, socket), other(NULL), closing(false) {};
	void join(Pipe* other);
	virtual void on_recv(Buffer* buffer);
	virtual void on_send(Buffer* buffer);
	virtual void on_close();
};

class ServerPipe : public Pipe {
public:
	ServerPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
};

class ClientPipe : public Pipe {
public:
	ClientPipe(Worker* parent, Socket* socket): Pipe(parent, socket) {};
	void init() {
		this->socket->connect();
	}
};



#endif /* PROXY_H_ */
