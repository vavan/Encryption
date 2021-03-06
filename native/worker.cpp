/*
 * worker.cpp

 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#include <algorithm>
#include "worker.h"
#include "proxy.h"

WorkItem::WorkItem(Worker* parent, Socket* socket): closing(false), error(false), parent(parent), socket(socket), send_queue(this) {
	this->parent->add(this);
}

WorkItem::~WorkItem() {
	if (this->socket) {
		delete this->socket;
	}
}

int WorkItem::get_fd() {
	return this->socket->get();
}

void WorkItem::sending(bool start) {
	if (start) {
		event->events |= POLLOUT;
	} else {
		event->events &= ~POLLOUT;
	}
}

void WorkItem::recv() {
	Socket::RetCode retcode = Socket::CLOSE;
	if (!this->closing)
		retcode = this->socket->recv();

    if (retcode == Socket::CLOSE || retcode == Socket::ERROR) {
		this->close();
	}
}

void WorkItem::send() {
	Socket::RetCode retcode = this->socket->send();

	if (this->closing || retcode == Socket::ERROR) {
		if (retcode == Socket::ERROR) {
			this->error = true;
		}
		this->close();

	}
}

void WorkItem::close() {
	if (this->send_queue.empty() || this->error) {
		this->parent->remove(this);
	} else {
		this->closing = true;
	}
}



void Worker::add(WorkItem* point) {
	add_item_list.push_back(point);
}

void Worker::remove(WorkItem* point) {
	delete_item_list.insert(point);
}

bool Worker::delete_items() {
	if (!delete_item_list.empty()) {
		for(DeletedItems::iterator di = delete_item_list.begin(); di != delete_item_list.end(); ++di) {
			WorkItem* p = (*di);
			WorkItems::iterator wi = find(items.begin(), items.end(), p);
			if (wi == items.end()) {
				LOG.errorStream() << "Unable to find for delete: " << p;
				LOG.errorStream() << "Cont: " << p->get_fd();
			} else {
				items.erase(wi);
				delete p;
			}
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
	bool added = add_items();
	bool deleted = delete_items();
	if (added || deleted) {
		events.clear();
		events.resize(items.size());
		WorkItems::iterator wi = items.begin();
		WorkItemEvents::iterator ei = events.begin();
		for (; wi != items.end(); ++wi, ++ei) {
			(*wi)->event = &(*ei);
			(*ei).fd = (*wi)->get_fd();
			(*ei).events = POLLIN;
			if (!(*wi)->send_queue.empty()) {
				(*ei).events |= POLLOUT;
			}
		}
	}
}


void Worker::run() {
	update_items();

	if (items.empty()) {
		LOG.infoStream() << "No more alive connection";
		this->running = false;
		return;
	}

	int retval = poll(&(this->events[0]), this->events.size(), -1);

	if (retval == -1) {
		LOG.errorStream() << "WORKER. Wait failed";
	} else {
		for (WorkItems::iterator wi = items.begin(); wi != items.end(); ++wi ) {
			if ((*wi)->event->revents & POLLOUT) {
				(*wi)->send();
			}
			if ((*wi)->event->revents & POLLIN) {
				(*wi)->recv();
			}
		}
	}
}
