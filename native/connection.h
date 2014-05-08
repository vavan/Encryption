/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef PROXY22_H_
#define PROXY22_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <deque>
#include <vector>
#include "socket.h"

using namespace std;

class Worker;

typedef vector<char> Buffer;


class Queue {
public:
	void push(Buffer& msg) {
		queue.push_back(msg);
	}
	Buffer pop() {
		Buffer b = queue.front();
		queue.pop_front();
		return b;
	}
	bool empty() {
		return queue.empty();
	}
private:
	deque<Buffer> queue;
};

class Point {
protected:
	Worker* parent;
	Socket* socket;
	Buffer buffer;
	Queue queue;
	bool closed;
	static const int BUFFER_SIZE = 4096;

	virtual Buffer& recv();
	virtual void send(Buffer& msg);

public:
	Point(Worker* parent, Socket* socket);
	virtual ~Point();
	int get_fd() {
		return this->socket->s;
	}
	bool is_closed() {
		return closed;
	}
	Socket* relese_socket() {
		Socket* released = this->socket;
		this->socket = NULL;
		return released;
	}
	virtual void init() {};
	virtual void on_recv(Buffer& data) {};
	virtual void on_send();
	virtual void on_close() {};


	friend Worker;
};


class Worker {
	typedef list<Point*> Points;
	Points points;
	fd_set recv_fds, send_fds;
	int max_fd;
public:
	void build();
	void add(Point* point);
	void remove(Point* point);
	void run();
	bool empty();
};

#endif /* PROXY22_H_ */
