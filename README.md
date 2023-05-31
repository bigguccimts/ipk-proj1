# Documentation of IPK Project 1 - Client for remote calculator

## Table of contents
1. [Theory](#theory)
    1. [TCP](#theory-tcp)
    2. [UDP](#theory-udp)
2. [Portability](#portability)
3. [Testing](#testing)
4. [Known bugs](#knownbugs)
5. [Bibliography](#bibliography)

#  <a name="theory"></a>Theory
Application ```ipkcpc``` is used as a client to a remote calculator, which uses the IPK Calculator Protocol (IPKCP).

### Usage
```utf-8
./ipkcpc [--help] -h, --host <host> -p, --port <port> -m, --mode <tcp/udp>
```
```utf-8
-p, --port      - specifies port, where the server is waiting for communication
-h, --host      - specifies the host, where the ipkcp server is hosted
-m, --mode      - specifies the mode of the communication (tcp/udp)
--help          - displays help
```
### Functionality
First thing being checked are the command-line arguments. After those, based on the mode selected, connection is estabilished. 

##  <a name="theory-tcp"></a>TCP
Client must begin communication by sending ```HELLO``` message to estabilish communication. Server after recieving the ```HELLO``` message sends ```HELLO``` message back to the client and transitions to the Estabilished state. In this state the client can send queries to be solved in the following format: ```"SOLVE" SP query LF```. Client ends communication by sending ```BYE``` message or using C-c, which gracefully exits [1].
</br>
Application is exited only when the server responds with ```BYE``` message.

##  <a name="theory-udp"></a>UDP
Every message in UDP communication in the ipkcpc application has to begin with an opcode 0, which means we are sending request. Next byte contains the length of the payload and the rest of the UDP packet contains the payload which is encoded as ASCII string. [1]
</br>
Application can be exited using C-c.

#  <a name="portability"></a>Portability
Application is fully ported to Windows [4][5]. It can be built using ```make``` command (it is needed that GNU make is installed on Windows). 
</br>
Using sockets on Windows is really similar to Unix sockets, yet some changes had to be done. For example:

```c++
WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            cerr << "Error: WSA error " << WSAGetLastError() << endl;
            return EXIT_FAILURE;
        }
```
The ```WSAStartup``` function initiates use of the Winsock DLL [6]. Without this, the sockets would not work.

Also after every closing of the socket, function ```WSACleanup``` has to be called. Maybe the main difference is that in Unix, sockets are essentially of the datatype ```int```, while in Windows, sockets are of the datatype ```SOCKET```.

Correct header files were assured by using ```#ifdef``` clauses, where the condition was the OS itself:

```c++
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
```

#  <a name="testing"></a>Testing
Testing set was creating using ```googletest``` [2] as the main testing framework and ```boost::process``` [3] libraries for starting process of the client application and piping ```stdout``` into variable, so output could be compared with expected values. 
</br>
Testing set is contained in folder ```tests/``` in ```tests.cpp``` source file. Tests were compiled and ran using ```make test``` on the reference VM with NixOS. Output of the testing set can be seen in file ```tests_out.txt``` in the ```tests/``` directory
</br>
Changing value of macro ```TEST_WRITE_OUT``` from ```false``` to ```true``` will display actual and expected values within tests.

Example test from test suite UDP_Tests:
```c++
test_udp_ok("(+ 1 0)", "OK:1");
test_udp_ok("(+ 1 -4)", "OK:-3");
test_udp_ok("(+ 1 2147483647)", "OK:2147483648");
test_udp_ok("(+ -2147483647 2147483647)", "OK:0");
test_udp_ok("(+ -2147483647 -2147483647)", "OK:-4294967294");
```
Example function for testing:
```c++
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
```

#  <a name="knownbugs"></a>Known bugs
Application does not exit with exit code 0 when C-c is used in Windows.

#  <a name="bibliography"></a>Bibliography
Parts of my code were paraphrased from some of these sources:

[1] https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md

[2] https://github.com/google/googletest

[3] https://www.boost.org/doc/libs/1_64_0/doc/html/process.html

[4] https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2

[5] https://www.binarytides.com/winsock-socket-programming-tutorial/

[6] https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup