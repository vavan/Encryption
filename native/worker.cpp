/*
 * worker.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */
#include "worker.h"
#include "poll.h"

WorkItem::WorkItem(Worker* parent, Socket* socket): parent(parent), socket(socket) {
	this->parent->add(this);
	this->closed = false;
}

WorkItem::~WorkItem() {
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

Worker::Worker() {
	this->size = Worker::INITIAL_PULL;
	this->fds = new Item[this->size];
	this->fd_changed = true;
}

Worker::~Worker() {
	delete[] fds;
}

void Worker::add(WorkItem* point) {
	points.push_back(point);
	this->fd_changed = true;
}

void Worker::remove(WorkItem* point) {
	points.remove(point);
	this->fd_changed = true;
}

bool Worker::empty() {
	return points.empty();
}

void Worker::reallocate_fds() {
	if (this->fd_changed) {
		if (this->size < points.size()) {
			delete[] fds;
			this->fds = new Item[points.size()];
		}
		this->size = points.size();
		this->fd_changed = false;
	}
}

void Worker::update_items() {
	reallocate_fds();
	size_t i = 0;
	for (WorkItems::iterator wi = points.begin(); wi != points.end(); ++wi) {
		Item *item = this->fds + i++;
		item->fd = (*wi)->get_fd();
		item->events = POLLIN;
		if ((*wi)->is_sending()) {
			item->events |= POLLOUT;
		}
	}
}

void Worker::close_items() {
	WorkItems::iterator wi = points.begin();
	while (wi != points.end()) {
		if ((*wi)->is_closed()) {
			WorkItem* p = (*wi);
			points.erase(wi++);
			delete p;
			this->fd_changed = true;
		} else {
			++wi;
		}
	}
}

void Worker::run() {
//	LOG.errorStream() << "1111";

	update_items();
//	LOG.errorStream() << "2222";
	int retval = poll(this->fds, this->size, -1);
//	LOG.errorStream() << "3333";
	if (retval == -1) {
		LOG.errorStream() << "WORKER. Select failed";
	}
	else if (retval) {
		size_t i = 0;
		for (WorkItems::iterator wi = points.begin(); wi != points.end(), i < this->size; ++wi ) {
			Item *item = this->fds + i++;
			if (item->revents & POLLIN) {
				(*wi)->recv();
			}
			if (item->revents & POLLOUT) {
				(*wi)->send();
			}
		}
		close_items();
	}
}
