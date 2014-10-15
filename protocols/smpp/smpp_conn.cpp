/*
 * smpp.cpp
 *
 *  Created on: Dec 27, 2010
 *      Author: Rohit Joshi
 *  Arawat Inc Reserved 2010
 */
#include <cstdio>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "smpp_conn.hpp"
#include "smpp_message.hpp"
#include "smpp.hpp"
#include "log.hpp"

using namespace arawat::util;
using namespace ::boost;

namespace arawat {
namespace protocol {
#define SMPP_HEADER_SIZE 16
/**
 * constructor
 * @param config
 * @return
 */
smpp_conn::smpp_conn(const smpp_config& config, const observer_msg_type& observer) :
	conn_interface(config.id_, msg::PROTO_SMPP),
	config_(config), seqnum_(0),unbind_sent_(false), unbind_resp_received_(false),conn_ptr_(
			new client_connection(SMPP_HEADER_SIZE)), stop_(
			false), enquirelink_thread_ptr_((boost::thread*) NULL),
			last_activity_time_(0){
	LOG_IN("");


	LOG_INFO("Connection id[%s]", id_.c_str());



	update_handler_msg.connect(observer);

	conn_ptr_->add_error_handler(
			boost::bind(&smpp_conn::handle_error, this, _1));

	conn_ptr_->add_header_handler(boost::bind(&smpp_conn::get_payload_size,
			this, _1));

	conn_ptr_->add_message_handler(boost::bind(
			&smpp_conn::process_incoming_msg, this, _1));

	thread_ptr_t tmp_ptr(new boost::thread(boost::bind(
			&smpp_conn::enquirelink_loop, this)));

	enquirelink_thread_ptr_ = tmp_ptr;

	LOG_OUT("");
}
	/**
	 * send enquire link periodically
	 */
	void smpp_conn::enquirelink_loop() {
		LOG_IN("");
		while (!stop_) {
			boost::this_thread::sleep(boost::posix_time::seconds(config_.keepalive_));
			if (running_) {
				//send enquire link only if last message sent was more than keep alive time
			if((time(NULL) - last_activity_time_ ) >= config_.keepalive_)
			send_enquire_link();
		}
	}
	LOG_OUT();
}
/**
 * destructor
 * @return
 */
smpp_conn::~smpp_conn() {
	enquirelink_thread_ptr_->interrupt();
	if (enquirelink_thread_ptr_.get() && enquirelink_thread_ptr_->joinable()) {
		enquirelink_thread_ptr_->join();
		thread_ptr_t tmp_ptr((boost::thread*) NULL);
		enquirelink_thread_ptr_ = tmp_ptr;
	}
}

/**
 * Initialize the connection
 * @param conf
 * @return
 */
result_t smpp_conn::init() {
	LOG_IN("");
	boost::asio::ip::tcp::resolver::query query(config_.host_, config_.port_);


	if(running_) {
		stop();
	}
	running_ = false;
	//connect
	result_t r;
	do {
		r = conn_ptr_->connect(query);
		if (r.get<0> ()) {
			LOG_INFO("%s", r.get<1> ().c_str());
			conn_ptr_->start();
			r = send_bind();
			if (r.get<0> ()) {
				r = send_enquire_link();
			}
		} else {
			LOG_ERROR("Failed connect. Retrying in [%d] seconds.", config_.rebind_period_);
		}
		//if error, retry...
		if(!r.get<0> ()) {
			sleep(config_.rebind_period_);
		}
	}while (!r.get<0> ());

	LOG_INFO("Initialized connection successfully..");
	LOG_RET_RESULT(r);

}
/**
 * stop
 */
void smpp_conn::stop(bool force) {
	LOG_IN("");

	if(running_) {
		send_unbind();
	    unsigned counter = 3;
	    while(!unbind_resp_received_ && counter--) {
		  sleep(1);
		  LOG_INFO("Waiting for unbind response");
	   }
	}
	stop_ = true;
	running_ = false;
	conn_ptr_->stop(force);

	LOG_OUT("");
}
/**
 * start
 */
void smpp_conn::start() {
	LOG_IN("");
	stop_ = false;
	running_ = true;
	LOG_OUT("");
}
/**
 * error
 * @param error_code
 */
void smpp_conn::handle_error(const boost::system::error_code& error_code) {
	LOG_IN("");
	int errval = error_code.value();
	std::string errmsg = error_code.message();
	LOG_ERROR("ErrorCode[%d], ErrorMessage[%s]", errval, errmsg.c_str());
	switch (errval) {
		case boost::system::errc::no_such_file_or_directory:
		case boost::system::errc::broken_pipe:
		case boost::system::errc::connection_aborted:
		case boost::system::errc::connection_reset:
		case boost::system::errc::host_unreachable:
		case boost::system::errc::io_error:
		case boost::system::errc::network_reset:
		running_ = false;
		init();
		start();
		break;
		default:
		break;
	}
	LOG_OUT("");

}
/**
 * send bind command
 * @return
 */
result_t smpp_conn::send_bind() {
	LOG_IN("");
	LOG_INFO("Sending bind type[%d]...",config_.bindtype_);
	switch (config_.bindtype_) {
		case smpp_config::BIND_RX: {
			Smpp::BindTransceiver pdu;
			result_t r = send_bind(pdu);
			LOG_RET_RESULT(r);
		}
		break;
		case smpp_config::BIND_TX: {
			Smpp::BindTransceiver pdu;
			result_t r = send_bind(pdu);
			LOG_RET_RESULT(r);
		}
		break;
		case smpp_config::BIND_TXRX: {
			Smpp::BindTransceiver pdu;
			result_t r = send_bind(pdu);
			LOG_RET_RESULT(r);
		}
		break;
		default:
		std::stringstream ss;
		ss << "Invalid bind type " << config_.bindtype_;
		LOG_ERROR("Failed to send_bind. Error[%s]", ss.str().c_str())
		;

		LOG_RET_RESULT(result_t(false, ss.str()))
		;

	}
}
/**
 * send bind
 * @param pdu
 * @return
 */
template<typename T> result_t smpp_conn::send_bind(T& pdu) {
	LOG_IN("");

	//initialize request from config
	pdu.sequence_number(increment_seqnum());
	pdu.system_id(config_.sysid_);
	pdu.password(config_.password_);
	pdu.system_type(config_.systype_);
	pdu.interface_version(config_.ver_);
	pdu.addr_ton(config_.ton_);
	pdu.addr_npi(config_.npi_);
	LOG_INFO("Sending a bind ...");

	boost::uint8_t* d = (boost::uint8_t*) pdu.encode();
	int l = pdu.command_length();
	hex_dump(d, l);
	result_t r = send_async(d, l);
	LOG_RET_RESULT(r);
}
/**
 * send unbind command
 * @return
 */
result_t smpp_conn::send_unbind() {
	LOG_IN("");
	Smpp::Unbind pdu(0x7fffffff); // set the sequence number
	LOG_INFO("Sending an unbind");
	boost::uint8_t* d = (boost::uint8_t*) pdu.encode();
	int l = pdu.command_length();
	hex_dump(d, l);
	unbind_sent_ = true;
	result_t r = send_async(d, l);
	LOG_RET_RESULT(r);
}

/**
 * Send message
 * @param data
 * @return
 */
result_t smpp_conn::send_message(const proto_data_interface& msg) {
	LOG_IN("");

	boost::uint32_t cid = 0;
	if(msg.type() == util::msg::TYPE_REQUEST) {
		LOG_TRACE("It's a request..");
		const smpp_message_req& data = (const smpp_message_req&)msg;
		cid = data.command_id_;
	} else {
		LOG_TRACE("It's a response..");
		const smpp_message_resp& data = (const smpp_message_resp&)msg;
		cid = data.command_id_;
	}
	LOG_TRACE("Command id[%x]", cid);
	switch (cid) {
		case Smpp::CommandId::SubmitMulti: {
			Smpp::SubmitMulti pdu;
			const smpp_message_req& data = (const smpp_message_req&)msg;
			if (!data.to_list_.size()) {
				LOG_RET_RESULT(result_t(false, "SubmitMulti: destination list(to_list_) is empty. Can't send msg."));
			}

			pdu.priority_flag(Smpp::PriorityFlag(data.priority_));
			pdu.replace_if_present_flag(Smpp::ReplaceIfPresentFlag(
							data.replace_if_present_));
			pdu.sm_default_msg_id(Smpp::SmDefaultMsgId(data.default_msgid_));
			pdu.short_message(
					reinterpret_cast<const boost::uint8_t*> (data.msg_.data()),
					data.msg_.length());
			result_t r = send_msg(pdu, data);
			LOG_RET_RESULT(r);
		}
		break;
		case Smpp::CommandId::SubmitSm: {
			const smpp_message_req& data = (const smpp_message_req&)msg;
			Smpp::SubmitSm pdu;
			pdu.priority_flag(Smpp::PriorityFlag(data.priority_));
			pdu.replace_if_present_flag(Smpp::ReplaceIfPresentFlag(
							data.replace_if_present_));
			pdu.sm_default_msg_id(Smpp::SmDefaultMsgId(data.default_msgid_));
			pdu.short_message(
					reinterpret_cast<const boost::uint8_t*> (data.msg_.data()),
					data.msg_.length());
			result_t r = send_msg(pdu, data);
			LOG_RET_RESULT(r);
		}
		break;
		case Smpp::CommandId::DataSm: {
			const smpp_message_req& data = (const smpp_message_req&)msg;
			Smpp::DataSm pdu;
			pdu.insert_array_tlv(Smpp::Tlv::message_payload, data.msg_.length(),
					reinterpret_cast<const boost::uint8_t*> (data.msg_.data()));
			result_t r = send_msg(pdu, data);
			LOG_RET_RESULT(r);
		}
		break;
		case Smpp::CommandId::DeliverSmResp: {
			const smpp_message_resp& data = (const smpp_message_resp&)msg;
			Smpp::DeliverSmResp pdu(data.command_status_, data.seqnum_);
			boost::uint8_t* d = (boost::uint8_t*) pdu.encode();
			int l = pdu.command_length();
			hex_dump(d, l);
			result_t r = send_async(d, l);
			LOG_RET_RESULT(r);
		}
		case Smpp::CommandId::EnquireLink: {
			const smpp_message_req& data = (const smpp_message_req&)msg;
			Smpp::EnquireLink pdu(data.seqnum_);
			boost::uint8_t* d = (boost::uint8_t*) pdu.encode();
			int l = pdu.command_length();
			hex_dump(d, l);
			result_t r = send_async(d, l);
			LOG_RET_RESULT(r);
		}
		default:
		break;
	}
	std::stringstream ss;
	ss << "Unsupported:Invalid message type " << cid;
	LOG_ERROR("Failed to send_msg. Error[%s]", ss.str().c_str());
	LOG_RET_RESULT(result_t(false, ss.str()))

}
/**
 * send enquire link message
 * @return
 */
result_t smpp_conn::send_enquire_link() {
	LOG_IN("");
	LOG_INFO("Sending Enquire Link message");
	smpp_message_req msg(Smpp::CommandId::EnquireLink);
	msg.seqnum_ = increment_seqnum();
	LOG_TRACE("Command id[%d]", msg.command_id_);
	result_t r = send_message(msg);
	LOG_RET_RESULT(r);
}
/**
 * send message
 * @private
 * @param pdu
 * @return
 */
template<typename T, typename D> result_t smpp_conn::send_msg(T& pdu, D& data) {
	LOG_IN("");
	unsigned seqnum = data.seqnum_;
	if (!seqnum) {
		seqnum = increment_seqnum();
	}
	pdu.sequence_number(seqnum);
	pdu.service_type(config_.servicetype_);

	pdu.source_addr(Smpp::SmeAddress(Smpp::Ton(data.from_.ton_), Smpp::Npi(
							data.from_.npi_), Smpp::Address(data.from_.address_)));


	smpp_addresses_t::const_iterator it = data.to_list_.begin();
	for (; it != data.to_list_.end(); ++it) {
		Smpp::SmeAddress addr(Smpp::Ton(it->ton_), Smpp::Npi(it->npi_),
				Smpp::Address(it->address_));
		pdu.destination_addr(addr);
	}


	pdu.registered_delivery(Smpp::RegisteredDelivery(data.registered_delivery_));

	pdu.data_coding(Smpp::DataCoding(data.data_coding_));

	pdu.esm_class(Smpp::EsmClass(data.esm_class_));

	boost::uint8_t* d = (boost::uint8_t*) pdu.encode();
	int l = pdu.command_length();
	hex_dump(d, l);
	LOG_INFO("Sending message type [%d]", data.command_id_);
	last_activity_time_ = time(NULL);
	result_t r = send_async(d, l);
	LOG_RET_RESULT(r);
}

/**
 * send_async data
 * @param buffer
 * @param length
 * @return
 */
result_t smpp_conn::send_async(boost::uint8_t* buffer, int length) {
	LOG_IN("buffer[%p], length[%d]", buffer, length);
	client_connection::uchar_buffer_t message(buffer, buffer + length);
	if (!conn_ptr_->send_async(message)) {
		LOG_RET_RESULT(result_t(false, "Failed to send message."));
	}
	LOG_RET_RESULT(result_t(true, "Successfully posted a message."));
}
/**
 * hex dump
 * @param d
 * @param len
 */
void smpp_conn::hex_dump(const boost::uint8_t* d, unsigned len) {
	if (log::is_trace()) {
		std::ostringstream ostrm;
		hex_dump(d, len, ostrm);
		LOG_TRACE("%s", ostrm.str().c_str());
	}
}

void smpp_conn::hex_dump(const boost::uint8_t* d, unsigned len,
		std::ostringstream& ostrm) {
	if (log::is_trace()) {
		Smpp::hex_dump(d, len, ostrm);
	}
}

/**
 * get payload size
 * This is used by client_connection class to read remaning payload
 * @param
 * @param
 * @return
 */
size_t smpp_conn::get_payload_size(
		const client_connection::uchar_buffer_t& buffer) {
	LOG_IN("");
	if (!buffer.size()) {
		LOG_ERROR("received empty buffer. Can't process.");
		return 0;
	}
	Smpp::Uint32 len = Smpp::get_command_length((boost::uint8_t*) &buffer[0]);

	if (len > 16) {
		len = len - 16;
		LOG_RET("body length[%d]", len);
	}
	LOG_RET("body length[%d]", 0);
}

/**
 * Process incoming msg
 * @param
 * @param
 */
bool smpp_conn::process_incoming_msg(
		const client_connection::uchar_buffer_t& buffer) {
	LOG_IN("");
	if (!buffer.size()) {
		LOG_ERROR("received empty buffer. Can't process.");
		LOG_RET("failed", false);;
	}
	Smpp::Uint32 cid = 0;

	bool status = true;

	try {
		cid = Smpp::CommandId::decode((const char*) &buffer[0]);
	} catch (std::exception & ex) {
		LOG_ERROR("Failed to decode the received data");
		LOG_RET("failed", false);
	}
	LOG_INFO("Received message type[0x%X]", cid);
	switch (cid) {
		/*response command id from smsc*/
		case Smpp::CommandId::BindReceiverResp: {
			Smpp::BindReceiverResp pdu;
			status = check_error(pdu, buffer);
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::BindTransceiverResp: {
			Smpp::BindTransceiverResp pdu;
			status = check_error(pdu, buffer);
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::BindTransmitterResp: {
			Smpp::BindTransmitterResp pdu;
			status = check_error(pdu, buffer);
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::EnquireLinkResp: {
			Smpp::EnquireLinkResp pdu;
			status = check_error(pdu, buffer);
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::UnbindResp: {
			LOG_INFO("Unbind resp received.");
			unbind_resp_received_ = true;
			Smpp::UnbindResp pdu;
			status = check_error(pdu, buffer);

			LOG_RET("Returning :%d", status);
		}

		case Smpp::CommandId::SubmitSm: {
			LOG_TRACE("SubmitSm received");
			Smpp::SubmitSm pdu;
			smpp_message_req msg(cid);
			bool status = read_msg_sm(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::SubmitSmResp: {
			Smpp::SubmitSmResp pdu;
			smpp_message_resp msg(cid);
			status = read_sm_resp(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::SubmitMultiResp: {
			Smpp::SubmitMultiResp pdu;
			smpp_message_resp msg(cid);
			status = read_sm_resp(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			LOG_RET("Returning :%d", status);
		}
		case Smpp::CommandId::DataSmResp: {
			Smpp::DataSmResp pdu;
			smpp_message_resp msg(cid);
			status = read_sm_resp(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			LOG_RET("Returning :%d", status);
		}

		case Smpp::CommandId::QuerySmResp: {
			Smpp::DataSmResp pdu;
			smpp_message_resp msg(cid);
			status = read_sm_resp(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			LOG_RET("Returning :%d", status);
		}

		case Smpp::CommandId::DeliverSm: {
			LOG_TRACE("DeliverSm received");
			Smpp::SubmitSm pdu;
			smpp_message_req msg(cid);
			bool status = read_msg_sm(pdu, buffer, msg);
			if (status) {
				add_to_queue(msg);
			}
			if (config_.auto_deliversm_resp_ && status) {
				smpp_message_resp send_resp(
						(boost::uint32_t) Smpp::CommandId::DeliverSmResp);
				send_resp.seqnum_ = msg.seqnum_;
				send_message(send_resp);
			}
			LOG_RET("Returning :%d", true);
		}

		case Smpp::CommandId::ReplaceSmResp:
		break;
		case Smpp::CommandId::CancelSmResp:
		break;

		case Smpp::CommandId::BroadcastSmResp:
		break;
		case Smpp::CommandId::QueryBroadcastSmResp:
		break;
		case Smpp::CommandId::CancelBroadcastSmResp:
		break;

		case Smpp::CommandId::Outbind:
		break;
		case Smpp::CommandId::EnquireLink:
		break;
		case Smpp::CommandId::AlertNotification:
		break;
		case Smpp::CommandId::Unbind:
		break;
		case Smpp::CommandId::GenericNack:
		break;
		default:
		break;
	}
	LOG_RET("NOT IMPLEMENTED:Returning :%d", false);
}
/**
 * read deliver sm
 * @param buffer
 * @return
 */
template<typename T> bool smpp_conn::read_msg_sm(T& pdu,
		const client_connection::uchar_buffer_t& buffer, smpp_message_req& data) {
	LOG_IN("");
	hex_dump(&buffer[0], buffer.size());
	//check if need pdu.decode

	try {
		pdu.decode(&buffer[0]); //FIXME / TODO check if need a copy assign operator
	}

	catch (std::exception & ex) {
		std::ostringstream ostrm;
		hex_dump(&buffer[0], buffer.size(), ostrm);
		LOG_ERROR("Failed to decode the received deliver_sm data. Exception:[%s], Smpp Pdu:\n%s",
				ex.what(), ostrm.str().c_str());

		LOG_RET("Returning:%d", false);
	}

	data.seqnum_ = pdu.sequence_number();
	//data.command_status_ = pdu.command_status();
	data.command_id_ = pdu.command_id();

	data.from_.address_ = pdu.source_addr().address();
	data.from_.ton_ = pdu.source_addr().ton();
	data.from_.npi_ = pdu.source_addr().npi();

	smpp_address addr;
	addr.address_ = pdu.destination_addr().address();
	addr.ton_ = pdu.destination_addr().ton();
	addr.npi_ = pdu.destination_addr().npi();
	data.to_list_.push_back(addr);

	data.registered_delivery_ = pdu.registered_delivery();

	data.priority_ = pdu.priority_flag();

	data.replace_if_present_ = pdu.replace_if_present_flag();

	data.default_msgid_ = pdu.sm_default_msg_id();

	//std::string m_sServiceType = pdu.service_type();
	std::ostringstream os;
	if (pdu.sm_length()) {

		std::copy(pdu.short_message().begin(), pdu.short_message().end(),
				std::ostream_iterator<char>(os));
		data.msg_ = os.str();

	} else { //sm_length 0. check the tlv payload for message
		const Smpp::Tlv* tlv = pdu.find_tlv(Smpp::Tlv::message_payload);
		if (tlv) {
			std::copy(tlv->value(), tlv->value() + tlv->length(),
					std::ostream_iterator<char>(os));
			data.msg_ = os.str();

		} else {
			LOG_INFO("No short message in delivery receipt");
		}
		const Smpp::Tlv* msgstate = pdu.find_tlv(Smpp::Tlv::message_state);
		if (msgstate)
		LOG_INFO("Message state: 0x%02x", *msgstate->value());
		/**
		 const Smpp::Tlv* necode = pdu.find_tlv(Smpp::Tlv::network_error_code);
		 if (necode) {
		 LOG_INFO("Network Type:   0x%02x", *necode->value());
		 data.network_error_code_ = Smpp::ntoh16(necode->value() + 1);
		 LOG_INFO("Error code:   0x%04x", data.network_error_code_);
		 }
		 **/
	}
	LOG_RET("Returning:%d", true);

}
/**
 * check error on received payload
 * @param pdu
 * @param buffer
 * @return
 */
template<typename T> bool smpp_conn::check_error(T& pdu,
		const client_connection::uchar_buffer_t& buffer) {
	LOG_IN("buf[%p]", &buffer);

	LOG_INFO("Process response to check error");
	hex_dump(&buffer[0], buffer.size());
	try {
		pdu.decode(&buffer[0]);
	} catch (std::exception & ex) {
		LOG_ERROR("Failed to decode the received data. Exception[%s]", ex.what());
		LOG_RET("Returning:%d", false);
	}
	if (pdu.command_status()) {
		LOG_ERROR("Received error for command_id[%d], command_status[%d]", pdu.command_id(), pdu.command_status());
		LOG_RET("Returning:%d", false);
	}
	LOG_RET("Returning:%d", true);
}
/**
 * Read sm response
 * @param pdu
 * @param buffer
 * @return
 */
template<typename T> bool smpp_conn::read_sm_resp(T& pdu,
		const client_connection::uchar_buffer_t& buffer, smpp_message_resp& msg) {
	LOG_IN("buf[%p]", &buffer);

	LOG_INFO("Read  response");
	hex_dump(&buffer[0], buffer.size());
	try {
		pdu.decode(&buffer[0]);
		const Smpp::Tlv* necode = pdu.find_tlv(Smpp::Tlv::network_error_code);
		if (necode) {
			LOG_INFO("Network Type:   0x%02x", *necode->value());
			msg.error_code_ = Smpp::ntoh16(necode->value() + 1);
			LOG_INFO("Error code:   0x%04x",msg.error_code_);

		}

		msg.seqnum_ = pdu.sequence_number();
		msg.command_id_ = pdu.command_id();
		msg.command_status_ = pdu.command_status();
		msg.msgid_ = pdu.message_id();
		LOG_INFO("Received Response msg.msg_id[%s], msg.seqnum_[%d],msg.command_id_[0x%X], msg.command_status_[%d]",
				msg.msgid_.c_str(), msg.seqnum_, pdu.command_id(), msg.command_status_);

		//if command_id is submitMultiResp, get all the list of unsuccessfull addresses.
		if(pdu.command_id() == Smpp::CommandId::SubmitMultiResp) {
			Smpp::SubmitMultiResp& pdumulti = (SubmitMultiResp&)pdu;
			Smpp::UnsuccessSmeColl unsuccess_sme = pdumulti.unsuccess_sme();
			if(unsuccess_sme.size()) {
				const Smpp::UnsuccessSmeColl::List& l = unsuccess_sme.get_list();
				Smpp::UnsuccessSmeColl::List::const_iterator it = l.begin();
				for(; it != l.end(); ++it) {
					const Smpp::UnsuccessSme* paddr = *it;
					if(paddr) {
						smpp_address addr;
						addr.address_ = paddr->smeAddress().address();
						addr.ton_ = paddr->smeAddress().ton();
						addr.npi_ = paddr->smeAddress().npi();
						addr.error_code_ = paddr->error();
						msg.to_list_.push_back(addr);
					}
				}
			}
		}
	} catch (std::exception & ex) {
		LOG_ERROR("Failed to decode the received data");
		LOG_RET("Returning:%d", false);
	}
	if (msg.command_status_) {
		LOG_ERROR("Received error for command_id[%d], command_status[%d]", msg.command_id_, msg.command_status_);
		LOG_RET("Returning:%d", false);
	}
	LOG_RET("Returning:%d", true);
}

bool smpp_conn::add_to_queue(const proto_data_interface& msg) {
	LOG_IN("");
	return update_handler_msg(msg, conn_ptr_->id());
}
} //protocol
}//arawat
