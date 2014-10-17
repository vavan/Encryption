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

#include "normal_socket.h"
#include "queue.h"
#include "config.h"

#define LISTEN_BACKLOG 100

NormalSocket::NormalSocket(const Addr& addr) :
		Socket(addr) {
	this->s = socket(AF_INET, SOCK_STREAM, 0);
	this->nonblock();
	LOG.debugStream() << "SOCKET["<< this->s << "]. Create/New";
}

NormalSocket::NormalSocket(const Addr& addr, int accepted) :
		Socket(addr, accepted) {
	this->nonblock();
	LOG.debugStream() << "SOCKET["<< this->s << "]. Create/Accepted";
}

NormalSocket::~NormalSocket() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Delete/Close";
	::close(this->s);
}


Socket::RetCode NormalSocket::connect() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Connect to " << addr.str();
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
			return Socket::ERROR;
		}
	}
	return Socket::OK;
}

Socket::RetCode NormalSocket::listen() {
	LOG.infoStream() << "SOCKET["<< this->s << "]. Listen on " << addr.str();
	struct sockaddr_in serveraddr;
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	inet_pton(AF_INET, addr.ip.c_str(), &(serveraddr.sin_addr));

	if (bind(s, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Bind failed:" << errno;
		return Socket::ERROR;
	}
	if (::listen(s, LISTEN_BACKLOG) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Listen failed:" << errno;
		return Socket::ERROR;
	}
	return Socket::OK;
}

Socket* NormalSocket::copy(const Addr& newaddr, int newsocket) {
	LOG.infoStream() << "SOCKET["<< newsocket << "]. Accepted from: " << newaddr.str();
	return new NormalSocket(addr, newsocket);
}

Socket* NormalSocket::accept() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Accept";
	struct sockaddr_in clientaddr;
	unsigned int addr_size = sizeof(clientaddr);
	int newsocket = ::accept(s, (struct sockaddr *) &clientaddr, &addr_size);
	if (newsocket > 0) {
		char string_addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientaddr.sin_addr), string_addr, sizeof(string_addr));
		Addr newaddr(string_addr, clientaddr.sin_port);
		return this->copy(newaddr, newsocket);
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

Socket::RetCode NormalSocket::send() {
	Buffer* buffer = send_queue->get_back();
	ssize_t ret = ::send(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		LOG.debugStream() << "SOCKET["<< this->s << "]. Send:" << ret;
		send_queue->compleate(ret);
		return Socket::OK;
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Send failed:" << errno;
		return Socket::ERROR;
	}
}

Socket::RetCode NormalSocket::recv() {
	Buffer* buffer = recv_queue->get_front();
	if (buffer == NULL) {
		return Socket::ERROR;
	}
	ssize_t ret = ::recv(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		LOG.debugStream() << "SOCKET["<< this->s << "]. Recv:" << ret;
		if (ret > 0) {
			recv_queue->compleate(ret);
			return Socket::OK;
		} else {
			return Socket::CLOSE;
		}
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Recv failed:" << errno;
		return Socket::ERROR;
	}
}

