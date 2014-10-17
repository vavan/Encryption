/*
 * proxy.h
 *
 *  Created on:
 *      Author: vova
 */
#ifndef QUEUE_H_
#define QUEUE_H_
#include <list>
#include <deque>
#include <vector>
#include "socket.h"
#include "config.h"


using namespace std;


typedef vector<char> Buffer;
class WorkItem;

class Queue {
private:
	static const int ENTRIES = 100;
	static const int DEPTH = 4096;
	vector<Buffer*> queue;

	enum State {
		IDLE,
		FRONT,
		BACK
	};

	State state;
	int front;
	int back;
	int entries;
	int inc(int index);
	void max_size();

public:
	static int max;

	WorkItem* work_item;
	Queue(WorkItem* workItem);
	~Queue();
	Buffer* get_front();
	Buffer* get_back();
	void compleate(ssize_t actual);
	bool empty();
};


#endif /* QUEUE_H_ */


