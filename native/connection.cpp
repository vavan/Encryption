#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}

void BufferedPoint::recv() {
	Buffer* buffer = this->get_buffer();
	int recved = this->socket->recv(&(*buffer)[0], buffer->size());
	if (recved > 0) {
		buffer->resize(recved);
		LOG.errorStream() << "SOCKET. recv bytes=" << recved;
		this->on_recv(buffer);
	} else if (recved == 0) {
		this->return_buffer();
		this->on_close();
	} else if (recved == Socket::ERROR) {
		this->return_buffer();
		//TODO close connection
	} else if (recved == Socket::INPROGRESS) {
		this->return_buffer();
	}
}

void BufferedPoint::send() {
	if (!send_queue.empty()) {
		Buffer* buffer = send_queue.try_back();
		size_t size = buffer->size();
		ssize_t sent = this->socket->send(&(*buffer)[0], size);
		if (sent > 0) {
			send_queue.get_back();
			this->on_send(buffer);
		} else if (sent == Socket::ERROR) {
			LOG.errorStream() << "BufferedPoint. Send error";
		}
	}
}

bool BufferedPoint::is_sending() {
	return this->socket->is_sending() || !this->send_queue.empty();
}

void BufferedPoint::push(Buffer* msg) {
	Buffer* buffer = send_queue.get_front();
	buffer->assign(msg->begin(), msg->end());
}





