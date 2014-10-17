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

class QueueStat {
private:
	static const int PERIOD = 20;
	static int iteration;
	static int max;
public:
	static void max_size(int size);
	static void print();
};


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
	int size();

public:

	WorkItem* work_item;
	Queue(WorkItem* workItem);
	~Queue();
	Buffer* get_front();
	Buffer* get_back();
	void compleate(ssize_t actual);
	bool empty();
};


#endif /* QUEUE_H_ */


