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

class Socket;

using namespace std;

class Worker;
typedef struct pollfd WorkItemEvent;

class WorkItem {
protected:
	Worker* parent;
	Socket* socket;

public:
	WorkItemEvent* event;
	void sending(bool start);

	WorkItem(Worker* parent, Socket* socket);
	virtual ~WorkItem();
	int get_fd();

	virtual void init() = 0;
	virtual void recv() = 0;
	virtual void send() = 0;
};


class Worker {
private:
	typedef list<WorkItem*> WorkItems;
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
	void add(WorkItem* point);
	void remove(WorkItem* point);
	void run();
};




#endif /* WORKER_H_ */
