/**
 * @file tests.cpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains testing set
 */
#include <iostream>
#include "gtest/gtest.h"
#include <cstdlib>
#include <sys/wait.h>
#include <string.h>
#include <regex>
#include <cstdio>
#include <boost/process.hpp>

namespace bp = boost::process;
using namespace std;

#define TEST_UDP "./ipkcpc -h merlin.fit.vutbr.cz -m udp -p 10002"
#define TEST_TCP "./ipkcpc -h merlin.fit.vutbr.cz -m tcp -p 10002"

#define TEST_WRITE_OUT false // Used for displaying expected and actual values while testing

void test_udp_ok(string in_str, string exp_out)
{
    bp::ipstream pipe;
    bp::opstream in;

    string actual_out;

    bp::child c(TEST_UDP, bp::std_out > pipe, bp::std_in < in);

    in_str.append("\n");

    in.pipe().write(in_str.c_str(), in_str.length());
    in.pipe().close();

    while (pipe && getline(pipe, actual_out) && !actual_out.empty())
    {
        if (TEST_WRITE_OUT)
        {
            cerr << "Actual out - " << actual_out << endl;
        }
        ASSERT_TRUE(actual_out == exp_out);
    }
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_SUCCESS);
}

void test_tcp_ok(string file, string exp_out)
{
    bp::ipstream pipe;

    string actual_out;

    bp::child c(TEST_TCP, bp::std_out > pipe, bp::std_in < file);
    while (pipe && getline(pipe, actual_out) && !actual_out.empty())
    {
        if (TEST_WRITE_OUT)
        {
            cerr << "Actual out - " << actual_out << endl;
        }
        if (!actual_out.compare("HELLO"))
        {
            if (TEST_WRITE_OUT)
            {
                cerr << "Expected - "
                     << "HELLO" << endl;
            }
            ASSERT_TRUE(actual_out == "HELLO");
        }
        else if (!actual_out.compare(exp_out))
        {
            if (TEST_WRITE_OUT)
            {
                cerr << "Expected - " << exp_out << endl;
            }
            ASSERT_TRUE(actual_out == exp_out);
        }
        else if (!actual_out.compare("BYE"))
        {
            if (TEST_WRITE_OUT)
            {
                cerr << "Expected - "
                     << "BYE" << endl;
            }
            ASSERT_TRUE(actual_out == "BYE");
        }
        else
        {
            continue;
        }
    }
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_SUCCESS);
}

void test_udp_err(string in_str)
{
    bp::ipstream pipe;
    bp::opstream in;

    string actual_out;

    bp::child c(TEST_UDP, bp::std_out > pipe, bp::std_in < in);

    in_str.append("\n");

    in.pipe().write(in_str.c_str(), in_str.length());
    in.pipe().close();

    while (pipe && getline(pipe, actual_out) && !actual_out.empty())
    {
        if (TEST_WRITE_OUT)
        {
            cerr << "Actual out - " << actual_out << endl;
        }
        ASSERT_TRUE(regex_match(actual_out, regex("ERR:.*")));
    }
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_SUCCESS);
}

void test_tcp_err(string file)
{
    bp::ipstream pipe;

    string actual_out;

    bp::child c(TEST_TCP, bp::std_out > pipe, bp::std_in < file);
    while (pipe && getline(pipe, actual_out) && !actual_out.empty())
    {
        if (TEST_WRITE_OUT)
        {
            cerr << "Actual out - " << actual_out << endl;
        }
        if (!actual_out.compare("HELLO"))
        {
            if (TEST_WRITE_OUT)
            {
                cerr << "Expected - "
                     << "HELLO" << endl;
            }
            ASSERT_TRUE(actual_out == "HELLO");
        }
        else if (!actual_out.compare("BYE"))
        {
            if (TEST_WRITE_OUT)
            {
                cerr << "Expected - "
                     << "BYE" << endl;
            }
            ASSERT_TRUE(actual_out == "BYE");
        }
        else
        {
            continue;
        }
    }
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_SUCCESS);
}

