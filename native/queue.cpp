/*
 * queue.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */

#include "queue.h"
#include "worker.h"


int Queue::inc(int index) {
	if  (index < ENTRIES-1) {
		index++;
	} else {
		index = 0;
	}
	return index;
}

Queue::Queue(WorkItem* workItem) : workItem(workItem) {
	state = IDLE;
	front = 0;
	back = 0;
	for(int i = 0; i < ENTRIES; i++) {
		Buffer* b = new Buffer(DEPTH);
		queue.push_back(b);
	}
}

Queue::~Queue() {
	for(int i = 0; i < ENTRIES; i++) {
		Buffer* b = queue.back();
		delete b;
		queue.pop_back();
	}
	LOG.alertStream() << "QUEUE. MAX Size: " << max;
}

Buffer* Queue::get_front() {
	if (front + 1 == back) {
		//TODO reallocate instead of fail
		LOG.alertStream() << "QUEUE. Buffer overflow";
		return NULL;
	}
	state = FRONT;
	Buffer* b = queue[front];
	b->resize(DEPTH);
	return b;
}

Buffer* Queue::get_back() {
	state = BACK;
	Buffer* b = queue[back];
	return b;
}

void Queue::compleate(ssize_t actual) {
	if (state == FRONT) {
		queue[front]->resize(actual);
		front = inc(front);
		this->workItem->sending(true);
	} else if (state == BACK){
		back = inc(back);
		if (front == back) {
			this->workItem->sending(false);
		}
	}
	state = IDLE;
}

bool Queue::empty() {
	return front == back;
}


