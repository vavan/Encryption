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

Queue::Queue(WorkItem* work_item) : work_item(work_item) {
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
	for(vector<Buffer*>::iterator i = queue.begin(); i != queue.end(); ++i) {
		Buffer* b = (*i);
		delete b;
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
		LOG.infoStream() << "QUEUE["<< this->work_item->get_fd() << "]. Not enough buffers - reallocate. Old size=" << queue.size();
		LOG.infoStream() << "QUEUE. " << this->work_item << "|" << this->work_item->event << "|" << this->work_item->event->events << "|";
		reallocate();
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
		this->work_item->sending(true);
	} else if (state == BACK){
		back = inc(back);
		if (front == back) {
			this->work_item->sending(false);
		}
	}
	state = IDLE;
}

bool Queue::empty() {
	return front == back;
}


