/*
 * queue.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */

#include "queue.h"


int Queue::inc(int index) {
	if  (index < ENTRIES-1) {
		index++;
	} else {
		index = 0;
	}
	return index;
}

Queue::Queue() {
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
}

Buffer* Queue::get_front() {
	if (front + 1 == back) {
		LOG.alertStream() << "QUEUE. Buffer overflow";
		return NULL;
	}
	state = FRONT;
	Buffer* b = queue[front];
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
	} else if (state == BACK){
		back = inc(back);
	}
	state = IDLE;
}
bool Queue::empty() {
	return front == back;
}


