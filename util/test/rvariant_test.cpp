/*
 * rvariant_test.cpp
 *
 *  Created on: Feb 28, 2011
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#include "rvariant.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include "log.hpp"
#include "boost_logger.hpp"
#include <gtest/gtest.h>

using namespace arawat::util;

class rvariantTest : public ::testing::Test
{
protected:
   rvariantTest(){}
   virtual ~rvariantTest(){}
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }

};



TEST_F(rvariantTest, test_rvariant) {
	rvariant r(1);
	EXPECT_EQ(1 , r.get<boost::int32_t>());

	//string
	std::string s = "123";
	rvariant r1 = s;
	EXPECT_EQ(s , r1.get<std::string>());

	//int32_t
	boost::int32_t i = 123;
	rvariant r2 = i;
	EXPECT_EQ(i , r2.get<boost::int32_t>());

	//uuid
	boost::uuids::random_generator gen;
	boost::uuids::uuid uid = gen();
	boost::uuids::uuid t1(gen());
	rvariant bv = t1;
	EXPECT_EQ(t1 , bv.get<boost::uuids::uuid>()) ;


}
TEST_F(rvariantTest, test_rvariant_type_bool) {
    LOG_IN("");
	rvariant_type bv = false;
	EXPECT_EQ(false, boost::get<bool>(bv)) ;
	bv = true;
	EXPECT_EQ(true, boost::get<bool>(bv)) ;
    rvariant_type b1 = true;
    rvariant_type b2 = true;

   EXPECT_EQ(boost::get<bool>(b1), boost::get<bool>(b2));
	LOG_OUT("");
}

TEST_F(rvariantTest, test_rvariant_type_string) {
	LOG_IN("");
	std::string t1("test1"), t2("test2");
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<std::string>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<std::string>(bv)) ;
	EXPECT_EQ(t2 , boost::get<std::string>(bv)) ;
	LOG_OUT("");
}
TEST_F(rvariantTest, test_rvariant_type_int32_t) {
	LOG_IN("");
	boost::int32_t t1(1), t2(2);
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<boost::int32_t>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<boost::int32_t>(bv)) ;
	EXPECT_EQ(t2 , boost::get<boost::int32_t>(bv)) ;
	LOG_OUT("");
}
TEST_F(rvariantTest, test_rvariant_type_uint32_t) {
	LOG_IN("");
	boost::uint32_t t1(1), t2(2);
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<boost::uint32_t>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<boost::uint32_t>(bv)) ;
	EXPECT_EQ(t2 , boost::get<boost::uint32_t>(bv)) ;
	LOG_OUT("");
}
TEST_F(rvariantTest, test_rvariant_type_uint8_t) {
	LOG_IN("");
	boost::uint8_t t1(1), t2(2);
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<boost::uint8_t>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<boost::uint8_t>(bv)) ;
	EXPECT_EQ(t2 , boost::get<boost::uint8_t>(bv)) ;
	LOG_OUT("");
}
TEST_F(rvariantTest, test_rvariant_type_double) {
	LOG_IN("");
	double t1(1.1), t2(2.2);
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<double>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<double>(bv)) ;
	EXPECT_EQ(t2 , boost::get<double>(bv)) ;
	LOG_OUT("");
}
#
TEST_F(rvariantTest, test_rvariant_type_uuid) {
	LOG_IN("");
	boost::uuids::random_generator gen;
	boost::uuids::uuid uid = gen();
	boost::uuids::uuid t1(gen()), t2(gen());
	rvariant_type bv = t1;
	EXPECT_EQ(t1 , boost::get<boost::uuids::uuid>(bv)) ;
	bv = t2;
	EXPECT_NE(t1 , boost::get<boost::uuids::uuid>(bv)) ;
	EXPECT_EQ(t2 , boost::get<boost::uuids::uuid>(bv)) ;
	LOG_OUT("");
}
#if 0
TEST_F(rvariantTest, test_rvariant_type_vector) {
	LOG_IN("");
	std::vector<rvariant_type > t1, t2;
	t1.push_back(1);
	t1.push_back(2);
	t2.push_back(1);
	t2.push_back("2");
	t1.push_back(t2);

	rvariant_type bv = t1;
	EXPECT_EQ(t1.size() , (boost::get<std::vector<rvariant_type > >(bv)).size()) ;
	assert(t1 == boost::get<std::vector<rvariant_type > >(bv)) ;

	(boost::get<std::vector<rvariant_type > >(bv)).push_back(t2);
	assert(boost::get<std::vector<rvariant_type > >(t1[2]) == t2) ;

	std::vector<rvariant_type > t3= boost::get<std::vector<rvariant_type > >(bv);
	assert(t1[0] == t3[0]) ;
	assert(t1[0] == t2[0]) ;
	EXPECT_EQ(1 ,  boost::get<int>(t3[0]));


	LOG_OUT("");
}
TEST_F(rvariantTest, test_rvariant_type_map) {
	LOG_IN("");
	std::map<std::string, rvariant_type > t1,t2;

	t1["1"] = 1;
	t1["2"] = 2;
	t2["1"] = 1;
	t2["2"] = "2";
	t1["t2"] = t2;

	rvariant_type bv = t1;
	EXPECT_EQ(t1.size() , (boost::get<std::map<std::string, rvariant_type > >(bv)).size()) ;
	assert(t1 == (boost::get<std::map<std::string, rvariant_type > >(bv))) ;
	(boost::get<std::map<std::string, rvariant_type > >(bv))["t2"] = t2;
	assert((boost::get<std::map<std::string, rvariant_type > >(t1["t2"]))== t2) ;
	std::map<std::string, rvariant_type > t3= boost::get<std::map<std::string, rvariant_type > >(bv);
	assert(t1["1"] == t3["1"]) ;
	assert(t1["1"] == t2["1"]) ;
	EXPECT_EQ(1 ,  boost::get<int>(t3["1"]));


	LOG_OUT("");
}
#endif
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
	//return EXIT_SUCCESS;
}

