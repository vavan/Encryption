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
#include <log4cpp/Category.hh>
#include "addr.h"

/*
 * Logging system
 */
//Access to the log
#define LOG log4cpp::Category::getRoot()


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
