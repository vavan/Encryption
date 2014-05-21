/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <deque>
#include <vector>
#include <exception>
#include "socket.h"
#include "config.h"

using namespace std;

class Worker;

typedef vector<char> Buffer;


class Queue {
	int front;
	int back;
	int inc(int index) {
		if  (index < ENTRIES-1) {
			index++;
		} else {
			index = 0;
		}
		return index;
	}
public:
	static const int ENTRIES = 50;
	static const int DEPTH = 4096;
	Queue() {
		front = 0;
		back = 0;
		for(int i = 0; i < ENTRIES; i++) {
			Buffer* b = new Buffer(DEPTH);
//			b.reserve(DEPTH);
			queue.push_back(b);
		}
	}
	~Queue() {
		for(int i = 0; i < ENTRIES; i++) {
			Buffer* b = queue.back();
			delete b;
			queue.pop_back();
		}
	}
	Buffer* push() {
		Buffer* b = queue[front];
		front = inc(front);
		LOG << "<" << front;
		if (front == back) {
			LOG << "Proxy buffer overflow. Stop";
			//throw "Proxy buffer overflow. Stop";
		}
		return b;
	}
	Buffer* pop() {
		Buffer* b = queue[back];
		back = inc(back);
		LOG << ">" << back;
		return b;
	}
	bool empty() {
		return front == back;
	}
private:
	vector<Buffer*> queue;
};

class Point {
protected:
	Worker* parent;
	Socket* socket;
//	Buffer buffer;
	Queue send_queue;
	Buffer recv_buffer;
	bool closed;
//	static const int BUFFER_SIZE = 4096;

	virtual Buffer* recv();
	virtual void send(Buffer* msg);

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
	virtual void on_recv(Buffer* data) {};
	virtual void on_send();
	virtual void on_close() {};


	friend class Worker;
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

#endif /* CONNECTION_H_ */
