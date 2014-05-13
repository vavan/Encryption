/*
 * config.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdlib.h>
#include <string>
#include "addr.h"
#ifdef LOG4CPP
#include <log4cpp/Category.hh>
#else
#include <iostream>
#endif

/*
 * Logging system
 */
//Access to the log
#ifdef LOG4CPP
#define LOG log4cpp::Category::getRoot().debugStream()
#else
#define LOG std::cerr << std::endl
#endif

//Initialization of the log
void init_log();

/*
 * Major configuration class
 */
class Config {
	static Config* instance;
	~Config() {
		if (this->child != NULL) {
			free(this->child);
		}
	}
public:

	Addr server;
	Addr client;
	char** child;

	static void init() {
		Config::instance = new Config();
	}
	static void done() {
		delete Config::instance;
	}
	static Config& get() {
		return *Config::instance;
	}
};




#endif /* CONFIG_H_ */
