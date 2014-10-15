/*
 * msgpack_test.cpp
 *
 *  Created on: Feb 28, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */
#include <iostream>
#include "msgpack_util.hpp"
#include <cassert>
#include "log.hpp"
#include "boost_logger.hpp"
#include <gtest/gtest.h>

using namespace arawat::util;

class msgpackutilTest : public ::testing::Test
{
protected:
   msgpackutilTest(){}
   virtual ~msgpackutilTest(){}
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }

};

class smpp_address {
public:
	smpp_address():addr_("8045550000"), ton_(0x0), npi_(0x0), error_code_(-1){}
	std::string addr_;
	boost::uint8_t ton_;
	boost::uint8_t npi_;
	boost::int32_t error_code_;

	MSGPACK_DEFINE(addr_, ton_, npi_, error_code_);
};
class smpp_base {
public:

	 typedef enum {
		SMPP,
		SMTP
	 }protocol_t;


	typedef enum {
			IN,
			OUT
	}dir_t;

	smpp_base():p_(SMPP),d_(IN){}
	boost::uint8_t p_;
	boost::uint8_t d_;
	MSGPACK_DEFINE(p_,d_);
};

class smpp_msg :public smpp_base {
public:
	typedef std::vector<smpp_address> smpp_addresses_t;

	smpp_msg():	command_id_(0x00),seqnum_(0),command_status_(0),
				error_code_(-1){

			}
	boost::uint32_t command_id_;
	boost::uint32_t seqnum_; //unused:0
	boost::uint32_t command_status_;
	int error_code_;
	std::string msgid_; //unused:0x00
	smpp_addresses_t to_list_; //submitmulti

    MSGPACK_DEFINE(p_, d_, command_id_, seqnum_, command_status_, error_code_, msgid_, to_list_);
};

TEST_F(msgpackutilTest, testSmppAddressString) {
    msgpack::sbuffer sbuf;
	smpp_address addr1, addr2;
	addr1.addr_ = "test";
    msgpack_util<smpp_address>::encode(addr1, sbuf);
    msgpack_util<smpp_address>::decode(sbuf, addr2);
    EXPECT_EQ(addr1.addr_ , addr2.addr_);
}
TEST_F(msgpackutilTest, testSmppBaseType) {
    msgpack::sbuffer sbuf;
    smpp_base b1, b2;
    b1.p_ = smpp_base::SMTP;
    msgpack_util<smpp_base>::encode(b1, sbuf);
    msgpack_util<smpp_base>::decode(sbuf, b2);
    EXPECT_EQ(b1.p_ , b2.p_);
}
TEST_F(msgpackutilTest, testSmppMsgAddrSeq) {
    msgpack::sbuffer sbuf;
    smpp_address addr1;
	addr1.addr_ = "test";
    smpp_msg msg1,msg2;
    msg1.to_list_.push_back(addr1);
    msg1.seqnum_ = 1;
    msg1.d_ = smpp_base::OUT;
    msgpack_util<smpp_msg>::encode(msg1, sbuf);
    msgpack_util<smpp_msg>::decode(sbuf, msg2);
    EXPECT_EQ(msg1.seqnum_, msg2.seqnum_);
    EXPECT_EQ((msg1.to_list_[0]).addr_ , (msg2.to_list_[0]).addr_);
    EXPECT_EQ(msg1.d_ , msg2.d_);
}
TEST_F(msgpackutilTest, testSmppMsgBaseAddrSeq) {
    msgpack::sbuffer sbuf;
    smpp_msg msg3;
    smpp_address addr1;
	addr1.addr_ = "test";
    msg3.to_list_.push_back(addr1);
    msg3.seqnum_ = 1;
    msg3.d_ = smpp_base::OUT;
    msgpack_util<smpp_msg>::encode(msg3, sbuf);
    smpp_base b3;
    msgpack_util<smpp_base>::decode(sbuf, b3);
    EXPECT_EQ(b3.p_ , smpp_base::SMPP);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

	return EXIT_SUCCESS;
}
