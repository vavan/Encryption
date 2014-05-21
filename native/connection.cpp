#include "connection.h"
#include "config.h"



Point::Point(Worker* parent, Socket* socket): parent(parent), socket(socket) {
	LOG << "+Point: " << this;
	this->parent->add(this);
	this->closed = false;
	this->recv_buffer.reserve(Queue::DEPTH);
}
Point::~Point() {
	this->parent->remove(this);
	if (this->socket) delete this->socket;
	LOG << "-Point: " << this;
}

Buffer* Point::recv() {
//	static char b[4096];
	recv_buffer.resize(Queue::DEPTH);
	int recved = this->socket->recv(&recv_buffer[0], recv_buffer.size());
	if (recved >= 0) {
		recv_buffer.resize(recved);
	} else {
		recv_buffer.resize(0);
		LOG << "Recv ERROR. Drop connection";
	}
	return &recv_buffer;
}

void Point::send(Buffer* msg) {
	Buffer* buffer = send_queue.push();
	LOG << "push: " << buffer;
	buffer->assign(msg->begin(), msg->end());
}
void Point::on_send() {
	if (!send_queue.empty()) {
		Buffer* buffer = send_queue.pop();
		LOG << "pop: " << buffer;
//		LOG << "on send: " << (*buffer)[0] << &(*buffer)[0];
		this->socket->send(&(*buffer)[0], buffer->size());
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
		if (!(*i)->send_queue.empty()) {
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
				Buffer* data = (*i)->recv();
				if (data->size() > 0) {
					(*i)->on_recv(data);
				} else {
					(*i)->on_close();
				}
			}
			if FD_ISSET(fd, &send_fds) {
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

