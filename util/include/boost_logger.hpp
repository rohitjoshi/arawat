/*
 * File:   boost_logger.hpp
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on August 29, 2009, 4:03 PM
 * Copyright (c) 2009 . See LICENSE for details.
 */

#ifndef _ARWT_BOOST_LOGGER_HPP
#define _ARWT_BOOST_LOGGER_HPP
#include <fstream>
#include <string>

//namespace
namespace arawat {
namespace util {

/**
 * boost_logger class.
 */
class boost_logger {
public:
    enum severity_level {
        trace, info, event, warn, error
    	//normal, warning, error
    };
    static boost_logger& instance() {
    	static boost_logger instance_;
        return instance_;
    }

    //initialize
    bool init(const std::string& logfile);
    bool isInit() {
    	return init_;
    }
    /**
     * write log
     * @param logtype
     * @param file
     * @param line
     * @param func
     * @param buf
     */
    static void write_log(const unsigned int logtype,
    		const char* file, unsigned int line, const char* func, const char * buf,
    		...);

private:
    boost_logger();
    ~boost_logger();
    bool init_;

};


} //ns util
}//ns arawat
#endif  /* _ARWT_BOOST_LOGGER_HPP */
