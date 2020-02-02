
#include <winsock.h>

#include <stdio.h>

#include <stdint.h>

#define DEFAULT_PORT 27015


typedef enum {
    S_STOPPED,
    S_OPENING,
    S_RUNNING,
    S_CLOSING
} socket_status_t;

typedef struct context {
    int client_socket;
    int server_socket;

    struct sockaddr_in server_addr;
    socket_status_t status;

    uint8_t buffer[25];

} context_t;

static context_t context;

int init_winsock()
{
    int result;
    WSADATA wsaData;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    return result;
}

int free_winsock()
{
    return WSACleanup();
}

int connect_client_socket()
{
    int bool_opt; // Reuse the Server Socket.

    // Setup the server socket sockaddr.
    context.server_addr.sin_port = htons(DEFAULT_PORT);
    context.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    context.server_addr.sin_family = AF_INET;

    context.server_socket = socket(AF_INET, SOCK_STREAM, 0);

    bool_opt = TRUE;
    setsockopt(context.server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&bool_opt, sizeof(bool_opt));

    // Bind the server Socket.
    bind(context.server_socket, (struct sockaddr*) & context.server_addr, sizeof(context.server_addr));
    listen(context.server_socket, 1);

    struct timeval tv;
    struct sockaddr addr;
    int addr_sz = sizeof(addr);

    u_long ioMode = 1; // 0 = Blocking, 1 = Non-Blocking.

    // This is run under a state machine, does it really need to be looped through.. ?

    while (TRUE) {
        context.client_socket = accept(context.server_socket, &addr, &addr_sz);

        // Setup the Socket as Non Blocking.
        ioctlsocket(context.client_socket, FIONREAD, &ioMode);

        if (context.client_socket == INVALID_SOCKET)
        {
            context.status = S_CLOSING;
            return -1;
        }

        // We should have valid socket here.
        // socket == 0 , should not be valid.
        if (context.client_socket > 0) {
            memset(&tv, 0, sizeof(tv));

            tv.tv_sec = 30; // 30 seconds timeout.

            setsockopt(context.client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

            break;
        }

    }

    context.status = S_RUNNING;

    return 0;
}

int close_client_socket()
{
    shutdown(context.client_socket, 2);
    shutdown(context.server_socket, 2);

    return 0;
}

int main(int argc, char* argv[])
{
    if (init_winsock()) {
        return 1;
    }

    context.status = S_OPENING;

    while (TRUE) {
        if (context.status == S_OPENING) {
            connect_client_socket();
            continue;
        }
        else if (context.status == S_CLOSING) {
            // Close the Master Socket.
            close_client_socket();
        }

        // recv(..) from the connected Master -socket.

        while ( 1 ) {
            int res = recv(context.client_socket, context.buffer, sizeof(context.buffer), 0);

            if (res > 0) {
                printf("Buffer : %s\n", context.buffer);
            }

            Sleep(100);

            // Error Check
            if (res == 0) {
                const char* foo = { 0x00 };
                res = send(context.client_socket, foo, 1, 0);

                if (res == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
                    printf("Testing the Client socket.\n");
                    printf("Failed with WSAECONNRESET\n");
                    break;
                }

            }

            //res = recv(context.client_socket, NULL, 0, 0);

            //if (res == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
            //    printf("Testing the Client socket.\n");
            //    printf("Failed with WSAECONNRESET\n");
            //    break;
            //}

            
        }

        break;
    }

    printf("Exiting slave.\n");

    WSACleanup();
}