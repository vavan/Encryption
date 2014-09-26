#include "config.h"
#include "connection.h"


void BufferedPoint::init() {
}

void BufferedPoint::recv() {
	Buffer* buffer = this->get_buffer();
	int recved = this->socket->recv(&(*buffer)[0], buffer->size());
	if (recved > 0) {
		buffer->resize(recved);
		this->on_recv(buffer);
	} else if (recved == 0) {
		this->on_close();
	} else {
		LOG.errorStream() << "BufferedPoint. Recv error";
	}
}

void BufferedPoint::send() {
	if (!send_queue.empty()) {
		Buffer* buffer = send_queue.get_back();
		size_t size = buffer->size();
		if (this->socket->send(&(*buffer)[0], size) != size) {
			LOG.errorStream() << "BufferedPoint. Send error";
		} else {
			this->on_send(buffer);
		}
	}
};

bool BufferedPoint::is_sending() {
	return !this->send_queue.empty();
}

void BufferedPoint::push(Buffer* msg) {
	Buffer* buffer = send_queue.get_front();
	buffer->assign(msg->begin(), msg->end());
}





