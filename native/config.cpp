/*
 * config.cpp
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#ifdef LOG4CPP
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"
#endif

#include "config.h"

Config* Config::instance = NULL;

void init_log()
{
	log4cpp::Appender *appender = new log4cpp::FileAppender("default", "atunnel.log");
	appender->setLayout(new log4cpp::BasicLayout());

	log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("%d %p %m%n");
	appender->setLayout(layout);

	log4cpp::Category& root = log4cpp::Category::getRoot();
	root.setPriority(log4cpp::Priority::DEBUG);
	root.addAppender(appender);
}

