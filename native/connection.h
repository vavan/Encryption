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


//TODO get rid of this class
class BufferedPoint: public WorkItem {
protected:
	bool closing;

public:
	Queue send_queue;
	BufferedPoint(Worker* parent, Socket* socket) : WorkItem(parent, socket), closing(false), send_queue(this) {};


	virtual void init();
	virtual void recv();
	virtual void send();
	virtual void close();

//	virtual void on_send() = 0;
//	virtual void on_close() = 0;
};




#endif /* CONNECTION_H_ */
