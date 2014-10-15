/*
 * smpp_config.hpp
 *
 *  Created on: Jan 1, 2011
 *      Author: Rohit Joshi
 *  Arawat Inc Reserved 2010
 */

#ifndef ARWT_SMPP_CONFIG_HPP_
#define ARWT_SMPP_CONFIG_HPP_


#include <sstream>
#include <uuid.hpp>
#include "smpp.hpp"
namespace arawat {
namespace protocol {

#define SMPP_INFVER  0x34

namespace {
static const std::string S_BIND_TX = "transmitter";
static const std::string S_BIND_RX = "receiver";
static const std::string S_BIND_TXRX= "transceiver";
static const std::string S_BIND_UNDEF = "undefined";
}
/**
 * Smpp Configuration class
 */
struct smpp_config {
public:
	/**
	 * Connection bind type
	 */
	enum bind_type {
		BIND_UNDEF,//!< BIND_UNDEF
		BIND_TX,    //!< BIND_TX
		BIND_RX,    //!< BIND_RX
		BIND_TXRX   //!< BIND_TXRX
	};

	/**
	 * default initialization
	 * @return
	 */
	smpp_config() {
		ver_ = SMPP_INFVER;  //ox34
		keepalive_ = 600; //secs
		rebind_period_ = 10; //in case of disconected, retry every 10 seconds
		ton_ = 0x00; //national
		npi_ = 0x00; //national
		auto_deliversm_resp_ = true;
		bindtype_ = BIND_UNDEF;
		scaddr_ = "";
		use_datasm_ = false;
		id_ = arawat::util::uuid::gen_uid_str();
	}



	static std::string bindtype(bind_type t)  {
		if(t == BIND_TX) {
			return S_BIND_TX;
		}else if(t == BIND_RX) {
			return S_BIND_RX;
		}else if(t == BIND_TXRX) {
			return S_BIND_TXRX;
		}
		return S_BIND_UNDEF;
	}

	std::string bindtype() const {
			if(bindtype_ == BIND_TX) {
				return S_BIND_TX;
			}else if(bindtype_ == BIND_RX) {
				return S_BIND_RX;
			}else if(bindtype_ == BIND_TXRX) {
				return S_BIND_TXRX;
			}
			return S_BIND_UNDEF;
		}

	std::string host_;
	std::string port_;
	Smpp::SystemId sysid_;
	Smpp::Password password_;
	Smpp::SystemType systype_;
	Smpp::ServiceType servicetype_;
	Smpp::Uint8 ver_;
	Smpp::Ton ton_;
	Smpp::Npi npi_;
	bool auto_deliversm_resp_;
	bool use_datasm_;
	//if connection with smsc is lost than how offent we want to retry...
	unsigned rebind_period_; //every 10 seconds

	unsigned keepalive_; //in sec. used to send keep alive
	bind_type bindtype_;
	std::string scaddr_; //sc address
	std::string id_; //connection id suppllied by client



};
} //ns protocol
}//ns arawat
#endif /* ARWT_SMPP_CONFIG_HPP_ */
