#include "connection.h"
#include "config.h"



Point::Point(Worker* parent, Socket* socket): parent(parent), socket(socket) {
	LOG << "+Point: " << this;
	this->parent->add(this);
	this->closed = false;
	this->buffer.reserve(Point::BUFFER_SIZE);
}
Point::~Point() {
	this->parent->remove(this);
	if (this->socket) delete this->socket;
	LOG << "-Point: %08X" << this;
}

Buffer& Point::recv() {
	char b[4096];
	int recved = this->socket->recv(b, 4096);
	if (recved > 0) {
		buffer.assign(b, b+recved);
	} else if (recved < 0) {
		LOG << "Recv ERROR. Drop connection";
		//TODO handle error!!!
	}
	return buffer;
}

void Point::send(Buffer& msg) {
	queue.push(msg);
}

void Point::on_send() {
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
			LOG << "Send on: " << fd;
		}
		FD_SET(fd, &recv_fds);
		LOG << "Read on: " << fd;
		if (fd > max_fd) max_fd = fd;
	}
	max_fd++;
}
void Worker::run() {
	build();
	int retval = select(max_fd, &recv_fds, &send_fds, NULL, NULL);
	LOG << "#select " << retval << "|" << max_fd;
	if (retval == -1)
		perror("select()");
	else if (retval) {
		Points::iterator i = points.begin();
		while( i != points.end() ) {
			int fd = (*i)->get_fd();
			if FD_ISSET(fd, &recv_fds) {
				LOG << "#recv " << fd;
				Buffer& data = (*i)->recv();
				if (data.size() > 0) {
					(*i)->on_recv(data);
				} else {
					(*i)->on_close();
				}
			}
			if FD_ISSET(fd, &send_fds) {
				LOG << "#send " << fd;
				(*i)->on_send();
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
