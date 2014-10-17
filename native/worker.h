/*
 * worker.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <poll.h>
#include <list>
#include <set>
#include <vector>
#include "queue.h"

class Socket;

using namespace std;


class Worker;
typedef struct pollfd WorkItemEvent;

class WorkItem {
	friend class Worker;
protected:
	bool closing;
	bool error;
	Worker* parent;
	Socket* socket;
	Queue send_queue;
	WorkItemEvent* event;

public:

	WorkItem(Worker* parent, Socket* socket);
	virtual ~WorkItem();
	int get_fd();
	void sending(bool start);

	virtual void init() {};
	virtual void recv();
	virtual void send();
	virtual void close();
};


class Worker {
	friend class WorkeItem;
private:
	typedef vector<WorkItem*> WorkItems;
	typedef set<WorkItem*> DeletedItems;
	typedef vector<WorkItemEvent> WorkItemEvents;
	WorkItems items;
	WorkItems add_item_list;
	DeletedItems delete_item_list;
	WorkItemEvents events;
	void update_items();
	bool delete_items();
	bool add_items();

public:
	Worker() : running(true) {};
	bool running;
	void add(WorkItem* point);
	void remove(WorkItem* point);
	void run();
};




#endif /* WORKER_H_ */
