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
	Listener(Worker* parent, Socket* socket): Point(parent, socket) {};
	void init();
	void do_recv();
};

class Pipe: public Point {
private:
	Pipe* other;
	bool closing;

	virtual Buffer* recv();

public:
	Pipe(Worker* parent, Socket* socket): Point(parent, socket) {
		other = NULL;
		closing = false;
	};
	void join(Pipe* other);
	virtual void on_recv(Buffer* buffer);
	virtual void on_send();
	virtual void on_close();
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
