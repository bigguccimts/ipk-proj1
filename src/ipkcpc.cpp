/**
 * @file ipkcpc.cpp
 * @author Matúš Ďurica (xduric06@stud.fit.vutbr.cz)-
 * @brief Main source file
 */
#ifdef __linux__
#include <getopt.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
socklen_t serv_length;
int client_sock;
#elif _WIN32
#include <winsock2.h>
int serv_length;
SOCKET client_sock;
#define close(socket) closesocket(socket);
#define SHUT_RDWR SD_BOTH
#endif

#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cstdbool>
#include <iostream>
#include <string>

#include "../include/tcp/ipkcpc_tcp.hpp"
#include "../include/udp/ipkcpc_udp.hpp"
#include "../include/ipkcpc.hpp"

using namespace std;

struct sockaddr_in serv_addr;

bool int_flag = false; // flag for windows sigint

int main(int argc, char **argv)
{
    string host, port, mode;
    if (!check_args(argc, argv, host, port, mode))
    {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            cerr << "Error: WSA error " << WSAGetLastError() << endl;
            return EXIT_FAILURE;
        }
#endif
        int port_num;
        struct hostent *server;
        string buffer = "";
        port_num = stoi(port);
        if (port_num < 1024 || port_num > 65535)
        {
            cerr << "Error: Bad port" << endl;
            return EXIT_FAILURE;
        }

        if ((server = gethostbyname(host.c_str())) == NULL)
        {
            cerr << "Error: Bad host" << endl;
            shutdown(client_sock, SHUT_RDWR);
            return EXIT_FAILURE;
        }
        serv_length = sizeof(serv_addr);

        memset((char *)&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memmove((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
        serv_addr.sin_port = htons(port_num);

        // https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
        struct timeval timeout = {10, 0};
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
        if (!mode.compare("tcp"))
        {
#ifdef __linux__
            struct sigaction sa;
            sa.sa_handler = sigint_handler_tcp;
            sigaction(SIGINT, &sa, NULL);
#elif _WIN32
            SetConsoleCtrlHandler(routine_handler_tcp, TRUE);
#endif
            if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
            {
                cerr << "Error: Socket" << endl;
                return EXIT_FAILURE;
            }
            if ((server = gethostbyname(host.c_str())) == NULL)
            {
                cerr << "Error: Bad host" << endl;
                shutdown(client_sock, SHUT_RDWR);
                close(client_sock);
#ifdef _WIN32
                WSACleanup();
#endif
                return EXIT_FAILURE;
            }

            if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                cerr << "Error: Connection failed" << endl;
                shutdown(client_sock, SHUT_RDWR);
                close(client_sock);
#ifdef _WIN32
                WSACleanup();
#endif
                return EXIT_FAILURE;
            }
            while (true)
            {
                if (!int_flag)
                {
                    get_message_tcp(buffer);
                    if (buffer.compare("\n"))
                    {
                        if ((sendto(client_sock, buffer.c_str(), buffer.length(), 0, (struct sockaddr *)&serv_addr, serv_length)) < 0)
                        {
                            cerr << "Error: Writing" << endl;
                            shutdown(client_sock, SHUT_RDWR);
                            close(client_sock);
#ifdef _WIN32
                            WSACleanup();
#endif
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        continue;
                    }
                    memset(buffer.data(), 0, buffer.length());
                    if ((recvfrom(client_sock, buffer.data(), buffer.length(), 0, (struct sockaddr *)&serv_addr, &serv_length)) < 0)
                    {
                        cerr << "Error: Reading" << endl;
                        shutdown(client_sock, SHUT_RDWR);
                        close(client_sock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return EXIT_FAILURE;
                    }
                    cout << buffer;

                    if (!buffer.compare(0, 3, "BYE"))
                        break;
                }
                else
                {
                    break;
                }
            }
            shutdown(client_sock, SHUT_RDWR);
            close(client_sock);
#ifdef _WIN32
            WSACleanup();
#endif
        }
        else if (!mode.compare("udp"))
        {
#ifdef __linux__
            struct sigaction sa;
            sa.sa_handler = sigint_handler_udp;
            sigaction(SIGINT, &sa, NULL);
#elif _WIN32
            SetConsoleCtrlHandler(routine_handler_udp, TRUE);
#endif
            if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
            {
                cerr << "Error: Socket" << endl;
                return EXIT_FAILURE;
            }

            if (connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                cerr << "Error: Connection failed" << endl;
                shutdown(client_sock, SHUT_RDWR);
                close(client_sock);
#ifdef _WIN32
                WSACleanup();
#endif
                return EXIT_FAILURE;
            }

            while (true)
            {
                if (!int_flag)
                {
                    get_message_udp(buffer);
                    if ((sendto(client_sock, buffer.c_str(), buffer.length(), 0, (struct sockaddr *)&serv_addr, serv_length)) < 0)
                    {
                        cerr << "Error: Writing" << endl;
                        shutdown(client_sock, SHUT_RDWR);
                        close(client_sock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return EXIT_FAILURE;
                    }
                    memset(buffer.data(), 0, buffer.length());
                    if ((recvfrom(client_sock, buffer.data(), buffer.length(), 0, (struct sockaddr *)&serv_addr, &serv_length)) < 0)
                    {
                        cerr << "Error: Reading" << endl;
                        shutdown(client_sock, SHUT_RDWR);
                        close(client_sock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return EXIT_FAILURE;
                    }
                    print_response_udp(buffer);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            cerr << "Error: Unknown mode!" << endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Checks command-line arguments
 *
 * @param argc Count of arguments
 * @param argv Array with arguments
 * @param h Return host string
 * @param p Return port string
 * @param m Return mode string
 * @return int - Returns 1 if any error occured, returns 0 if no errors occured
 */
int check_args(int argc, char **argv, string &h, string &p, string &m)
{
    bool help_flag = false;
    if (argc == 7 || argc == 2)
    {
#ifdef __linux__
        int opt;
        opterr = 0;
        struct option long_options[] = {
            {"host", required_argument, 0, 'h'},
            {"port", required_argument, 0, 'p'},
            {"mode", required_argument, 0, 'm'},
            {"help", no_argument, 0, 't'},
            {0, 0, 0, 0}};
        while ((opt = getopt_long(argc, argv, "h:p:m:", long_options, NULL)) != -1)
        {
            switch (opt)
            {
            case 'h':
                h = optarg;
                break;
            case 'p':
                p = optarg;
                break;
            case 'm':
                m = optarg;
                break;
            case 't':
                help_flag = true;
                break;
            default:
                cerr << "Error: Unknown argument/s!" << endl;
                cerr << "Usage: ipkcpc -h <host> -p <port> -m <mode>" << endl;
                return EXIT_FAILURE;
                break;
            }
        }
#elif _WIN32
        for (int i = 1; i < argc; i++)
        {

            if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host"))
            {
                h = argv[i + 1];
                i++;
            }
            else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
            {
                p = argv[i + 1];
                i++;
            }
            else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mode"))
            {
                m = argv[i + 1];
                i++;
            }
            else if (!strcmp(argv[i], "--help"))
            {
                help_flag = true;
                break;
            }
            else
            {
                cerr << "Error: Unknown argument/s!" << endl;
                cerr << "Usage: ipkcpc -h <host> -p <port> -m <mode>" << endl;
                return EXIT_FAILURE;
                break;
            }
        }
#endif
        if (argc == 2 && help_flag)
        {
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if (argc == 7)
        {
            return EXIT_SUCCESS;
        }
        else
        {
            cerr << "Error: Unknown amount of arguments!" << endl;
            cerr << "Usage: ipkcpc -h <host> -p <port> -m <mode>" << endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        cerr << "Error: Unknown amount of arguments!" << endl;
        cerr << "Usage: ipkcpc -h <host> -p <port> -m <mode>" << endl;
        return EXIT_FAILURE;
    }
}

/**
 * @brief Prints help
 *
 */
void print_help()
{
#ifdef _WIN32
    // Setting displaying UTF-8 correctly
    SetConsoleOutputCP(CP_UTF8);
#endif
    cout << "Made by Matúš Ďurica (xduric06) VUT FIT v Brně 2023" << endl;
    cout << endl;
    cout << "\033[1mNAME\033[0m\n";
    cout << "ipkcpc\t - Client for the IPK Calculator Protocol utilizing TCP or UDP" << endl;
    cout << endl;
    cout << "\033[1mSYNOPSIS\033[0m" << endl;
    cout << "./ipkcpc [--help] -h, --host <host> -p, --port <port> -m, --mode <tcp/udp>" << endl;
    cout << endl;
    cout << "\033[1mEXAMPLES\033[0m" << endl;
    cout << "\033[1mTCP\033[0m - Input:\t\t\033[1mUDP\033[0m - Input:" << endl;
    cout << "HELLO\t\t\t(+ 1 1)" << endl;
    cout << "SOLVE (+ 1 1)\t\t(a b)" << endl;
    cout << "BYE" << endl;
    cout << endl;
    cout << "\033[1mTCP\033[0m - Output:\t\t\033[1mUDP\033[0m - Output:" << endl;
    cout << "HELLO\t\t\tOK:2" << endl;
    cout << "RESULT 2\t\tERR:<error message>" << endl;
    cout << "BYE" << endl;
}