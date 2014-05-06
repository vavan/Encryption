#include "connection.h"
#include "config.h"



Point::Point(Worker* parent, Socket* socket): parent(parent), socket(socket) {
	this->parent->add(this);
	this->closed = false;
	this->buffer.reserve(Point::BUFFER_SIZE);
}
Point::~Point() {
	this->parent->remove(this);
	delete this->socket;
}

Buffer& Point::recv() {
	int recved = this->socket->recv(&buffer[0], buffer.size());
	buffer.resize(recved);
	return buffer;
}

void Point::send(Buffer& msg) {
	queue.push(msg);
}

void Point::do_send() {
	if (!queue.empty()) {
		Buffer buffer = queue.pop();

		this->socket->send(&buffer[0], buffer.size());
	}
};

void Worker::add(Point* point) {
	points.push_back(point);
}
void Worker::remove(Point* point) {
	points.remove(point);
}
bool Worker::empty() {
	return points.empty();
}
void Worker::build() {
	max_fd = 0;
	FD_ZERO(&send_fds);
	FD_ZERO(&recv_fds);
	for (Points::iterator i = points.begin(); i != points.end(); i++) {
		int fd = (*i)->get_fd();
		if (!(*i)->queue.empty()) {
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
	if (retval == -1)
		perror("select()");
	else if (retval) {
		Points::iterator i = points.begin();
		while( i != points.end() ) {
			int fd = (*i)->get_fd();
			if FD_ISSET(fd, &recv_fds) {
				(*i)->do_recv();
			}
			if FD_ISSET(fd, &send_fds) {
				(*i)->do_send();
			}
			if ((*i)->is_closed()) {
				Point* p = (*i);
				points.erase(i++);
				delete p;
			} else {
				++i;
			}
		}
	}
}

