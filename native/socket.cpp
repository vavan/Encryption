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


#include <sstream>
#include "socket.h"


std::string Addr::str() const {
	std::stringstream os;
	os << ip << ":" << port;
	return os.str();
}

Socket::Socket(const Addr& addr) : addr(addr), s(0), recv_queue(NULL), send_queue(NULL) {}

Socket::Socket(const Addr& addr, int accepted) : addr(addr), s(accepted), recv_queue(NULL), send_queue(NULL) {}

Socket::~Socket() {}

int Socket::get() {
	return this->s;
}



