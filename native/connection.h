/*
 * proxy.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <deque>
#include <vector>
#include <exception>
#include "socket.h"
#include "config.h"
#include "queue.h"
#include "worker.h"


class BufferedPoint: public WorkItem {
protected:
	Queue send_queue;
//
//	virtual Buffer* get_buffer() = 0;
//	virtual void return_buffer() = 0;
public:
	BufferedPoint(Worker* parent, Socket* socket) : WorkItem(parent, socket) {};
	virtual void init();
	virtual void recv();
	virtual void send();
	virtual bool is_sending();

	virtual void push(Buffer* data);

	virtual void on_recv() = 0;
	virtual void on_send() = 0;
	virtual void on_close() = 0;
};




#endif /* CONNECTION_H_ */
