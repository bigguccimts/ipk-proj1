/**
 * @file ipkcpc_udp.hpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains declarations of functions for UDP
 */
#ifndef IPKCPC_UDP_HPP
#define IPKCPC_UDP_HPP

#include <string>
using namespace std;

void get_message_udp(string &payload);

void sigint_handler_udp(int sig);

#ifdef _WIN32
BOOL WINAPI routine_handler_udp(DWORD dwCtrlType);
#endif

void print_response_udp(string str);

#endif // IPKCPC_UDP_HPP