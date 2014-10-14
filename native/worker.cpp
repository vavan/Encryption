/*
 * worker.cpp

 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#include <algorithm>
#include "worker.h"


WorkItem::WorkItem(Worker* parent, Socket* socket): closing(false), parent(parent), socket(socket), send_queue(this) {
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
	Socket::RetCode retcode = this->socket->recv();

    if (retcode == Socket::CLOSE || retcode == Socket::ERROR) {
		this->close();
	}
}

void WorkItem::send() {
	Socket::RetCode retcode = this->socket->send();

	if (this->closing || retcode == Socket::ERROR) {
		this->close();
	}
}

void WorkItem::close() {
	if (this->send_queue.empty()) {
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
		for(DeletedItems::iterator wi = delete_item_list.begin(); wi != delete_item_list.end(); ++wi) {
			WorkItem* p = (*wi);
			WorkItems::iterator delete_iter = find(items.begin(), items.end(), p);
			int position = delete_iter - items.begin();
			items.erase(delete_iter);
			events.erase(events.begin()+position);
			delete p;
		}
		delete_item_list.clear();
		if (items.empty()) {
			this->running = false;
		}
		return true;
	}
	return false;
}

bool Worker::add_items() {
	if (!add_item_list.empty()) {
		int last_event = events.size();
		events.resize(events.size() + add_item_list.size());
		WorkItems::iterator wi = add_item_list.begin();
		WorkItemEvents::iterator ei = events.begin() + last_event;

		for (; wi != add_item_list.end(); ++wi, ++ei) {
			items.push_back((*wi));
			(*ei).fd = (*wi)->get_fd();
			(*ei).events = POLLIN;
			if (!(*wi)->send_queue.empty()) {
				(*ei).events |= POLLOUT;
			}
		}
		add_item_list.clear();
		return true;
	}
	return false;
}

void Worker::update_items() {
	if (add_items() || delete_items()) {
		WorkItems::iterator wi = items.begin();
		WorkItemEvents::iterator ei = events.begin();
		for (; wi != items.end(); ++wi, ++ei) {
			(*wi)->event = &(*ei);
		}
	}
}


void Worker::run() {
	update_items();

	int retval = poll(&(this->events[0]), this->events.size(), -1);

	if (retval == -1) {
		LOG.errorStream() << "WORKER. Wait failed";
	} else {
		for (WorkItems::iterator wi = items.begin(); wi != items.end(), retval > 0; ++wi ) {
			if ((*wi)->event->revents & POLLOUT) {
				(*wi)->send();
				retval--;
			}
			if ((*wi)->event->revents & POLLIN) {
				(*wi)->recv();
				retval--;
			}
		}
	}
}
