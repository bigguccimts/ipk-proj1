/**
 * @file ipkcpc_udp.hpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)
 * @brief File contains definitions of functions for TCP
 */
#ifdef __linux__
#include <sys/socket.h>
extern socklen_t serv_length;
extern int client_sock;
#elif _WIN32
#include <winsock2.h>
extern int serv_length;
extern bool int_flag;
extern SOCKET client_sock;
#define close(socket) closesocket(socket);
#define SHUT_RDWR SD_BOTH
#endif

#include <cstring>
#include <csignal>
#include <iostream>
#include "../../include/tcp/ipkcpc_tcp.hpp"

extern struct sockaddr_in serv_addr;

/**
 * @brief Gets message from STDIN and formats it for TCP
 *
 * @param payload Return string
 */
void get_message_tcp(string &payload)
{
#ifdef _WIN32
    if (!int_flag)
    {
#endif
        getline(cin, payload);
        payload += '\n'; // Appending \n to string, since it is not present when using getline()
#ifdef _WIN32
    }
    else
    {
        payload = "BYE\n";
    }
#endif
}

#ifdef _WIN32
/**
 * @brief Handler for SIGINT for TCP for Windows
 *
 * @param dwCtrlType SIGINT number
 */
BOOL WINAPI routine_handler_tcp(DWORD dwCtrlType)
{
    int_flag = true;
    string buf = "BYE\n";
    if ((sendto(client_sock, buf.c_str(), buf.length(), 0, (struct sockaddr *)&serv_addr, serv_length)) < 0)
    {
        cerr << "Error: Writing" << endl;
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    memset(buf.data(), 0, buf.length());
    if ((recvfrom(client_sock, buf.data(), buf.length(), 0, (struct sockaddr *)&serv_addr, &serv_length)) < 0)
    {
        cerr << "Error: Reading" << endl;
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << buf;
    cout.flush();
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    WSACleanup();
    ExitProcess(EXIT_SUCCESS);
}
#endif

/**
 * @brief Handler for SIGINT for TCP
 *
 * @param sig SIGINT number
 */
void sigint_handler_tcp(int sig)
{
    (void)sig;
    // Sending BYE to server for grateful exit
    string buf = "BYE\n";
    if ((sendto(client_sock, buf.c_str(), buf.length(), 0, (struct sockaddr *)&serv_addr, serv_length)) < 0)
    {
        cerr << "Error: Writing" << endl;
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    memset(buf.data(), 0, buf.length());
    if ((recvfrom(client_sock, buf.data(), buf.length(), 0, (struct sockaddr *)&serv_addr, &serv_length)) < 0)
    {
        cerr << "Error: Reading" << endl;
        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    cout << buf;
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    exit(EXIT_SUCCESS);
}