/*
 * queue.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */

#include "queue.h"
#include "worker.h"


int Queue::inc(int index) {
	if  (index < entries-1) {
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
	entries = ENTRIES;
	for(int i = 0; i < entries; i++) {
		Buffer* b = new Buffer(DEPTH);
		queue.push_back(b);
	}
}

Queue::~Queue() {
	for(int i = 0; i < entries; i++) {
		Buffer* b = queue.back();
		delete b;
		queue.pop_back();
	}
}

void Queue::reallocate() {
	int current_size = queue.size();
	entries *= 2;
	for(int i = current_size; i < entries; i++) {
		Buffer* b = new Buffer(DEPTH);
		queue.push_back(b);
	}
}

Buffer* Queue::get_front() {
	if (inc(front) == back) {
		LOG.infoStream() << "QUEUE. Buffer overflow - reallocate";
		reallocate();
//		return NULL;
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


