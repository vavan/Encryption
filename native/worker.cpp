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

Worker::Worker() {}

Worker::~Worker() {}

void Worker::add(WorkItem* point) {
	add_item_list.push_back(point);
}

void Worker::remove(WorkItem* point) {
	delete_item_list.push_back(point);
}

bool Worker::empty() {
	return items.empty();
}

bool Worker::delete_items() {
	if (!delete_item_list.empty()) {
		for(WorkItems::iterator wi = delete_item_list.begin(); wi != delete_item_list.end(); ++wi) {
			WorkItem* p = (*wi);
			items.remove((*wi));
			delete p;
		}
		delete_item_list.clear();
		return true;
	}
	return false;
}

bool Worker::add_items() {
	if (!add_item_list.empty()) {
		items.insert(items.end(), add_item_list.begin(), add_item_list.end());
		add_item_list.clear();
		return true;
	}
	return false;
}


void Worker::update_items() {
	bool add = add_items();
	bool del = delete_items();
	if (add || del) {
		events.clear();
		events.resize(items.size());
		WorkItems::iterator wi = items.begin();
		WorkItemEvents::iterator ei = events.begin();
		for (; wi != items.end(); ++wi, ++ei) {
			(*ei).fd = (*wi)->get_fd();
			(*ei).events = POLLIN;
			if ((*wi)->is_sending()) {
				(*ei).events |= POLLOUT;
			}
		}
	} else {
		WorkItems::iterator wi = items.begin();
		WorkItemEvents::iterator ei = events.begin();
		for (; wi != items.end(); ++wi, ++ei) {
			(*ei).events = POLLIN;
			if ((*wi)->is_sending()) {
				(*ei).events |= POLLOUT;
			}
		}
	}
}

void Worker::run() {
	update_items();
	int retval = poll(&(this->events[0]), this->events.size(), -1);
	if (retval == -1) {
		LOG.errorStream() << "WORKER. Select failed";
	}
	else if (retval) {
		WorkItems::iterator wi = items.begin();
		WorkItemEvents::iterator ei = events.begin();
		for (; wi != items.end(); ++wi, ++ei ) {
			if ((*ei).revents & POLLIN) {
				(*wi)->recv();
			}
			if ((*ei).revents & POLLOUT) {
				(*wi)->send();
			}
		}
	}
}