TEST(Arguments, no_args)
{
    bp::child c("./ipkcpc", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, unknown_args)
{
    bp::child c("./ipkcpc -a -b -c", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, missing_arg_vals)
{
    bp::child c("./ipkcpc -m -h -p", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, missing_host_val)
{
    bp::child c("./ipkcpc -m tcp -h -p 2023", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, missing_mode_val)
{
    bp::child c("./ipkcpc -m -h localhost -p 2023", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, missing_port_val)
{
    bp::child c("./ipkcpc -m tcp -h localhost -p", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, unknown_mode)
{
    bp::child c("./ipkcpc -m ipk -h localhost -p 2023", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, bad_port_over)
{
    bp::child c("./ipkcpc -m udp -h localhost -p 65536", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(Arguments, bad_port_under)
{
    bp::child c("./ipkcpc -m tcp -h localhost -p 1023", bp::std_err > "/dev/null");
    c.wait();
    ASSERT_TRUE(c.exit_code() == EXIT_FAILURE);
}

TEST(UDP_Tests, plus)
{
    test_udp_ok("(+ 1 0)", "OK:1");
    test_udp_ok("(+ 1 -4)", "OK:-3");
    test_udp_ok("(+ 1 2147483647)", "OK:2147483648");
    test_udp_ok("(+ -2147483647 2147483647)", "OK:0");
    test_udp_ok("(+ -2147483647 -2147483647)", "OK:-4294967294");
}

TEST(UDP_Tests, minus)
{
    test_udp_ok("(- 1 0)", "OK:1");
    test_udp_ok("(- 1 -4)", "OK:5");
    test_udp_ok("(- 1 2147483647)", "OK:-2147483646");
    test_udp_ok("(- -2147483647 2147483647)", "OK:-4294967294");
    test_udp_ok("(- 2147483647 4294967294)", "OK:-2147483647");
}

TEST(UDP_Tests, mul)
{
    test_udp_ok("(* 1 0)", "OK:0");
    test_udp_ok("(* 1 -4)", "OK:-4");
    test_udp_ok("(* 1 2147483647)", "OK:2147483647");
    test_udp_ok("(* -2147483647 2)", "OK:-4294967294");
    test_udp_ok("(* -2147483647 -3)", "OK:6442450941");
}

TEST(UDP_Tests, div)
{
    test_udp_err("(/ 1 0)");
    test_udp_ok("(/ 1 -4)", "OK:-1/4");
    test_udp_ok("(/ 1 2147483647)", "OK:1/2147483647");
    test_udp_ok("(/ -2147483647 2147483647)", "OK:-1");
    test_udp_ok("(/ -4294967294 2147483647)", "OK:-2");
}

TEST(UDP_Tests, bad_input)
{
    test_udp_err("(a b)");
    test_udp_err("()");
    test_udp_err("(    )");
    test_udp_err("(+11)");
    test_udp_err("(+ b)");
}

TEST(TCP_Tests, plus)
{
    test_tcp_ok("./tests/tcp_tests/plus1.txt", "RESULT 2");
    test_tcp_ok("./tests/tcp_tests/plus2.txt", "RESULT -3");
    test_tcp_ok("./tests/tcp_tests/plus3.txt", "RESULT 2147483648");
    test_tcp_ok("./tests/tcp_tests/plus4.txt", "RESULT 0");
    test_tcp_ok("./tests/tcp_tests/plus5.txt", "RESULT -4294967294");
}

TEST(TCP_Tests, minus)
{
    test_tcp_ok("./tests/tcp_tests/minus1.txt", "RESULT 1");
    test_tcp_ok("./tests/tcp_tests/minus2.txt", "RESULT 5");
    test_tcp_ok("./tests/tcp_tests/minus3.txt", "RESULT -2147483646");
    test_tcp_ok("./tests/tcp_tests/minus4.txt", "RESULT -4294967294");
    test_tcp_ok("./tests/tcp_tests/minus5.txt", "RESULT -2147483647");
}

TEST(TCP_Tests, mul)
{
    test_tcp_ok("./tests/tcp_tests/mul1.txt", "RESULT 0");
    test_tcp_ok("./tests/tcp_tests/mul2.txt", "RESULT -4");
    test_tcp_ok("./tests/tcp_tests/mul3.txt", "RESULT 2147483647");
    test_tcp_ok("./tests/tcp_tests/mul4.txt", "RESULT -4294967294");
    test_tcp_ok("./tests/tcp_tests/mul5.txt", "RESULT 6442450941");
}

TEST(TCP_Tests, div)
{
    test_tcp_err("./tests/tcp_tests/div1.txt");
    test_tcp_ok("./tests/tcp_tests/div2.txt", "RESULT -1/4");
    test_tcp_ok("./tests/tcp_tests/div3.txt", "RESULT 1/2147483647");
    test_tcp_ok("./tests/tcp_tests/div4.txt", "RESULT -1");
    test_tcp_ok("./tests/tcp_tests/div5.txt", "RESULT -2");
}

TEST(TCP_Tests, bad_input)
{
    test_tcp_err("./tests/tcp_tests/bad1.txt");
    test_tcp_err("./tests/tcp_tests/bad2.txt");
    test_tcp_err("./tests/tcp_tests/bad3.txt");
    test_tcp_err("./tests/tcp_tests/bad4.txt");
    test_tcp_err("./tests/tcp_tests/bad5.txt");
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}