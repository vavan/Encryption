/*
 * socket.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */
/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include "normal_socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "queue.h"
#include "config.h"

#define LISTEN_BACKLOG 100

NormalSocket::NormalSocket(const Addr& addr) :
		Socket(addr) {
	this->s = socket(AF_INET, SOCK_STREAM, 0);
	this->nonblock();
	LOG.infoStream() << "SOCKET["<< this->s << "]. Create/New";
}

NormalSocket::NormalSocket(const Addr& addr, int accepted) :
		Socket(addr, accepted) {
	this->nonblock();
	LOG.infoStream() << "SOCKET["<< this->s << "]. Create/Accepted";
}

NormalSocket::~NormalSocket() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Delete/Close";
	::close(this->s);
}


bool NormalSocket::connect() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Connect";
	struct sockaddr_in serveraddr;
	socklen_t addr_size;

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	inet_pton(AF_INET, addr.ip.c_str(), &serveraddr.sin_addr);

	addr_size = sizeof(serveraddr);
	if (::connect(s, (struct sockaddr *) &serveraddr, addr_size) == -1) {
		int ret = errno;
		if (ret != EINPROGRESS) {
			LOG.errorStream() << "SOCKET["<< this->s << "]. Connect failed:" << errno;
			return false;
		}
	}
	return true;
}

bool NormalSocket::listen() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Listen";
	struct sockaddr_in serveraddr;
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	inet_pton(AF_INET, addr.ip.c_str(), &(serveraddr.sin_addr));

	if (bind(s, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Bind failed:" << errno;
		return false;
	}
	if (::listen(s, LISTEN_BACKLOG) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Listen failed:" << errno;
		return false;
	}
	return true;
}

Socket* NormalSocket::copy(const Addr& addr, int newsocket) {
	return new NormalSocket(this->addr, newsocket);
}

Socket* NormalSocket::accept() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Accept";
	struct sockaddr_in clientaddr;
	unsigned int addr_size = sizeof(clientaddr);
	int newsocket = ::accept(s, (struct sockaddr *) &clientaddr, &addr_size);
	if (newsocket > 0) {
		return this->copy(this->addr, newsocket);
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Accept failed:" << errno;
		return NULL;
	}
}

void NormalSocket::nonblock() {
	int rc = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
	if (rc < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Couldn't set non blocking mode";
	}
}

ssize_t NormalSocket::send() {
	Buffer* buffer = send_queue->get_back();
	ssize_t ret = ::send(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		LOG.debugStream() << "SOCKET["<< this->s << "]. Send:" << ret;
		send_queue->compleate(ret);
		return ret;
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Send failed:" << errno;
		return Socket::ERROR;
	}
	return 0;
}

ssize_t NormalSocket::recv() {
	Buffer* buffer = recv_queue->get_front();
	ssize_t ret = ::recv(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		LOG.debugStream() << "SOCKET["<< this->s << "]. Recv:" << ret;
		if (ret > 0) recv_queue->compleate(ret);
		return ret;
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Recv failed:" << errno;
		return Socket::ERROR;
	}
}

