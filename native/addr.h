/*
 * addr.h
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */

#ifndef ADDR_H_
#define ADDR_H_

#include <sstream>

class Addr {
public:
	std::string ip;
	int port;
	Addr(std::string ip, int port) {
		this->ip=ip;
		this->port=port;
	}
	Addr(const Addr& addr) {
		this->ip=addr.ip;
		this->port=addr.port;
	}
	Addr() {
		this->ip="0.0.0.0";
		this->port=0;
	}
	std::string str()
	{
		std::stringstream os;
		os << ip << ":" << port;
		return os.str();
	}
};




#endif /* ADDR_H_ */
