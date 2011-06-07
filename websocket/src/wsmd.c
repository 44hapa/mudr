#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "md5.c"
#include "websocket.c"

#define PORT 8080
#define BUF_LEN 0x1FF
#define PACKET_DUMP

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

int terminate = FALSE;

int client_worker(int clientsocket)
{
    static uint8_t buffer[BUF_LEN];
    size_t readed_length = 0;
    size_t out_len = BUF_LEN;
    int written = 0;
    enum ws_frame_type frame_type = WS_INCOMPLETE_FRAME;
    struct handshake hs;
    nullhandshake(&hs);


    // read openinig handshake
    while (frame_type == WS_INCOMPLETE_FRAME) {
        int readed = recv(clientsocket, buffer+readed_length, BUF_LEN-readed_length, 0);
        if (!readed) {
            fprintf(stderr, "Recv failed: %d\n", errno);
            close(clientsocket);
            return EXIT_FAILURE;
        }
        #ifdef PACKET_DUMP
            printf("In packet:\n");
            fwrite(buffer, 1, readed, stdout);
            printf("\n");
        #endif
        readed_length+= readed;
        assert(readed_length <= BUF_LEN);
        frame_type = ws_parse_handshake(buffer, readed_length, &hs);
        if (frame_type == WS_INCOMPLETE_FRAME && readed_length == BUF_LEN) {
            fprintf(stderr, "Buffer too small\n");
            close(clientsocket);
            return EXIT_FAILURE;
        } else
        if (frame_type == WS_ERROR_FRAME) {
            fprintf(stderr, "Error in incoming frame\n");
            close(clientsocket);
            return EXIT_FAILURE;
        }
    }
    assert(frame_type == WS_OPENING_FRAME);

    // if resource is right, generate answer handshake and send it
    if (strcmp(hs.resource, "/echo") != 0) {
        fprintf(stderr, "Resource is wrong:%s\n", hs.resource);
        close(clientsocket);
        return EXIT_FAILURE;
    }
    out_len = BUF_LEN;
    ws_get_handshake_answer(&hs, buffer, &out_len);
    #ifdef PACKET_DUMP
        printf("Out packet:\n");
        fwrite(buffer, 1, out_len, stdout);
        printf("\n");
    #endif
    written = send(clientsocket, buffer, out_len, 0);
    if (written == SOCKET_ERROR) {
        fprintf(stderr, "Send failed: %d\n", errno);
        close(clientsocket);
        return EXIT_FAILURE;
    }
    if (written != out_len) {
        fprintf(stderr, "Written %u of %u\n", written, out_len);
        close(clientsocket);
        return EXIT_FAILURE;
    }
    
    // connect success!
    // read incoming packet and parse it;
    readed_length = 0;
    frame_type = WS_INCOMPLETE_FRAME;
    while (frame_type == WS_INCOMPLETE_FRAME) {
        int readed = recv(clientsocket, buffer+readed_length, BUF_LEN-readed_length, 0);
        if (!readed) {
            fprintf(stderr, "Recv failed: %d\n", errno);
            close(clientsocket);
            return EXIT_FAILURE;
        }
        #ifdef PACKET_DUMP
            printf("In packet:\n");
            fwrite(buffer, 1, readed, stdout);
            printf("\n");
        #endif
        readed_length+= readed;
        assert(readed_length <= BUF_LEN);
        size_t data_len;
        uint8_t *data;
        frame_type = ws_parse_input_frame(buffer, readed_length, &data, &data_len);
        if (frame_type == WS_INCOMPLETE_FRAME && readed_length == BUF_LEN) {
            fprintf(stderr, "Buffer too small\n");
            close(clientsocket);
            return EXIT_FAILURE;
        } else
        if (frame_type == WS_CLOSING_FRAME) {
            send(clientsocket, "\xFF\x00", 2, 0); // send closing frame
            close(clientsocket); // and close connection
            break;
        } else
        if (frame_type == WS_ERROR_FRAME) {
            fprintf(stderr, "Error in incoming frame\n");
            close(clientsocket);
            return EXIT_FAILURE;
        } else
        if (frame_type == WS_TEXT_FRAME) {
            out_len = BUF_LEN;
            frame_type = ws_make_frame(data, data_len, buffer, &out_len, WS_TEXT_FRAME);
            if (frame_type != WS_TEXT_FRAME) {
                fprintf(stderr, "Make frame failed\n");
                close(clientsocket);
                return EXIT_FAILURE;
            }
            #ifdef PACKET_DUMP
                printf("Out packet:\n");
                fwrite(buffer, 1, out_len, stdout);
                printf("\n");
            #endif
            written = send(clientsocket, buffer, out_len, 0);
            if (written == SOCKET_ERROR) {
                fprintf(stderr, "Send failed: %d\n", errno);
                close(clientsocket);
                return EXIT_FAILURE;
            }
            if (written != out_len) {
                fprintf(stderr, "Written %u of %u\n", written, out_len);
                close(clientsocket);
                return EXIT_FAILURE;
            }
            readed_length = 0;
            frame_type = WS_INCOMPLETE_FRAME;
        }
    } // read/write cycle

    close(clientsocket);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    int result;
    int listensocket = socket(AF_INET,SOCK_STREAM, 0);
    if (listensocket == INVALID_SOCKET) {
        fprintf(stderr, "Create socket failed: %ld\n", errno);
        exit(2);
    }

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);
    result = bind(listensocket, (struct sockaddr*)&local, sizeof(local));
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed: %ld\n", errno);
        return EXIT_FAILURE;
    }

    result = listen(listensocket, 1);
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed: %ld\n", errno);
        return EXIT_FAILURE;
    }

    printf("Server started at localhost:%d...\n", PORT);

    while (!terminate) {
        struct sockaddr_in remote;
        int sockaddr_len = sizeof(remote);
        int clientsocket = accept(listensocket, (struct sockaddr*)&remote, &sockaddr_len);
        if (clientsocket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed: %d\n", errno);
            return EXIT_FAILURE;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(remote.sin_addr), ip, INET_ADDRSTRLEN);
        printf("Connected %s:%d\n", ip, ntohs(remote.sin_port));

        client_worker(clientsocket);
        printf("Next...\n");
    }

    close(listensocket);
    return (EXIT_SUCCESS);
}


