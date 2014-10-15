/*
 * message.hpp
 *
 *  Created on: Mar 24, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_MESSAGE_HPP_
#define JOSHI_MESSAGE_HPP_
#include "rvariant.hpp"
#include <msgpack.hpp>


namespace arawat {
namespace util {

namespace msg {

static const std::string  S_MSG_ID 				= "id"; //request-response
static const std::string  S_MSG_PROTOCOL 			= "protocol"; //request-response
static const std::string  S_MSG_TYPE 				= "type"; //request-response
static const std::string  S_MSG_STATUS 			= "status";
static const std::string  S_MSG_STATUS_CODE 		= "status_code";
static const std::string  S_MSG_CONTENT_TYPE		= "content-type";
static const std::string  S_MSG_CONTENT_length 	= "content-length";
static const std::string  S_MSG_CONTENT_DISP 		= "content-disposition";
static const std::string  S_MSG_FROM 				= "from";
static const std::string  S_MSG_TO 				= "to";
static const std::string  S_MSG_DATE 				= "date";
static const std::string  S_MSG_SUBJECT 			= "subject";

typedef enum {
	TYPE_UNDEF,
	TYPE_REQUEST,
	TYPE_RESPONSE
}e_msg_type;



/**
 * protocol type
 */
typedef enum  {
	PROTO_UNDEF,//!< PROTO_UNDEF
	PROTO_SMPP, //!< PROTO_SMPP
	PROTO_SMTP, //!< PROTO_SMTP
	PROTO_IMAP, //!< PROTO_IMAP
	PROT_SIP, //!< PROT_SIP
	PROTO_LDAP, //!< PROTO_LDAP
	PROTO_SS7, //!< PROTO_SS7
	PROTO_HTTP, //!< PROTO_HTTP,
	PROTO_QPID  //!< QPID
}e_msg_protocol;

/**
 * encoding
 */
typedef enum {
	ENC_TEXT_PLAIN,    //!< ENC_TEXT_PLAIN
	ENC_JASON,         //!< ENC_JASON

	//custom
	ENC_MSG_PACK = 100,//!< ENC_MSG_PACK
	ENC_GOOGLE_PB      //!< ENC_GOOGLE_PB
}e_msg_encoding;

} //ns msg



struct hdr_val {
	typedef std::list<hdr_val> hdr_vals_t;
	typedef std::map<std::string, hdr_vals_t > headers_t;


	rvariant_type val_;
	map_str_rvariant_type props_ ; //map

//	MSGPACK_DEFINE(val_, props_);

};
struct msg_part {
	typedef std::vector<msg_part> msg_parts_t;

	map_str_rvariant_type props_;
	hdr_val::headers_t headers_;
	std::vector<boost::uint8_t> content_;
//	MSGPACK_DEFINE(props_, headers_, content_);
};
struct message {
	map_str_rvariant_type props_;
	hdr_val::headers_t headers_; //std::string, hdr_val:
	msg_part::msg_parts_t parts_;
	//MSGPACK_DEFINE(props_, headers_, parts_);
};
}
}

MSGPACK_ADD_ENUM(arawat::util::msg::e_msg_protocol);
MSGPACK_ADD_ENUM(arawat::util::msg::e_msg_type);
MSGPACK_ADD_ENUM(arawat::util::msg::e_msg_encoding);
#endif /* JOSHI_MESSAGE_HPP_ */
