/*
 * log.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: Rohit Joshi
 *  Arawat Inc Reserved 2011
 */

#include <cstdarg>
#include <cstdio>
#include "log.hpp"

/*
 *Namespace arawat
 */
namespace arawat {

/**
 * namespace util
 */
namespace util {

log::T_LOG_UTIL_LOG log::log_func_ptr_ = NULL;

static const unsigned LF_EVENT = 1L << log::LOG_EVENT;
static const unsigned LF_INFO = 1L << log::LOG_INFO;
static const unsigned LF_WARNING = 1L << log::LOG_WARNING;
static const unsigned LF_ERROR = 1L << log::LOG_ERROR;
static const unsigned LF_TRACE = 1L << log::LOG_TRACE;
static const unsigned LF_TRACE_IN = 1L << log::LOG_TRACE_IN;
static const unsigned LF_TRACE_OUT = 1L << log::LOG_TRACE_OUT;
static const unsigned LF_ALL = LF_INFO | LF_WARNING | LF_ERROR | LF_TRACE
    | LF_TRACE_IN | LF_TRACE_OUT;
static const unsigned LF_DEFAULT = LF_WARNING | LF_ERROR;

unsigned log::log_levels_ = LF_DEFAULT;


 std::string log::format_arg_list(const char *pszFormat, va_list args) {
	 if (!pszFormat) return "";

	    char szBuf[FORMAT_BUFFER_SIZE];
	    int nRet = vsnprintf(szBuf, FORMAT_BUFFER_SIZE, pszFormat, args);
	    if (nRet >= (int) FORMAT_BUFFER_SIZE) {
	        // redo the formatting
	        char *buffer = (char*)malloc ((nRet + 1) * sizeof(char));
	        vsnprintf(buffer, nRet, pszFormat, args);
	        std::string sMsg(buffer, nRet);
	        free(buffer);
	        return sMsg;
	    } else {
	        std::string sMsg(szBuf, nRet);
	        return sMsg;
	    }
 }
 /**
 	 * Set the log level
 	 */
 	void log::log_level(log::logtype_t eType) {
 		log_levels_ |= eType;
 	}

 	/**
 	 * Enable info
 	 */
 	void log::enable_error() {
 		    log_levels_ |= LF_ERROR;
 		    log_levels_ &= (~LF_WARNING);
 	 		log_levels_ &= (~LF_EVENT);
 	 		log_levels_ &= (~LF_INFO);
 	 		log_levels_ &= (~LF_TRACE);
 	 		log_levels_ &= (~LF_TRACE_IN);
 	 		log_levels_ &= (~LF_TRACE_OUT);
 	}
 	/**
 	 * Enable info
 	 */
 	void log::enable_warn() {
 		log_levels_ |= LF_ERROR;
 		log_levels_ |= LF_WARNING;
 		log_levels_ &= (~LF_EVENT);
 		log_levels_ &= (~LF_INFO);
 		log_levels_ &= (~LF_TRACE);
 		log_levels_ &= (~LF_TRACE_IN);
 		log_levels_ &= (~LF_TRACE_OUT);

 	}
 	/**
 	 * Enable event
 	 */
 	void log::enable_event(){
 		log_levels_ |= LF_ERROR;
 		log_levels_ |= LF_WARNING;
 		log_levels_ |= LF_EVENT;
 		log_levels_ &= (~LF_INFO);
 		log_levels_ &= (~LF_TRACE);
 		log_levels_ &= (~LF_TRACE_IN);
 		log_levels_ &= (~LF_TRACE_OUT);
 	}
 	void log::disable_event(){
 	 		log_levels_ &= (~LF_EVENT);
 	 	}
 	/**
 	 * Enable info
 	 */
 	void log::enable_info(){
 		log_levels_ |= LF_ERROR;
 		log_levels_ |= LF_WARNING;
 		log_levels_ |= LF_EVENT;
 		log_levels_ |= LF_INFO;
 		log_levels_ &= (~LF_TRACE);
 		log_levels_ &= (~LF_TRACE_IN);
 		log_levels_ &= (~LF_TRACE_OUT);
 	}
 	void log::disable_info(){
 	 		log_levels_ &= (~LF_INFO);
 	 	}

 	/**
 	 * Enable trace
 	 */
 	void log::enable_trace(){
 		log_levels_ |= LF_ERROR;
 		log_levels_ |= LF_WARNING;
 		log_levels_ |= LF_EVENT;
 		log_levels_ |= LF_INFO;
 		log_levels_ |= LF_TRACE;
 		log_levels_ |= LF_TRACE_IN;
 		log_levels_ |= LF_TRACE_OUT;
 	}

 	void log::disable_trace() {
 		log_levels_ &= (~LF_TRACE);
 		log_levels_ &= (~LF_TRACE_IN);
 		log_levels_ &= (~LF_TRACE_OUT);
 	}

}//util
}//arawat
