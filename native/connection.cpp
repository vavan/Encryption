#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}


void BufferedPoint::recv() {
	int recved = this->socket->recv();

    if (recved == Socket::CLOSE || recved == Socket::ERROR) {
		this->close();
	}
}

void BufferedPoint::send() {
	ssize_t sent = this->socket->send();

	if (this->closing || sent == Socket::ERROR) {
		this->close();
	}
}

void BufferedPoint::close() {
	if (this->send_queue.empty()) {
		this->parent->remove(this);
	} else {
		this->closing = true;
	}
}
