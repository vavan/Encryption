#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}

void BufferedPoint::recv() {
	int recved = this->socket->recv();

	if (recved > 0) {
		LOG.errorStream() << "SOCKET. recv bytes=" << recved;
		this->on_recv();
	} else if (recved == 0) {
		this->on_close();
	} else if (recved == Socket::ERROR) {
		//TODO close connection
	} else if (recved == Socket::INPROGRESS) {
//		this->return_buffer();
	}
}

void BufferedPoint::send() {
	ssize_t sent = this->socket->send();
	if (sent > 0) {
		this->on_send();
	} else if (sent == Socket::ERROR) {
		LOG.errorStream() << "BufferedPoint. Send error";
	}
}

bool BufferedPoint::is_sending() {
	return this->socket->is_sending() || !this->send_queue.empty();
}

void BufferedPoint::push(Buffer* msg) {
	Buffer* buffer = send_queue.get_front();
	buffer->assign(msg->begin(), msg->end());
}





