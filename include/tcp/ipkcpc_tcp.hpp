/**
 * @file ipkcpc_udp.hpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains declarations of functions for TCP
 */
#ifndef IPKCPC_TCP_HPP
#define IPKCPC_TCP_HPP

#include <string>
using namespace std;

void get_message_tcp(string &payload);

void sigint_handler_tcp(int sig);

#ifdef _WIN32
BOOL WINAPI routine_handler_tcp(DWORD dwCtrlType);
#endif

#endif // IPKCPC_TCP_HPP