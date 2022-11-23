//
// Created by du on 10/20/22.
//

#ifndef IERG4180_PROJECT2_SERVER_H
#define IERG4180_PROJECT2_SERVER_H

#endif //IERG4180_PROJECT2_SERVER_H

# include "util.h"

typedef struct server_argument {
    char* lhost;
    char* lport;
    long rbufsize;
    long sbufsize;
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

void print_prompt_information_server();

void server(int argc, char** argv);

void argument_parse_server(int argc, char** argv, Server_argument* server_argument);

void sys_server_from_client(server_argument server_argument, Sys_packet *sys_packet);

void init_statistics(Statistics* statistics);

void  out_packet(Sys_packet sysPacket);
