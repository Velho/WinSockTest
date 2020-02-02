
#include <windows.h>
#include <winsock.h>

#include <stdio.h>

#include <stdint.h>

#define DEFAULT_PORT 27015

typedef struct master {
    int server_socket;


    uint8_t buffer[128];

} master_t;

static master_t master;


int main(int argc, char* argv[])
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    master.server_socket = socket(AF_INET, SOCK_STREAM, 0);


    if (connect(master.server_socket, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Failed to connect\n.");
        printf("WSA : %d", WSAGetLastError());
    }


    uint8_t buf[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    for ( int i = 0; i < 10; i++) {
        send(master.server_socket, buf, strlen(buf), 0);

        Sleep(5000);
        closesocket(master.server_socket);

        recv(master.server_socket, master.buffer, sizeof(master.buffer), 0);
    }

    WSACleanup();
}