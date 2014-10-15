/*
 * protocol_data_interface.hpp
 *
 *  Created on: Jan 9, 2011
 *      Author: Rohit Arawat
 *  Arawat Inc Reserved 2010
 */

#ifndef ARWT_PROTOCOL_DATA_INTERFACE_HPP_
#define ARWT_PROTOCOL_DATA_INTERFACE_HPP_

#include <boost/assign.hpp>
#include <boost/asio/streambuf.hpp>
#include <msgpack.hpp>
#include <uuid.hpp>
#include <message.hpp>
#include <iostream>

using namespace boost::asio;
using namespace arawat::util;

namespace arawat {
namespace protocol {
/**
 * protocol data interface
 */
class proto_data_interface {
public:
	 typedef std::vector<boost::uint8_t> uchar_buffer_t;

	 proto_data_interface():protocol_(msg::PROTO_UNDEF),type_(msg::TYPE_UNDEF){
		 id_ = arawat::util::uuid::gen_uid_str();
	 }
	/**
	 * protocol data inteface
	 * @param type
	 * @return
	 */
	proto_data_interface(msg::e_msg_protocol proto, msg::e_msg_type dir=msg::TYPE_REQUEST):
		protocol_(proto),type_(dir){
		id_ = arawat::util::uuid::gen_uid_str();
	}
    /**
     * encode the data
     * @return
     */
//	virtual bool encode(uchar_buffer_t& buffer) const = 0;

	/**
	 * destructor
	 * @return
	 */
	virtual ~proto_data_interface() {
	}
	/**
	 * direction
	 * @return
	 */
	 msg::e_msg_type type() const {
		return type_;
	}
	/**
	 * type
	 * @return
	 */
	 msg::e_msg_protocol protocol() const {
		return protocol_;
	}

	std::string id() const {
		return id_;
	}





//protected:
	msg::e_msg_protocol protocol_;
	msg::e_msg_type type_;
	std::string id_;
	MSGPACK_DEFINE((int&)protocol_, (int&)type_, id_);

};

} //ns protocol
}//ns arawat

#endif /* ARWT_PROTOCOL_DATA_INTERFACE_HPP_ */
