#ifndef IERG4180_PROJECT2_CLIENT_H
#define IERG4180_PROJECT2_CLIENT_H

#endif //IERG4180_PROJECT2_CLIENT_H

# include "util.h"




void print_prompt_information_server();
void argument_parse_client(int argc, char** argv, Client_argument* client_argument);
int client(int argc, char** argv);
void send_sys_packet(Client_argument client_argument);
std :: string get_sys_packet_string(Client_argument client_argument, int new_port);
int create_socket(Client_argument client_argument, char* url);
int http_request(Client_argument client_argument);
int https_request(Client_argument client_argument);
char** getRequestInfo(char* url);


