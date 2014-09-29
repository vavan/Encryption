/*
 * worker.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */
#include "worker.h"

WorkItem::WorkItem(Worker* parent, Socket* socket): parent(parent), socket(socket) {
	this->parent->add(this);
	this->closed = false;
}

WorkItem::~WorkItem() {
	this->parent->remove(this);
	if (this->socket) {
		delete this->socket;
	}
}

int WorkItem::get_fd() {
	return this->socket->get();
}

bool WorkItem::is_closed() {
	return closed;
}

Socket* WorkItem::relese_socket() {
	Socket* released = this->socket;
	this->socket = NULL;
	return released;
}

void Worker::add(WorkItem* point) {
	points.push_back(point);
}

void Worker::remove(WorkItem* point) {
	points.remove(point);
}

bool Worker::empty() {
	return points.empty();
}

void Worker::build() {
	max_fd = 0;
	FD_ZERO(&send_fds);
	FD_ZERO(&recv_fds);
	for (WorkItems::iterator i = points.begin(); i != points.end(); i++) {
		int fd = (*i)->get_fd();
		if ((*i)->is_sending()) {
			FD_SET(fd, &send_fds);
		}
		FD_SET(fd, &recv_fds);
		if (fd > max_fd) max_fd = fd;
	}
	max_fd++;
}

void Worker::run() {
	build();
	int retval = select(max_fd, &recv_fds, &send_fds, NULL, NULL);
	if (retval == -1) {
		LOG.errorStream() << "WORKER. Select failed";
	}
	else if (retval) {
		WorkItems::iterator i = points.begin();
		while( i != points.end() ) {
			int fd = (*i)->get_fd();
			if FD_ISSET(fd, &recv_fds) {
				(*i)->recv();
			}
			if FD_ISSET(fd, &send_fds) {
				(*i)->send();
			}
			if ((*i)->is_closed()) {
				WorkItem* p = (*i);
				points.erase(i++);
				delete p;
			} else {
				++i;
			}
		}
	}
}



