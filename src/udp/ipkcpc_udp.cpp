/**
 * @file ipkcpc_udp.hpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains definitions of functions for UDP
 */
#ifdef __linux__
#include <sys/socket.h>
extern socklen_t serv_length;
extern int client_sock;
#elif _WIN32
#include <winsock2.h>
extern bool int_flag;
extern int serv_length;
extern SOCKET client_sock;
#define close(socket) closesocket(socket);
#define SHUT_RDWR SD_BOTH
#endif

#include <cstring>
#include <csignal>
#include <iostream>
#include "../../include/udp/ipkcpc_udp.hpp"

/**
 * @brief Gets message from STDIN and formats it for UDP
 *
 * @param payload Return string
 */
void get_message_udp(string &payload)
{
    /*
        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +---------------+---------------+-------------------------------+
        |     Opcode    |Payload Length |          Payload Data         |
        |      (8)      |      (8)      |                               |
        +---------------+---------------+ - - - - - - - - - - - - - - - +
        :                     Payload Data continued ...                :
        + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        |                     Payload Data continued ...                |
        +---------------------------------------------------------------+

    */
    memset(payload.data(), 0, payload.length());
    getline(cin, payload);
    // Shutting down client when eof is loaded (used for testing mainly)
    if (cin.eof())
    {
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        exit(EXIT_SUCCESS);
    }
    payload += '\n'; // Appending \n to string, since it is not present when using getline()
    string tmp;
    tmp = '\0';              // Opcode
    tmp += payload.length(); // Payload Length
    tmp += payload;          // Payload data
    payload = tmp;
}

/**
 * @brief Handler for SIGINT for UDP
 *
 * @param sig SIGINT number
 */
void sigint_handler_udp(int sig)
{
    (void)sig;
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    exit(EXIT_SUCCESS);
}

#ifdef _WIN32
/**
 * @brief Handler for SIGINT for UDP for Windows
 *
 * @param dwCtrlType SIGINT number
 */
BOOL WINAPI routine_handler_udp(DWORD dwCtrlType)
{
    // debug
    int_flag = true;
    cerr << "SIGINT RECEIVED" << endl;
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    WSACleanup();
    ExitProcess(EXIT_SUCCESS);
}
#endif

/**
 * @brief Prints UDP message correctly
 *
 * @param str String to be printed
 */
void print_response_udp(string str)
{
    /*
        Status	Value
        OK	    0
        Error	1

        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +---------------+---------------+---------------+---------------+
        |     Opcode    |  Status Code  |Payload Length | Payload Data  |
        |      (8)      |      (8)      |      (8)      |               |
        +---------------+---------------+---------------+ - - - - - - - +
        :                     Payload Data continued ...                :
        + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        |                     Payload Data continued ...                |
        +---------------------------------------------------------------+
    */
    // Status code checking

    const char *tmp = str.c_str();

    if (str[1] == 0)
    {
        // Printing payload
        cout << "OK:" << tmp + 3 << endl;
    }
    else if (str[1] == 1)
    {
        // Printing payload
        cout << "ERR:" << tmp + 3;
    }
}