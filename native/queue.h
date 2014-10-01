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
	int dec(int index) {
		if  (index > 0) {
			index--;
		} else {
			index = ENTRIES-1;
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
	void return_front() {
		front = dec(front);
	}
	Buffer* get_front() {
		Buffer* b = queue[front];
		front = inc(front);
		if (front == back) {
			LOG.alertStream() << "QUEUE. Buffer overflow";
			return NULL;
		}
		return b;
	}
	Buffer* get_back() {
		Buffer* b = queue[back];
		back = inc(back);
		return b;
	}
	Buffer* try_back() {
		Buffer* b = queue[back];
		return b;
	}
	bool empty() {
		return front == back;
	}
private:
	vector<Buffer*> queue;
};


#endif /* QUEUE_H_ */


