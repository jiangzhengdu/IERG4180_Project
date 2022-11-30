#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef IERG4180_PROJECT2_UTIL_H
#define IERG4180_PROJECT2_UTIL_H

#endif //IERG4180_PROJECT2_UTIL_H

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
#elif __linux__
# include <time.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <sys/fcntl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
# include <errno.h>
# include <sys/select.h>
# include <sys/types.h>
# include <sys/ioctl.h>
# include <sys/fcntl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <net/if.h>
# include <arpa/inet.h>
# include "threadpool.h"
#endif
# include <iostream>
# include <string.h>
# include <string>
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <sstream>
# define sys_packet_size 57
# define Jitter_max 100000
const int MAX_BUFFER_RECV = 65536;
typedef struct sys_packet{
    int mode; // 0: client send  1: server send  2: response
    int proto; // 0 udp  1 tcp
    int pktsize;
    int pktnum;
    int pktrate;
    int client_port;
    char * client_ip;
}Sys_packet;

typedef struct client_argument {
    int mode; // send = 0  recv = 1  response = 2
    long stat;
    char* rhost;
    char* rport;
    int proto; // udp = 0  tcp = 1
    long pktsize;
    long pktrate; // for send/recv 1000bps for response 10/s
    long pktnum;
    long sbufsize;
    long rbufsize;
    char* url;
    char* fileName;
}Client_argument;


int  solve_hostname_to_ip_linux(const char* hostname, char* ip);
char* generate_message(int message_length, int sequence_number);
int get_sequence_number(char* message);
long SetSendBufferSizeLinux(long size, int sockfd);
long SetReceiveBufferSizeLinux(long size, int sockfd);
double calculate_average_value(double* list, int n);
int get_free_port();
int get_ip(char *);
std :: string my_int_to_string(int value);
std :: string add_zero(std :: string old_string, int n_zero, char ch = '0');
void parse_sys_packet(const char * sys_packet_buf, Sys_packet * sys_packet);
int tcp_recv(int socket, int pktsize,
              clock_t * previous_clock, double * inter_arrival_list, double * J_i_list,
              int* cum_packet_number,double * cum_time_cost, double * cum_bytes_recv, int stat,
              double* cum_time_cost_session,  int using_select);
int tcp_send(int socket, int pktsize, int* left_pktnum, int pktnum,  int pktrate, int stat, int using_select);
int udp_send(int socket, int * p_num_index, int pktsize, int* left_pktnum, int pktnum, int pktrate, int stat, struct sockaddr_in target_address, int using_select);
int udp_recv(int socket, int pktsize, clock_t * previous_clock, double * inter_arrival_list, double * J_i_list,
             int* cum_packet_number,double * cum_time_cost, double * cum_bytes_recv, int stat,
             double* cum_time_cost_session, int* previous_SN, int *total_packet_loss, int using_select);
int InitializeWinsock();
int server_response_udp(int socket, struct sockaddr_in client_address, int client_port);
int client_response_tcp(struct sockaddr_in server_address, Client_argument client_argument);
int client_response_udp(int socket, struct sockaddr_in client_address, Client_argument client_argument, struct sockaddr_in server_address);


