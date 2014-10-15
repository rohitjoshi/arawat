/*
 * File:   SmscSim.cpp
 * Author: Rohit Joshi <rohit.joshi@arawat.com>
 *
 * Created on Jan 16, 2011
 */
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_generators.hpp>
//#include <boost/lexical_cast.hpp>

#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>

//#include "uuid.hpp"
#include "smpp.hpp"
#include "log.hpp"
#include <boost_logger.hpp>
using namespace Smpp;
using namespace arawat::util;


using  boost::asio::ip::tcp;


typedef boost::mutex::scoped_lock scoped_lock;
const int max_length = 1024;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

 void write_log(const unsigned int logtype,
		const char* file, unsigned int line, const char* func, const char * buf,
		...) {

	 std::string msg;
	 va_list args;
	 va_start(args, buf);
	 msg = log::format_arg_list(buf, args);
	 va_end(args);

	 std::cout << logtype << "|" << file << ":" << line << "|" << func << "|" << msg << std::endl;

 }

const char hax_chars[17] = "0123456789abcdef";

class SmscSim
{
public:
	typedef std::vector<boost::uint8_t> SmppPduBuffer;
boost::mutex m_mRecvMsg;
boost::mutex m_mSendMsg;

long long m_nSendMsgs;
long long m_nRecvMsgs;
unsigned write_delay;
bool std_out;
bool deliver_sm;
boost::mutex mx;
boost::mutex Rxmx;
std::vector<socket_ptr> RxSocks;

public:
SmscSim() {
 m_nSendMsgs = 0;
 m_nRecvMsgs = 0;
 write_delay = 0;
 std_out = false;
 deliver_sm = false;

}
/////////////////////////////////////////////////////////////////////////////
//GetMsgId

void GetMsgId(std::string& sVal) {
  time_t t;
  time(&t);
  sVal.reserve(16);
  //boost::uudi::uuid uid = arawat::util::uuid::gen_uid_uuid();
  for (unsigned i = 0; i < 16; i++) {
   sVal[i] = hax_chars[t & 0x0F];
    t >>= 2;
  }

}

/////////////////////////////////////////////////////////////////////////////
// receive a deliver sm

void HexDump(const Smpp::Uint8* d, unsigned len) {
  if (log::is_info()) {
    std::ostringstream ostrm;
    Smpp::hex_dump(d, len, ostrm);
    LOG_INFO("%s", ostrm.str().c_str());
  }
}
/////////////////////////////////////////////////////////////////////////////
// function that reads an SMPP PDU from a socket.
// The contents are placed in a std::vector which is returned

bool ReadSmppPdu(socket_ptr sock, SmppPduBuffer& v, bool *close_socket) {
  LOG_IN("v[%p]", &v);
  boost::system::error_code error;
  // read the header
  v.resize(16);
  LOG_TRACE("Reading....");
  int n = sock->read_some(boost::asio::buffer(&v[0], 16), error);
  LOG_TRACE("Read [%d] bytes", n);
  if (n == -1 || error == boost::asio::error::eof) {
    LOG_ERROR("");
    LOG_ERROR("Socket Error: Failed to read data error Code [%d]-[%s]", error.value(), error.message().c_str());
    *close_socket = true;
    LOG_RET("Returning %d", false);
  }
  LOG_TRACE("ReadSmppPdu with length:%d", n);

  // extract the length from the header
  // Smpp::get_command_length() is an auxiliary function defined in
  // aux_types.hpp used for extracting the command_length from an
  // encoded PDU.
  // There are similar functions for the other header parameters.
  Smpp::Uint32 len = Smpp::get_command_length((Smpp::Uint8*) & v[0]);

  LOG_TRACE("Length from header:%d", len);

  // read the remainder of the PDU.
  // Some PDUs (e.g. enquire_link) only contain a header (16 octets)
  // hence the condition.
  if (len > 16) {
    LOG_TRACE("Len > 16. Len:%d", len);
    v.resize(len); // resize to the number of octets.
    n = sock->read_some(boost::asio::buffer(&v[16], len - 16), error);
  }
  HexDump(&v[0], v.size());
  {
    scoped_lock guard(m_mRecvMsg);
    m_nRecvMsgs++;
  }
  LOG_RET("Returning %d", true);
}

void RemoveSock(socket_ptr sock) {
  std::vector<socket_ptr>::iterator it;
  boost::mutex::scoped_lock guard(Rxmx);
  for (it = RxSocks.begin(); it != RxSocks.end(); it++) {
    if ( (*it) == sock) {
      LOG_INFO("Erasing socket..");
      it = RxSocks.erase(it);
      sock->close();
      break;
    }
  }
}
////////////////////////////////////////////////////////////////

bool WriteDeliverSm(const Smpp::Uint8* buf, unsigned len) {
  LOG_IN("void");

  HexDump(buf, len);
  unsigned index = 0;
  bool status = false;

  unsigned attempt = RxSocks.size();
  while (!status && attempt) {
    static unsigned count = 0;
    {
      boost::mutex::scoped_lock gaurd(mx);
      if (count > RxSocks.size() - 1) {
        count = 0;
      }
      index = count++;
    }
    attempt--;
    if (!RxSocks[index]->is_open()) {
      RemoveSock(RxSocks[index]);
      continue;
    }
    try
    {
       boost::asio::write(*RxSocks[index], boost::asio::buffer(buf, len));
      status = true;
    }
    catch(std::exception & ex) {
    	LOG_ERROR("Failed to send DeliverSm. Error[%s]", ex.what());
      RemoveSock(RxSocks[index]);
    }
  }

  {
    scoped_lock guard(m_mSendMsg);
    m_nSendMsgs++;
  }
  //LOG_TRACE("error [%d]-[%s]", error.value(), error.message().c_str());
  LOG_RET("Returning %d", true);
}
////////////////////////////////////////////////////////////////

bool WriteResponse(socket_ptr sock, const Smpp::Uint8* buf, unsigned len, bool *close_socket) {
  LOG_IN("void");
  LOG_TRACE("Waiting for [%d] micro sec", write_delay);
  LOG_TRACE("Sending response...length[%d]", len);
  HexDump(&buf[0], len);
  //usleep(write_delay);
  boost::system::error_code error;
  int n = boost::asio::write(*sock, boost::asio::buffer(buf, len));
  if (n == -1) {
    LOG_ERROR("Socket Error: Failed to read data");
    *close_socket = true;
    LOG_RET("Returning %d", false);
  }
  {
    scoped_lock guard(m_mSendMsg);
    m_nSendMsgs++;
  }
  //LOG_TRACE("error [%d]-[%s]", error.value(), error.message().c_str());
  LOG_RET("Returning %d", true);
}
///////////////////////////////////////////////////////////////////////////////

bool SendDeliverSm(socket_ptr sock,SmppPduBuffer& vBuffer, bool *sock_closed) {
  LOG_IN("vBuffer[%p]", &vBuffer);

  bool status = false;
  try
  {

    SubmitSm submit;
    submit.decode(&vBuffer[0]);
    Smpp::DeliverSm resp(submit.sequence_number(), submit.service_type(), submit.source_addr(),
      submit.destination_addr(),
      submit.esm_class(), submit.protocol_id(), submit.priority_flag(), submit.schedule_delivery_time(),
      submit.validity_period(), submit.registered_delivery(), submit.replace_if_present_flag(),
      submit.data_coding(), submit.sm_default_msg_id(), submit.short_message());


    const Smpp::Uint8* buf = resp.encode();
    unsigned length = resp.command_length();
    LOG_INFO("Send DeliverSm sm\n");

    status = WriteResponse(sock, buf, length, sock_closed);
   // status = WriteDeliverSm(buf, length);
    //submit_sm.decore((char*) & vBuffer[0]);
  }

  catch(std::exception & ex) {
    LOG_ERROR("Can't send deliver sm. Failed to decode submit_sm message. Error:[%s]", ex.what());
    LOG_RET("Returning:%d", false);
  }
  LOG_RET("Returning:%d", status);
}
///////////////////////////////////////////////////////////////////////////////

bool ProcessRead(socket_ptr sock, SmppPduBuffer& vBuffer, bool *sock_closed) {
  LOG_IN("vBuffer[%p]", &vBuffer);
  bool status = false;
  Smpp::Uint32 cid = 0;
  Smpp::Uint32 seq_num = 0;
  Smpp::Uint32 cmd_dtatus = 0;
  static Smpp::SystemId sys_id("SmscSim");

  try
  {
    cid = Smpp::CommandId::decode((char*) & vBuffer[0]);
    seq_num = Smpp::SequenceNumber::decode((char*) & vBuffer[0]);
    LOG_INFO("Received msg with command_id[%d], sequence_num[%d]", cid, seq_num);
  }

  catch(std::exception & ex) {
    LOG_ERROR("Failed to decode the received data");
    LOG_RET("Returning:%d", false);
  }
  switch (cid) {
      /*response command id from smsc*/
    case Smpp::CommandId::BindReceiver:
    {
      LOG_TRACE("Received Smpp::CommandId::BindReceiver");
      try
      {
        Smpp::BindReceiverResp resp(cmd_dtatus, seq_num, sys_id);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();
        status = WriteResponse(sock, buf, length, sock_closed);
        boost::mutex::scoped_lock guard(Rxmx);
        RxSocks.push_back(sock);

      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }

      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::BindTransmitter:
    {
      LOG_TRACE("Received Smpp::CommandId::BindTransmitter");
      try
      {
        Smpp::BindTransmitterResp resp(cmd_dtatus, seq_num, sys_id);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();
        status = WriteResponse(sock, buf, length, sock_closed);
      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }

      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::SubmitSm:
    {
      LOG_TRACE("Received Smpp::CommandId::SubmitSm");
      try
      {
        std::string sMsgId;
        GetMsgId(sMsgId);
        MessageId msg_id(sMsgId);
        LOG_TRACE("Sending SubmitSm Resp");
        Smpp::SubmitSmResp resp(cmd_dtatus, seq_num, msg_id);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();

       status = WriteResponse(sock, buf, length, sock_closed);

        LOG_TRACE("Successfully sent submit sm response");

        if (deliver_sm) {

          //status = SendDeliverSm(sock, vBuffer, sock_closed);
          Smpp::DeliverSm resp1(&vBuffer[0]);
          LOG_TRACE("DeliverSm: Command id[%d]", resp1.command_id());
          const Smpp::Uint8* buf1 = resp1.encode();
          unsigned length1 = resp1.command_length();
          status = WriteResponse(sock, buf1, length1, sock_closed);

        }
      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }
      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::Unbind:
    {
      LOG_TRACE("Received Smpp::CommandId::Unbind");
      try
      {

        Smpp::UnbindResp resp(cmd_dtatus, seq_num);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();

        status = WriteResponse(sock, buf, length, sock_closed);
        LOG_INFO("Closing connection...");
        RemoveSock(sock);
        *sock_closed = true;


      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);

      }
      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::BindTransceiver:
    {
      LOG_TRACE("Received Smpp::CommandId::BindTransceiver");
      try
      {
        Smpp::BindTransceiverResp resp(cmd_dtatus, seq_num, sys_id);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();
        status = WriteResponse(sock, buf, length, sock_closed);
        boost::mutex::scoped_lock guard(Rxmx);
        RxSocks.push_back(sock);

      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }
      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::EnquireLink:
    {
      LOG_TRACE("Received Smpp::CommandId::EnquireLink");
      try
      {
        Smpp::EnquireLinkResp resp(cmd_dtatus, seq_num);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();
        status = WriteResponse(sock, buf, length, sock_closed);
      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }
      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::DataSm:
    {
      LOG_TRACE("Received Smpp::CommandId::DataSm");
      try
      {
        std::string sMsgId;
        GetMsgId(sMsgId);
        MessageId msg_id(sMsgId);
        Smpp::DataSmResp resp(cmd_dtatus, seq_num, msg_id);
        const Smpp::Uint8* buf = resp.encode();
        unsigned length = resp.command_length();

        status = WriteResponse(sock, buf, length, sock_closed);
        if (deliver_sm) {

          status = SendDeliverSm(sock, vBuffer, sock_closed);

        }
      }

      catch(std::exception & ex) {
        LOG_ERROR("Exception:[%s]", ex.what());
        LOG_RET("Returning:%d", false);
      }
      LOG_RET("Returning :%d", status);
    }

    case Smpp::CommandId::DeliverSmResp:
    {

      LOG_INFO("Received Smpp::CommandId::DeliverSmResp, Sequence num[%d]", seq_num);
      LOG_RET("Returning :%d", true);
    }
    case Smpp::CommandId::QuerySm:
      break;

    case Smpp::CommandId::ReplaceSm:
      break;
    case Smpp::CommandId::CancelSm:
      break;

    case Smpp::CommandId::SubmitMulti:
      break;

    case Smpp::CommandId::BroadcastSm:
      break;
    case Smpp::CommandId::QueryBroadcastSm:
      break;
    case Smpp::CommandId::CancelBroadcastSm:
      break;

    case Smpp::CommandId::Outbind:
      break;

    case Smpp::CommandId::AlertNotification:
      break;

    case Smpp::CommandId::GenericNack:
      break;
    default:
      break;
  }
  LOG_RET("NOT IMPLEMENTED:Returning :%d", false);

}

void session(socket_ptr sock) {
  bool sock_closed = false;
  try
  {

    while (!sock_closed) {
      SmppPduBuffer v;
      ReadSmppPdu(sock, v, &sock_closed);
      if (!sock_closed) {
        ProcessRead(sock, v, &sock_closed);
      }
      if (sock_closed) {
        LOG_INFO("sock closed is true. Closing socket");
        sock->close();
        break;
      }
    }

  }

  catch(std::exception & ex) {
    sock->close();
    LOG_ERROR("Exception:[%s]", ex.what());

  }
}

void print_counter() {
  while (true) {
    sleep(10);
    std::ostringstream ss;
    ss << "=============================================================\n"
      << "\tMessage Received:    " << m_nRecvMsgs << "\n"
      << "\tMessage Sent:        " << m_nSendMsgs << "\n"
      << "=============================================================\n";
    if (std_out) {
      std::cout << ss.str();
    } else {
      LOG_INFO("\n%s", ss.str().c_str());
    }
  }
}

void server(boost::asio::io_service& io_service, short port) {
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  for (;;) {
    socket_ptr sock(new tcp::socket(io_service));
    a.accept(*sock);
    boost::thread t(boost::bind(&SmscSim::session, this, sock));
  }
}
};
int main(int argc, char** argv) {
   SmscSim sim;
  if (argc > 4) {
    char* stdout = argv[4];
    if (strcasecmp(stdout, "true") == 0) {
      sim.std_out = true;
     // log::UseStdOut(true);
     //log::SetFileName(argv[0]);
    }
  }
  boost_logger::instance().init("/home/rjoshi/arawat_dev/config/boost-log.ini");
  log::init(boost_logger::write_log);
  //log::init(write_log);
  if (argc < 2) {
    std::cerr << "Usage SmscSim <port> [deliver_sm] [loglevel] [isStdOut]\n\te.g SmscSim 9000 false INFO yes" << std::endl;
    return (EXIT_SUCCESS);
  }

  if (argc > 3) {
    char* log_level = argv[3];
    if (strcasecmp(log_level, "INFO") == 0) {
      std::cout << "INFO logging is enabled..." << std::endl;
      log::enable_info();
      LOG_INFO("INFO logging is enabled...");
    } else if (strcasecmp(log_level, "TRACE") == 0) {
      std::cout << "TRACE logging is enabled..." << std::endl;
      log::enable_trace();
      LOG_TRACE("Trace logging is enabled...");
    } else if (strcasecmp(log_level, "WARN") == 0) {
        std::cout << "WARN logging is enabled..." << std::endl;
        log::enable_warn();
        LOG_WARN("Warn logging is enabled...");
   } else if (strcasecmp(log_level, "ERROR") == 0) {
          std::cout << "ERROR logging is enabled..." << std::endl;
          log::enable_error();
          LOG_ERROR("ERROR logging is enabled...");
        }
  }
  boost::asio::io_service io_service;
  int nPort = atoi(argv[1]);
  if (nPort < 100) {
    LOG_ERROR("Invalid port number[%d]", nPort);
    return (EXIT_SUCCESS);
  }

  LOG_INFO("Listening on port[%d]", nPort);

  if (argc > 2) {
    if ((strcasecmp(argv[2], "true") == 0) || (strcasecmp(argv[2], "yes") == 0)) {
      sim.deliver_sm = true;
      LOG_INFO("DeliverSm is enabled");
    }
  }


  boost::thread t(boost::bind(&SmscSim::print_counter, boost::ref(sim)));
  sim.server(io_service, nPort);

  return (EXIT_SUCCESS);
}

