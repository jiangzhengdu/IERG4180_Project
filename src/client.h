#ifndef IERG4180_PROJECT2_CLIENT_H
#define IERG4180_PROJECT2_CLIENT_H

#endif //IERG4180_PROJECT2_CLIENT_H

# include "util.h"
typedef struct client_argument {
    int mode; // send = 0  recv = 1
    long stat;
    char* rhost;
    char* rport;
    int proto; // udp = 0  tcp = 1
    long pktsize;
    long pktrate;
    long pktnum;
    long sbufsize;
    long rbufsize;
}Client_argument;



void print_prompt_information_server();
void argument_parse_client(int argc, char** argv, Client_argument* client_argument);
int client(int argc, char** argv);
void send_sys_packet(Client_argument client_argument);
std :: string get_sys_packet_string(Client_argument client_argument, int new_port);

