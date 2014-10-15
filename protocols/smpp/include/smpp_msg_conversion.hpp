/*
 * smpp_msg_conversion.hpp
 *
 *  Created on: Mar 24, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#ifndef JOSHI_SMPP_MSG_CONVERSION_HPP_
#define JOSHI_SMPP_MSG_CONVERSION_HPP_

#include "smpp_message.hpp"
#include <message.hpp>

namespace arawat {
namespace protocol {
namespace smpp {
static const std::string S_SMPP_TON = "ton";
static const std::string S_SMPP_NPI = "npi";
static const std::string S_SMPP_SEQ_NUM = "seq_num";
static const std::string S_SMPP_ERROR_CODE = "error_code";
}
/**
 * smpp msg conversion
 */
class smpp_msg_conversion {
public:
	/*
	 * to message
	 * @param addr_name
	 * @param msg
	 */
	static void smpp2message(const smpp_address& addr,
			arawat::util::hdr_val& hdr) {
		hdr.val_ = addr.address_;
		hdr.props_[smpp::S_SMPP_TON] = addr.ton_;
		hdr.props_[smpp::S_SMPP_NPI] = addr.npi_;
		if (addr.error_code_ != -1) {
			hdr.props_[smpp::S_SMPP_ERROR_CODE] = addr.error_code_;
		}

	}
	/**
	 * to message
	 * @param msg
	 */
	static void smpp2message(const smpp_message_resp& resp,
			arawat::util::message& msg) {
		msg.props_[arawat::util::msg::S_MSG_ID] = resp.msgid_;
		msg.props_[arawat::util::msg::S_MSG_STATUS]
				= Smpp::CommandStatus::description(resp.command_status_);
		msg.props_[arawat::util::msg::S_MSG_STATUS_CODE] = resp.command_status_;
		msg.props_[smpp::S_SMPP_SEQ_NUM] = resp.seqnum_;
		if (resp.error_code_ != -1) {
			hdr.props_[smpp::S_SMPP_ERROR_CODE] = resp.error_code_;
		}

		//to address
		arawat::util::hdr_vals_t hdr_vals;
		smpp_addresses_t::const_iterator it = resp.to_list_.begin();
		for (; it != to_list_.end(); ++it) {
			arawat::util::hdr_val hdr;
			smpp2message(*it, hdr);
			hdr_vals.push_back(hdr);
		}
		msg.headers_[arawat::util::msg::S_MSG_TO] = hdr_vals;

	}

	/**
	 * to message
	 * @param msg
	 */
	static void smpp2message(const smpp_message_req& req,
			arawat::util::message& msg) {
		msg.props_[arawat::util::msg::S_MSG_ID] = req.msgid_;
		msg.props_[arawat::util::msg::S_MSG_STATUS]
				= Smpp::CommandStatus::description(req.command_status_);
		msg.props_[arawat::util::msg::S_MSG_STATUS_CODE] = req.command_status_;
		msg.props_[smpp::S_SMPP_SEQ_NUM] = req.seqnum_;

		//from header
		arawat::util::hdr_val hdr;
		smpp2message(req.from_, hdr);
		msg.headers_[arawat::util::msg::S_MSG_FROM] = hdr;

		//to headers
		arawat::util::hdr_vals_t hdr_vals;
		smpp_addresses_t::const_iterator it = req.to_list_.begin();
		for (; it != req.to_list_.end(); ++it) {
			arawat::util::hdr_val hdr;
			smpp2message(*it, hdr);
			hdr_vals.push_back(hdr);
		}
		msg.headers_[arawat::util::msg::S_MSG_TO] = hdr_vals;

		//add part
		arawat::util::msg_part part;
		std::copy(req.msg_.begin(), req.msg_.end(), part.content_.begin());
		msg.parts_.push_back(part);

	}
};
}
}

#endif /* JOSHI_SMPP_MSG_CONVERSION_HPP_ */
