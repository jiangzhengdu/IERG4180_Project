//
// Created by du on 10/20/22.
//

#ifndef IERG4180_PROJECT2_SERVER_H
#define IERG4180_PROJECT2_SERVER_H

#endif //IERG4180_PROJECT2_SERVER_H

# include "util.h"
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <signal.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <openssl/ssl.h>
# include <openssl/err.h>
typedef struct server_argument {
    char* lhost;
    char* lport;
    long rbufsize;
    long sbufsize;
    int stat; // default is 500ms
    int serverModel; // 0 for threadPool 1 for single thread
    int poolSize;  // default is 8
    char* httpPort; // default is 4180
    char* httpsPort; // default is 4181
    int socket;
    SSL_CTX* ctx;
}Server_argument;

typedef struct statistics{
    int pktsize;
    int pktnum;
    int left_pktnum;
    int pktrate;
    int stat;
    clock_t previous_clock;
    double * inter_arrival_list;
    double * J_i_list;
    int cum_packet_number;
    double cum_time_cost;
    double cum_bytes_recv;
    double cum_time_cost_session;
    int previous_SN;
    int total_packet_loss;
    int p_num_index;
}Statistics;

typedef struct multiThreadFunArg{
    Server_argument server_argument;
    Sys_packet sys_packet;
    ThreadPool* pool;
    int socket;
}multiThreadFunArg;



void print_prompt_information_server();

void server(int argc, char** argv);

void argument_parse_server(int argc, char** argv, Server_argument* server_argument);

void sys_server_from_client(server_argument server_argument, Sys_packet *sys_packet);

void init_statistics(Statistics* statistics);

void out_packet(Sys_packet sysPacket);

void multiThreadServer(Server_argument serverArgument);

void client_send_tcp(void *arg);

void client_send_udp(void *argv);

void server_send_udp(void *arg);

void server_send_tcp(void *arg);

void server_response_tcp(void * arg);

void server_response_udp_multi_thread(void * arg);

void fun(void * argv);

void serverProject4(int argc, char **argv);

void https_server(void *arg);

int create_socket(int port);