/*
 * smpp_message_req.hpp
 *
 *  Created on: Feb 11, 2011
 *      Author: rjoshi
 *  Arawat Inc Reserved 2011
 */

#ifndef ARWT_SMPP_DATA_HPP_
#define ARWT_SMPP_DATA_HPP_

#include <boost/cstdint.hpp>
#include <proto_data_interface.hpp>
#include <uuid.hpp>

using namespace arawat::util;
namespace arawat {
namespace protocol {
/**
 * smpp address
 */
struct smpp_address {
public:

	/**
	 * default constructor
	 * @return
	 */
	smpp_address(): ton_(0x00), npi_(0x00),error_code_(-1) {
	}
	/**
	 * constrcutro
	 * @param addr
	 * @param ton
	 * @param npi
	 * @return
	 */
	smpp_address(const std::string& addr, boost::uint8_t ton, boost::uint8_t npi,
			int error_code=-1):
	address_(addr), ton_(ton), npi_(npi), error_code_(error_code){
	}

	std::string address_;
	boost::uint8_t ton_;
	boost::uint8_t npi_;
	int error_code_; //-1 not set
	MSGPACK_DEFINE(address_, ton_, npi_, error_code_);


};
typedef std::list<smpp_address> smpp_addresses_t;
class smpp_message_resp : public proto_data_interface {
public:
	/**
	 * constrcutor
	 * @return
	 */
	smpp_message_resp():
			proto_data_interface(msg::PROTO_SMPP,
					msg::TYPE_RESPONSE),
			command_id_(0x00),seqnum_(0),command_status_(0),
			error_code_(-1){


		}
	/**
	 * constructor
	 * @param command_id
	 * @return
	 */
	smpp_message_resp(boost::uint32_t command_id):
		proto_data_interface(msg::PROTO_SMPP,
				msg::TYPE_RESPONSE),
		command_id_(command_id),seqnum_(0),command_status_(0),
		error_code_(-1){


	}
	/**
	 * constructor
	 */
	smpp_message_resp(boost::uint32_t command_id,
			boost::uint32_t seqnum,
			boost::uint32_t command_status):
				proto_data_interface(msg::PROTO_SMPP,
						msg::TYPE_RESPONSE),
				command_id_(command_id),seqnum_(seqnum),command_status_(command_status),
				error_code_(-1){



	}

	boost::uint32_t command_id_;
	boost::uint32_t seqnum_; //unused:0
	boost::uint32_t command_status_;
	int error_code_;
	std::string msgid_; //unused:0x00
	smpp_addresses_t to_list_; //submitmulti

	MSGPACK_DEFINE(protocol_, type_, id_, command_id_, seqnum_, command_status_, error_code_, msgid_, to_list_);
};
/**
 * Smpp Data defination class
 */
class smpp_message_req : public proto_data_interface {
public:

	smpp_message_req():proto_data_interface(msg::PROTO_SMPP), command_id_(0x00), seqnum_(0),
	registered_delivery_(0x00), data_coding_(0x00),esm_class_(0x00),
	priority_(0x00),replace_if_present_(0x00),default_msgid_(0x00){

	}
	/**
	 * Constructor
	 * @return
	 */
	smpp_message_req(boost::uint32_t command_id):proto_data_interface(msg::PROTO_SMPP),
			command_id_(command_id), seqnum_(0),
			registered_delivery_(0x00), data_coding_(0x00),esm_class_(0x00),
			priority_(0x00),replace_if_present_(0x00),default_msgid_(0x00){

	}



//private:
	boost::uint32_t command_id_;
	unsigned seqnum_; //unused:0
	smpp_address from_;
	smpp_addresses_t to_list_;
	boost::uint8_t registered_delivery_;//non registered:0x00
	boost::uint8_t data_coding_; //default 0x00
	boost::uint8_t esm_class_; //transaction mode:0x02, normal:0x00
	boost::uint8_t priority_; //normail:0x00, immediate:0x01, high:0x02
	boost::uint8_t replace_if_present_; //noreplace:0x00, replace:0x01
	boost::uint8_t default_msgid_; //unused:0x00
	std::string msg_;

	MSGPACK_DEFINE((int&)protocol_, (int&)type_, id_, command_id_, seqnum_, from_, to_list_, registered_delivery_, data_coding_, esm_class_, priority_, replace_if_present_, default_msgid_, msg_);

};
}//protocol
}//arawat

#endif /* ARWT_SMPP_DATA_HPP_ */
