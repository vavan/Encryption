#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}


void BufferedPoint::recv() {
	int recved = this->socket->recv();

    if (recved == 0 || recved == Socket::ERROR) {
		this->on_close();
	}
}

void BufferedPoint::send() {
	ssize_t sent = this->socket->send();
	if (sent > 0) {
		this->on_send();
	} else if (sent == Socket::ERROR) {
		this->on_close();
	}
}
