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
private:
	vector<Buffer*> queue;

	enum State {
		IDLE,
		FRONT,
		BACK
	};

	State state;

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

public:
	static const int ENTRIES = 50;
	static const int DEPTH = 4096;
	Queue() {
		state = IDLE;
		front = 0;
		back = 0;
		for(int i = 0; i < ENTRIES; i++) {
			Buffer* b = new Buffer(DEPTH);
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
//	//TODO remove
//	void return_front() {
//		front = dec(front);
//	}
	Buffer* get_front() {
		if (front+1 == back) {
			LOG.alertStream() << "QUEUE. Buffer overflow";
			return NULL;
		}
		state = FRONT;
		Buffer* b = queue[front];
//		front = inc(front); //TODO remove
		return b;
	}
	Buffer* get_back() {
		state = BACK;
		Buffer* b = queue[back];
//		back = inc(back);//TODO remove
		return b;
	}
//	//TODO remove
//	Buffer* try_back() {
//		Buffer* b = queue[back];
//		return b;
//	}
	void compleate(ssize_t actual) {
		if (state == FRONT) {
			queue[front]->resize(actual);
			front = inc(front);
		} else if (state == BACK){
			back = inc(back);
		}
		state = IDLE;
	}
	bool empty() {
		return front == back;
	}
};


#endif /* QUEUE_H_ */


