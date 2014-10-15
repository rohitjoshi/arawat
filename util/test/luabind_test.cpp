/*
 * luabind_test.cpp
 *
 *  Created on: Mar 4, 2012
 *      Author: rjoshi
 *  Joshi Inc Reserved 2010
 */

#include "luabind_util.hpp"
#include <gtest/gtest.h>

using namespace arawat::util;

/**
 * LuaBind test
 */
class luabindutlTest : public ::testing::Test
{
protected:
	luabindutlTest(){}
    virtual ~luabindutlTest(){}
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }

};
/**
 * Test load file failure
 */
TEST_F(luabindutlTest, testLoadFileFailure) {
	result_t r = luabind_util::load_file("dummy.lua");
	std::cout << "testLoadFailure:" << r.get<1> () << std::endl;
	EXPECT_EQ(r.get<0> () , false);
}
/**
 * Test load file
 */
TEST_F(luabindutlTest, testLoadFileSuccess) {
	result_t r = luabind_util::load_file("luabind_test.lua");
	EXPECT_EQ(r.get<0> () , true);
}


/**
 * Test lua function add
 */
TEST_F(luabindutlTest, testCallFunctionAddSuccess) {
	int value = luabind::call_function<int>(luabind_util::get_lua_state(), "testadd", 2, 3);
	EXPECT_EQ(value , 5);
}

/**
 * Test lua function add
 */
TEST_F(luabindutlTest, testCallFunctionReturnStringSuccess) {
	std::string s = "original";
	std::string result = s;
	try {
	result = luabind::call_function<std::string>(luabind_util::get_lua_state(), "teststring", s);
	EXPECT_NE(s , result);
	}catch (luabind::error& ex) {
	  std::cerr << "Luabind::Exception: "<< ex.what() << std::endl;

	} catch (std::exception& ex) {
		std::cerr << "std::Exception: "<< ex.what() << std::endl;
	}
}

/**
 * Load lua string
 */
TEST_F(luabindutlTest, testLoadStringSuccess) {
	result_t r = luabind_util::load_string(
	    	"function testmul(a, b)\n"
	    		"return a*b\n"
	    	"end\n");
	EXPECT_EQ(r.get<0> () , true);
}
/**
 * Test function call multiple from loaded string
 */
TEST_F(luabindutlTest, testCallFunctionMultiplySuccess) {
	int value = luabind::call_function<int>(luabind_util::get_lua_state(), "testmul", 2, 3);
	EXPECT_EQ(value , 6);
}
/**
 * main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	    return RUN_ALL_TESTS();
}
