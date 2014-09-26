/*
 * worker.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef WORKER_H_
#define WORKER_H_

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

class WorkItem {
protected:
	Worker* parent;
	Socket* socket;
	bool closed;
public:
	WorkItem(Worker* parent, Socket* socket);
	virtual ~WorkItem();
	int get_fd();
	bool is_closed();
	Socket* relese_socket();

	virtual void init() = 0;
	virtual void recv() = 0;
	virtual void send() = 0;
	virtual bool is_sending() = 0;
};


class Worker {
	typedef list<WorkItem*> WorkItems;
	WorkItems points;
	fd_set recv_fds, send_fds;
	int max_fd;
	void build();
public:
	void add(WorkItem* point);
	void remove(WorkItem* point);
	bool empty();
	void run();
};




#endif /* WORKER_H_ */
