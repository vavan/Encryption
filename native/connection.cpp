#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}


//TODO clean up
void BufferedPoint::recv() {
	int recved = this->socket->recv();

    if (recved == 0) {
		this->on_close();
	}
}

//TODO clean up
void BufferedPoint::send() {
	ssize_t sent = this->socket->send();
	if (sent > 0) {
		this->on_send();
	}
}

bool BufferedPoint::is_sending() {
	return this->socket->is_sending() || !this->send_queue.empty();
}
