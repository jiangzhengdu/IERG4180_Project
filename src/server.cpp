
//// \\Mac\Home\Documents\GitHub\IERG4180_Project1\NetProbe\x64\Debug
//// \\Mac\Home\Documents\GitHub\example

# include "server.h"
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <signal.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <openssl/ssl.h>
# include <openssl/err.h>

int main(int argc, char **argv) {
    print_prompt_information_server();
    serverProject4(argc, argv);
    return 0;
}

// this function is for receive
void server(int argc, char **argv) {
    Server_argument server_argument;
    argument_parse_server(argc, argv, &server_argument);

    printf("the following content is the server arguments\n\n");
    printf("stat is %d\n", server_argument.stat);
    printf("lhost is %s\n", server_argument.lhost);
//    printf("lport is %s\n", server_argument.lport);;
    printf("http port is %s\n", server_argument.httpPort);;
    printf("https port is %s\n", server_argument.httpsPort);;
//    printf("sbufsize is %ld\n", server_argument.sbufsize);
//    printf("rbufsize is %ld\n", server_argument.rbufsize);
    if (server_argument.serverModel == 0) {
        printf("server model is using thread pool\n");
        printf("poolsize is %d\n", server_argument.poolSize);
    } else if (server_argument.serverModel == 1) {
        printf("server model is using single thread\n");
    }


    printf("\n\n\n");
    if (server_argument.serverModel == 0) {
        multiThreadServer(server_argument);
        return;
    }
    int server_listen_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char ip[100];
    solve_hostname_to_ip_linux(server_argument.lhost, ip);
    printf("%s listening to %s\n", server_argument.lhost, ip);

    // Creating socket file descriptor
    if ((server_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    SetSendBufferSizeLinux(server_argument.sbufsize, server_listen_socket);
    SetReceiveBufferSizeLinux(server_argument.rbufsize, server_listen_socket);

    address.sin_family = AF_INET;
    if (strcmp(ip, "127.0.0.1") == 0) {
        address.sin_addr.s_addr = INADDR_ANY;
    } else address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(atoi(server_argument.lport));

    // Forcefully attaching socket to the port
    if (bind(server_listen_socket, (struct sockaddr *) &address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);

    }
    char *realIp = (char *) malloc(20 * sizeof(char));
    memset(realIp, '\0', 20 * sizeof(char));
    get_ip(realIp);
    printf("server listen to real ip : %s the network port %d (host port %s)\n", realIp, (address.sin_port),
           server_argument.lport);
    if (listen(server_listen_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    free(realIp);
    const int maxSockets = 11; // at most 10 concurrent clients
    // this is socket config list
    Sys_packet socketConfigList[maxSockets];
    socketConfigList[0].mode = 0;
    Statistics statisticsList[maxSockets];
    // init the statisticsList
    for (int i = 1; i < maxSockets; i++) {
        init_statistics(&statisticsList[i]);
    }
    int socketHandles[maxSockets]; // arrays for the socket handles
    int socketTcpForUdp[maxSockets]; // arrays of tcp socket for the udp
    bool socketValid[maxSockets]; // bitmask to manage the array
    int response_udp_server_binding_ports_list[maxSockets];
    int portServerForTcpResponse = 0;
    int numActiveSockets = 1;
    struct sockaddr_in client_address[maxSockets];
    for (int i = 1; i < maxSockets; i++) {
        socketValid[i] = false;
        socketConfigList[i].mode = 0; // default is recv mode
    }
    socketHandles[0] = server_listen_socket;
    socketValid[0] = true;
    fd_set fdReadSet;
    fd_set fdWriteSet;
    while (1) {
        // set up the fd_set
        int topActiveSocket = 0;
        FD_ZERO(&fdReadSet);
        FD_ZERO(&fdWriteSet);
        for (int i = 0; i < maxSockets; i++) {
            if (socketValid[i]) { // socket[i] is ture
                // FD_SET(socketHandles[i], &fdReadSet);
                if (socketConfigList[i].mode == 0) {
                    // printf("put it into fread %d\n", i);
                    FD_SET(socketHandles[i], &fdReadSet);
                    // specially for server receive udp, also add it to the fdReadSet
                    if (socketConfigList[i].proto == 0) {
                        FD_SET(socketTcpForUdp[i], &fdReadSet);
                    }
                } else if (socketConfigList[i].mode == 1) {
//                    printf("put it into fw %d\n", i);
                    FD_SET(socketHandles[i], &fdWriteSet);
                    // specially for server send udp , also add it to the fdReadset
                    if (socketConfigList[i].proto == 0) {
                        FD_SET(socketTcpForUdp[i], &fdReadSet);
                    }
                } else if (socketConfigList[i].mode == 2) {
                    FD_SET(socketHandles[i], &fdReadSet);
                    // specially for server response udp, also add it to fdReadSet
//                    if (socketConfigList[i].proto == 0) {
                    FD_SET(socketTcpForUdp[i], &fdReadSet);
//                    }
                }

                if (socketHandles[i] > topActiveSocket) {
                    topActiveSocket = socketHandles[i];
                }
            }
        }
        // block on select()
        int ret;
        //double min_timeout_val = 1.0;
        struct timeval timeout_select;
//        timeout_select.tv_sec = 1;
//        timeout_select.tv_usec =  timeout_select.tv_sec * 1000000;
        timeout_select.tv_usec = 1 * 1000000;
//        if ((ret = select(topActiveSocket + 1, &fdReadSet, &fdWriteSet, NULL, &timeout_select)) <= 0) {
        if ((ret = select(topActiveSocket + 1, &fdReadSet, &fdWriteSet, NULL, &timeout_select)) <= 0) {
//            printf("\nselect() block timeout with select return value %d\n", ret);
            FD_ZERO(&fdReadSet);
            FD_ZERO(&fdWriteSet);
//            printf("timeout\n");
//            continue;
//            return;
        }
//        printf("skip the select\n");
        // process the active sockets
        for (int i = 0; i < maxSockets; i++) {
            if (!socketValid[i]) continue; // only check for the valid sockets
            if (FD_ISSET(socketHandles[i], &fdReadSet)) { // if the socket [i] is active for read
                if (i == 0) { // read the sys packet and first check the socket for accept()
                    // printf("need accept\n");
                    int new_socket = accept(server_listen_socket, (struct sockaddr *) &address,
                                            (socklen_t * ) & addrlen);
                    // printf("accept one\n");
                    if (new_socket < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    // find a free entry in the socketHandles[] and parse the sys packet
                    int j = 1;
                    for (; j < maxSockets; j++) {
                        if (!socketValid[j]) {
                            char sysPacketBuf[sys_packet_size + 1];
                            memset(sysPacketBuf, '\0', sys_packet_size + 1);
                            int len = sys_packet_size;
                            int num_read = read(new_socket, sysPacketBuf, len);
                            Sys_packet sysPacket;
                            parse_sys_packet(sysPacketBuf, &sysPacket);
                            // set socket config list
                            socketConfigList[j] = sysPacket;
                            //  out_packet(sysPacket);
                            // set statisticsList
                            init_statistics(&statisticsList[j]);
                            statisticsList[j].pktrate = sysPacket.pktrate;
                            statisticsList[j].left_pktnum = sysPacket.pktnum;
                            statisticsList[j].pktnum = sysPacket.pktnum;
                            statisticsList[j].previous_clock = clock();
                            statisticsList[j].pktsize = sysPacket.pktsize;
                            if (sysPacket.mode == 0 && sysPacket.proto == 0) { // client send udp
                                //  server should send a free port to client for sending udp
                                int free_port = get_free_port();
                                char free_port_buf[6];
                                memset(free_port_buf, '\0', 6);
                                std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
                                strcpy(free_port_buf, port_str.c_str());
                                write(new_socket, free_port_buf, 5);
                                socketConfigList[j].client_port = free_port;
                                printf("[%d] [Connected] connect to %s port %d client send using udp via server port %d %d Bps\n",
                                       j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
                                int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
                                struct sockaddr_in new_udp_socket_server_address;
                                memset(&new_udp_socket_server_address, 0, sizeof(struct sockaddr_in));
                                new_udp_socket_server_address.sin_family = AF_INET;
                                new_udp_socket_server_address.sin_port = htons(free_port);
                                new_udp_socket_server_address.sin_addr.s_addr = inet_addr(ip);
                                int res = bind(new_udp_socket, (struct sockaddr *) &new_udp_socket_server_address,
                                               sizeof(new_udp_socket_server_address));
                                if (res == -1) printf("bind error %d\n", res);
                                socketTcpForUdp[j] = new_socket;
                                new_socket = new_udp_socket;
                            } else if (sysPacket.mode == 0 && sysPacket.proto == 1) { // client send tcp
                                new_socket = new_socket;// no change
                                socketConfigList[j].client_port = address.sin_port;
                                printf("[%d] [Connected] connect to %s port %d client send using tcp %d Bps\n", j,
                                       inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                            } else if (sysPacket.mode == 1 && sysPacket.proto == 1) { // server send tcp
                                printf("[%d] [Connected] connect to %s port %d server send using tcp %d Bps\n", j,
                                       inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                                new_socket = new_socket; // no change
                            } else if (sysPacket.mode == 1 && sysPacket.proto == 0) { // server send udp
//                                socketConfigList[j].client_port = sysPacket.client_port; // change port
//                                socketConfigList[j].client_ip = sysPacket.client_ip; // change ip
                                printf("[%d] [Connected] connect to %s port %d server send using udp %d Bps\n", j,
                                       inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                                int new_udp_socket = socket(AF_INET, SOCK_DGRAM,
                                                            0); // replace tcp socket with new udp socket
                                socketTcpForUdp[j] = new_socket;
                                new_socket = new_udp_socket;
                            } else if (sysPacket.mode == 2) { // response
                                //  server should send a free port to client for sending response udp
                                int free_port = get_free_port();
                                char free_port_buf[6];
                                memset(free_port_buf, '\0', 6);
                                std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
                                strcpy(free_port_buf, port_str.c_str());
                                write(new_socket, free_port_buf, 5);
                                response_udp_server_binding_ports_list[j] = free_port;
                                socketTcpForUdp[j] = new_socket;
                                if (sysPacket.proto ==
                                    0) { // if response using udp, then just a normal udp socket binding the new port
                                    printf("[%d] [Connected] connect to %s port %d client response using udp via server port %d %d Bps\n",
                                           j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
                                    int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
                                    struct sockaddr_in new_udp_socket_server_address;
                                    memset(&new_udp_socket_server_address, 0, sizeof(struct sockaddr_in));
                                    new_udp_socket_server_address.sin_family = AF_INET;
                                    new_udp_socket_server_address.sin_port = htons(free_port);
                                    new_udp_socket_server_address.sin_addr.s_addr = htonl(INADDR_ANY);
                                    //  new_udp_socket_server_address.sin_addr.s_addr = inet_addr(ip);
                                    int res = bind(new_udp_socket, (struct sockaddr *) &new_udp_socket_server_address,
                                                   sizeof(new_udp_socket_server_address));
                                    if (res < 0) {
                                        perror("bind failed");
                                        exit(EXIT_FAILURE);
                                    }
                                    new_socket = new_udp_socket;
                                }
                                    // if response using tcp, then should new a tcp socket for listen, put this new tcp socket
                                else if (sysPacket.proto == 1) {
                                    printf("[%d] [Connected] connect to %s port %d client response using tcp via server port %d %d response/s\n",
                                           j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
                                    portServerForTcpResponse = free_port;
                                    int server_listen_response_socket;
                                    struct sockaddr_in response_address;
                                    // Creating socket file descriptor
                                    if ((server_listen_response_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                                        perror("socket failed");
                                        exit(EXIT_FAILURE);
                                    }
                                    response_address.sin_family = AF_INET;
                                    response_address.sin_addr.s_addr = htonl(INADDR_ANY);
                                    response_address.sin_port = htons(free_port);
                                    if (bind(server_listen_response_socket, (struct sockaddr *) &response_address,
                                             sizeof(response_address))
                                        < 0) {
                                        perror("bind failed");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (listen(server_listen_response_socket, 5) < 0) {
                                        perror("listen");
                                        exit(EXIT_FAILURE);
                                    }
                                    new_socket = server_listen_response_socket;
                                }
                            }
                            // out_packet(sysPacket);
                            socketValid[j] = true;
                            socketHandles[j] = new_socket;
                            numActiveSockets++;
                            if (numActiveSockets >= maxSockets) {
                                // ignore new accept()
                                socketValid[0] = false;
                            }
                            break;
                        }
                    }
                } else { // socket for recv()
                    if (socketConfigList[i].mode == 0 && socketConfigList[i].proto == 0) { // client send udp
                        int using_select = 1;
                        int res = udp_recv(socketHandles[i], statisticsList[i].pktsize,
                                           &statisticsList[i].previous_clock,
                                           statisticsList[i].inter_arrival_list, statisticsList[i].J_i_list,
                                           &statisticsList[i].cum_packet_number, &statisticsList[i].cum_time_cost,
                                           &statisticsList[i].cum_bytes_recv, statisticsList[i].stat,
                                           &statisticsList[i].cum_time_cost_session,
                                           &statisticsList[i].previous_SN, &statisticsList[i].total_packet_loss,
                                           using_select);
                        if (res == 1) {
                            socketValid[i] = false;
                            numActiveSockets--;
                            printf("[%d] [Stop] server recv stop packet from client ip %s port %d using udp\n", i,
                                   socketConfigList[i].client_ip, socketConfigList[i].client_port);
                            if (numActiveSockets == (maxSockets - 1)) {
                                socketValid[0] = true;
                            }
                            close(socketHandles[i]);
                            init_statistics(&statisticsList[i]);
                        }

                    }
                    if (socketConfigList[i].mode == 0 && socketConfigList[i].proto == 1) { // client send tcp
                        int using_select = 1;
                        int res = tcp_recv(socketHandles[i], statisticsList[i].pktsize,
                                           &statisticsList[i].previous_clock,
                                           statisticsList[i].inter_arrival_list, statisticsList[i].J_i_list,
                                           &statisticsList[i].cum_packet_number, &statisticsList[i].cum_time_cost,
                                           &statisticsList[i].cum_bytes_recv, statisticsList[i].stat,
                                           &statisticsList[i].cum_time_cost_session,
                                           using_select);
                        // if (res != 1) printf("[%d] recv one packet from socket %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                        if (res == 1) {
                            socketValid[i] = false;
                            numActiveSockets--;
                            printf("[%d] [Stop] server recv stop packet from client ip %s port %d using tcp\n", i,
                                   socketConfigList[i].client_ip, socketConfigList[i].client_port);
                            if (numActiveSockets == (maxSockets - 1)) {
                                socketValid[0] = true;
                            }
                            close(socketHandles[i]);
                            init_statistics(&statisticsList[i]);
                        }
                    }
                    if (socketConfigList[i].mode == 2 && socketConfigList[i].proto == 0) {// server send response udp
                        struct sockaddr_in response_socket_udp_address;
                        response_socket_udp_address.sin_family = AF_INET;
                        //printf("socket configList port is %d\n", socketConfigList[i].client_port);
                        response_socket_udp_address.sin_port = htons(socketConfigList[i].client_port);
                        // printf("should recv free port is %d\n", response_udp_server_binding_ports_list[i]);
                        response_socket_udp_address.sin_addr.s_addr = inet_addr(socketConfigList[i].client_ip);
                        //  printf("recv data, the server ip is %s port is %d\n",socketConfigList[i].client_ip,socketConfigList[i].client_port);
                        server_response_udp(socketHandles[i], response_socket_udp_address,
                                            response_udp_server_binding_ports_list[i]);
                        //printf("send a udp response !\n");
                    }
                    if (socketConfigList[i].mode == 2 && socketConfigList[i].proto == 1) {// server send response tcp
                        int new_tcp_response_socket = accept(socketHandles[i], (struct sockaddr *) &address,
                                                             (socklen_t * ) & addrlen);
                        int yes = 1;
                        int result = setsockopt(new_tcp_response_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes,
                                                sizeof(int));
                        if (result < 0) {
                            printf("set socket tcp no delay fail!\n");
                            return;
                        }
                        char *recvBuf = (char *) malloc(16 * sizeof(char));
                        memset(recvBuf, '\0', 16);
                        int num_read = read(new_tcp_response_socket, recvBuf, 15);
                        if (num_read != 15) {
                            printf("[%d] [Stopped] response to %s port %d client send using tcp\n", i,
                                   inet_ntoa(address.sin_addr), address.sin_port);

//                            printf("server receive response wrong! only %d\n", num_read);
                        }
                        write(new_tcp_response_socket, recvBuf, 15);
                        free(recvBuf);
                        close(new_tcp_response_socket);
                    }
                }
                if (--ret == 0) break; // all active sockets processed
            }
            // if the client will stop the udp sending, then the former tcp will be blocked
            if (FD_ISSET(socketTcpForUdp[i], &fdReadSet)) {
                socketValid[i] = false;
                numActiveSockets--;
                // printf("go to the socket for tcp\n");
                if (socketConfigList[i].mode == 0) {
                    printf("[%d] [Stop] client send data stop to %s port %d using udp\n", i,
                           socketConfigList[i].client_ip, socketConfigList[i].client_port);
                } else if (socketConfigList[i].mode == 1) {
                    printf("[%d] [Stop] server send data stop to %s port %d using udp\n", i,
                           socketConfigList[i].client_ip, socketConfigList[i].client_port);
                } else if (socketConfigList[i].mode == 2) {
                    printf("[%d] [Stop] server response data stop to %s port %d\n", i,
                           socketConfigList[i].client_ip, socketConfigList[i].client_port);
                    //close(socketHandles[i]);
                }

                if (numActiveSockets == (maxSockets - 1)) {
                    socketValid[0] = true;
                }
                close(socketHandles[i]);
                close(socketTcpForUdp[i]);
            }

        }
//         process all fd write
//        printf("complete the fw searching\n");
        for (int i = 1; i < maxSockets; i++) {
            if (!socketValid[i]) continue;
            if (FD_ISSET(socketHandles[i], &fdWriteSet)) {
//                printf("exist writing one\n");

                // server send udp
                if (socketConfigList[i].mode == 1 && socketConfigList[i].proto == 0) {
                    struct sockaddr_in new_udp_socket_client_address;
                    new_udp_socket_client_address.sin_family = AF_INET;
                    new_udp_socket_client_address.sin_port = htons(socketConfigList[i].client_port);
                    new_udp_socket_client_address.sin_addr.s_addr = inet_addr(socketConfigList[i].client_ip);
                    int using_select = 1;
//                    printf("target ip is %s\n", socketConfigList[i].client_ip);
//                    printf("target port is %d\n", socketConfigList[i].client_port);
                    int res = udp_send(socketHandles[i], &statisticsList[i].p_num_index, socketConfigList[i].pktsize,
                                       &socketConfigList[i].pktnum, socketConfigList[i].pktnum,
                                       socketConfigList[i].pktrate, 500, new_udp_socket_client_address, using_select);
//                    printf("after udp send the p_num_index is %d\n",statisticsList[i].p_num_index);
                    if (res == 1) {
                        socketValid[i] = false;
                        numActiveSockets--;
                        printf("[%d] [Stop] server send data stop to %s port %d using udp\n", i,
                               socketConfigList[i].client_ip, socketConfigList[i].client_port);
                        if (numActiveSockets == (maxSockets - 1)) {
                            socketValid[0] = true;
                        }
                        close(socketHandles[i]);
                    }
                }
                // server send tcp
                if (socketConfigList[i].mode == 1 && socketConfigList[i].proto == 1) {
                    //printf("go into the server send tcp section\n");
                    int using_select = 1;
                    int res = tcp_send(socketHandles[i], socketConfigList[i].pktsize, &socketConfigList[i].pktnum,
                                       socketConfigList[i].pktnum,
                                       socketConfigList[i].pktrate, 500, using_select);
                    //if (res != 1) printf("[%d] server send one packet from  %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                    //printf("res is %d", res);

                    if (res == 1) {
                        socketValid[i] = false;
                        numActiveSockets--;
                        printf("[%d] [Stop] server send data stop to %s port %d using tcp\n", i,
                               socketConfigList[i].client_ip, socketConfigList[i].client_port);
                        if (numActiveSockets == (maxSockets - 1)) {
                            socketValid[0] = true;
                        }
                        close(socketHandles[i]);
                    }
                }
                if (--ret == 0) break;
            }
        }
    }
    close(server_listen_socket);
}

// this function is to parse the receiver argument though argv
void argument_parse_server(int argc, char **argv, Server_argument *server_argument) {
    server_argument->lhost = (char *) "localhost";
    server_argument->lport = (char *) "4180";
    server_argument->rbufsize = 65536;
    server_argument->sbufsize = 65536;
    server_argument->serverModel = 0;
    server_argument->poolSize = 8;
    server_argument->socket = 0;
    server_argument->stat = 500;
    server_argument->httpPort = (char *) "4080";
    server_argument->httpsPort = (char *) "4081";

    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc && argv[i][0] == '-') {
            if (strcmp(argv[i], "-lhost") == 0) {
                server_argument->lhost = argv[i + 1];
                continue;
            } else if (strcmp(argv[i], "-stat") == 0) {
                server_argument->stat = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-lport") == 0) {
                server_argument->lport = argv[i + 1];
                continue;
            } else if (strcmp(argv[i], "-rbufsize") == 0) {
                server_argument->rbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-sbufsize") == 0) {
                server_argument->sbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-server") == 0) {
                if (strcmp(argv[i + 1], "threadpool") == 0) {
                    server_argument->serverModel = 0;
                } else if (strcmp(argv[i + 1], "thread") == 0) {
                    server_argument->serverModel = 1;
                }
                continue;
            } else if (strcmp(argv[i], "-poolsize") == 0) {
                server_argument->poolSize = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-lhttpport") == 0) {
                server_argument->httpPort = argv[i + 1];
                continue;
            } else if (strcmp(argv[i], "-lhttpsport") == 0) {
                server_argument->httpsPort = argv[i + 1];
                continue;
            }
        }
    }
}

// this function is for print the prompt info (using -h)
void print_prompt_information_server() {
    printf(" welcome to use the NetProbe\n");
    printf("-stat xxx set update of statistics display to be once yyy ms. (Default = 0 = no stat display)");
    printf("-lhost hostname” hostname to bind to. (Default late binding, i.e., IN_ADDR_ANY)\n");
    printf("-lhttpport portnum” port number to bind for http connection (Default “4080”)\n");
    printf("-lhttpsport portnum” port number to bind for https connection (Default “4081”)\n");
    printf("-server [threadpool|thread]” set the concurrent server model to either threadpool or signal thread\n");
    printf("-poolsize psize” set the initial thread pool size (default 8 threads), valid for thread-pool server model only\n");
}

// this function is to print the sys packet
void out_packet(Sys_packet sysPacket) {
    printf("this is out\n");
    printf("mode is %d\n", sysPacket.mode);
    printf("proto is %d\n", sysPacket.proto);
    printf("pktsize is %d\n", sysPacket.pktsize);
    printf("pktnum is %d\n", sysPacket.pktnum);
    printf("pktrate is %d\n", sysPacket.pktrate);
    printf("port is %d\n", sysPacket.client_port);
    printf("client ip is %s\n", sysPacket.client_ip);
}

// this function is to init the statistics
void init_statistics(Statistics *statistics) {
    statistics->stat = 0;
    statistics->pktsize = 0;
    statistics->pktnum = 0;
    statistics->left_pktnum = 0;
    statistics->pktrate = 0;
    statistics->J_i_list = (double *) malloc(Jitter_max * sizeof(double));
    memset(statistics->J_i_list, 0, Jitter_max * sizeof(double));
    statistics->inter_arrival_list = (double *) malloc(Jitter_max * sizeof(double));
    memset(statistics->inter_arrival_list, 0, Jitter_max * sizeof(double));
    statistics->cum_packet_number = 0;
    statistics->cum_time_cost = 0;
    statistics->cum_bytes_recv = 0;
    statistics->cum_time_cost_session = 0;
    statistics->previous_clock = clock();
    statistics->previous_SN = 0;
    statistics->total_packet_loss = 0;
    statistics->p_num_index = 0;
}

// this function si to do the multiThreadServer
void multiThreadServer(Server_argument server_argument) {
    ThreadPool *pool = NULL;
    if (server_argument.poolSize >= 1) {
        pool = threadPollCreate(server_argument.poolSize, 128, 100);

    } else {
        pool = threadPollCreate(2, 128, 100);
    }

    int server_listen_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char ip[100];
    solve_hostname_to_ip_linux(server_argument.lhost, ip);
    printf("%s listening to %s\n", server_argument.lhost, ip);
    // Creating socket file descriptor
    if ((server_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    SetSendBufferSizeLinux(server_argument.sbufsize, server_listen_socket);
    SetReceiveBufferSizeLinux(server_argument.rbufsize, server_listen_socket);
    address.sin_family = AF_INET;
    if (strcmp(ip, "127.0.0.1") == 0) {
        address.sin_addr.s_addr = INADDR_ANY;
    } else address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(atoi(server_argument.lport));

    // Forcefully attaching socket to the port
    if (bind(server_listen_socket, (struct sockaddr *) &address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);

    }
//    printf("begin alive number is %d\n", threadPoolAliveNumber(pool));
//    threadPoolAdd(pool, fun, NULL);
//    threadPoolAdd(pool, fun, NULL);
//    threadPoolAdd(pool, fun, NULL);
//    printf("%d\n", pool->queueSize);
//    printf("after add alive number is %d\n", threadPoolAliveNumber(pool));
//    sleep(5);
//    printf("after add alive number is %d\n", threadPoolAliveNumber(pool));
//    sleep(20);

    char *realIp = (char *) malloc(20 * sizeof(char));
    memset(realIp, '\0', 20 * sizeof(char));
    get_ip(realIp);
    printf("server listen to real ip : %s the network port %d (host port %s)\n", realIp, (address.sin_port),
           server_argument.lport);
//    printf("go into listen %d\n",listen(server_listen_socket, 5));
    if (listen(server_listen_socket, 5) < 0) {
        perror("listen");
//        printf("go into listen\n");
        exit(EXIT_FAILURE);
    }
    free(realIp);
//    printf("13begin alive number is %d\n", threadPoolAliveNumber(pool));
    fd_set fdReadSet;
    FD_ZERO(&fdReadSet);
    clock_t current_clock;
    clock_t previous_clock = clock();
    double cum_time_cost = 0;
    double cum_time_cost_session = 0;

    while (true) {
//        printf("go to while\n");
//        printf("\nout tcp alive num is %d\n", getTcpNum(pool));
        // set up the fd_set
        int topActiveSocket = 0;
        FD_ZERO(&fdReadSet);
        FD_SET(server_listen_socket, &fdReadSet);
        // block on select()
        int ret = 0;

//
        struct timeval timeout_select;
//        timeout_select.tv_usec =  timeout_select.tv_sec * 1000000;
//        printf("stat is %d\n",server_argument.stat);
//        timeout_select.tv_usec = server_argument.stat * 1000;
        timeout_select.tv_sec = 0;
        timeout_select.tv_usec = server_argument.stat * 1000;
        if ((ret = select(server_listen_socket + 1, &fdReadSet, NULL, NULL, &timeout_select)) <= 0) {
//            printf("time out\n");
            FD_ZERO(&fdReadSet);
        }
        // process the active sockets
        // read the sys packet and first check the socket for accept()
        if (ret > 0) {
            int new_socket = accept(server_listen_socket, (struct sockaddr *) &address, (socklen_t * ) & addrlen);
//            printf("accept one\n");
            if (new_socket < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            char sysPacketBuf[sys_packet_size + 1];
            memset(sysPacketBuf, '\0', sys_packet_size + 1);
            int len = sys_packet_size;
            int num_read = read(new_socket, sysPacketBuf, len);
            Sys_packet sysPacket;
            parse_sys_packet(sysPacketBuf, &sysPacket);
//            char * message = generate_message(1000, -1);
//            while(true) {
//                int ret = send(new_socket, message, 1000, 0);
//                printf("have send %d\n", ret);
//                sleep(1);
//            }
            if (sysPacket.mode == 0 && sysPacket.proto == 1) { // client send tcp
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                funArg->pool = pool;
                threadPoolAdd(pool, client_send_tcp, funArg);
            }
            if (sysPacket.mode == 1 && sysPacket.proto == 1) { // server send tcp
                // printf("server send\n");
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                char *message = generate_message(1000, -1);
                funArg->pool = pool;
                threadPoolAdd(pool, server_send_tcp, funArg);
            }
            if (sysPacket.mode == 0 && sysPacket.proto == 0) { // client send udp
                // printf("server send\n");
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                funArg->pool = pool;
                threadPoolAdd(pool, client_send_udp, funArg);
            }
            if (sysPacket.mode == 1 && sysPacket.proto == 0) { // server send udp
                // printf("server send\n");
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                funArg->pool = pool;
                threadPoolAdd(pool, server_send_udp, funArg);
            }
            if (sysPacket.mode == 2 && sysPacket.proto == 1) { // server response tcp
                // printf("server send\n");
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                funArg->pool = pool;
                threadPoolAdd(pool, server_response_tcp, funArg);
            }
            if (sysPacket.mode == 2 && sysPacket.proto == 0) { // server response udp
                // printf("server send\n");
                multiThreadFunArg *funArg = (multiThreadFunArg *) malloc(sizeof(multiThreadFunArg));
                memset(funArg, 0, sizeof(multiThreadFunArg));
                funArg->server_argument = server_argument;
                funArg->sys_packet = sysPacket;
                funArg->socket = new_socket;
                funArg->pool = pool;
                threadPoolAdd(pool, server_response_udp_multi_thread, funArg);
            }


/*
//
//        if (sysPacket.mode == 0 && sysPacket.proto == 0) { // client send udp

//
//        }
//        else if (sysPacket.mode == 0 && sysPacket.proto == 1) { // client send tcp
//            new_socket = new_socket;// no change
//            socketConfigList[j].client_port = address.sin_port;
//            printf("[%d] [Connected] connect to %s port %d client send using tcp %d Bps\n", j,
//                   inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
//        }
//        else if (sysPacket.mode == 1 && sysPacket.proto == 1) { // server send tcp
//            printf("[%d] [Connected] connect to %s port %d server send using tcp %d Bps\n", j,
//                   inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
//            new_socket = new_socket; // no change
//        }
//        else if (sysPacket.mode == 1 && sysPacket.proto == 0) { // server send udp
//          socketConfigList[j].client_port = sysPacket.client_port; // change port
//          socketConfigList[j].client_ip = sysPacket.client_ip; // change ip
//            printf("[%d] [Connected] connect to %s port %d server send using udp %d Bps\n", j,
//                   inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
//            int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0); // replace tcp socket with new udp socket
//            socketTcpForUdp[j] = new_socket;
//            new_socket = new_udp_socket;
//        }
//        else if (sysPacket.mode == 2) { // response
//            //  server should send a free port to client for sending response udp
//            int free_port = get_free_port();
//            char free_port_buf[6];
//            memset(free_port_buf, '\0', 6);
//            std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
//            strcpy(free_port_buf, port_str.c_str());
//            write(new_socket, free_port_buf, 5);
//            socketConfigList[j].client_port = free_port;
//            socketTcpForUdp[j] = new_socket;
//            if (sysPacket.proto ==
//                0) { // if response using udp, then just a normal udp socket binding the new port
//                printf("[%d] [Connected] connect to %s port %d client response using udp via server port %d %d Bps\n",
//                       j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
//                int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
//                struct sockaddr_in new_udp_socket_server_address;
//                memset(&new_udp_socket_server_address, 0, sizeof(struct sockaddr_in));
//                new_udp_socket_server_address.sin_family = AF_INET;
//                new_udp_socket_server_address.sin_port = htons(free_port);
//                new_udp_socket_server_address.sin_addr.s_addr = inet_addr(ip);
//                int res = bind(new_udp_socket, (struct sockaddr *) &new_udp_socket_server_address,
//                               sizeof(new_udp_socket_server_address));
//                if (res == -1) printf("bind error %d\n", res);
//            }
//                // if response using tcp, then should new a tcp socket for listen, put this new tcp socket
//            else if (sysPacket.proto == 1) {
//                printf("[%d] [Connected] connect to %s port %d client response using tcp via server port %d %d response/s\n",
//                       j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
//                portServerForTcpResponse = free_port;
//                int server_listen_response_socket;
//                struct sockaddr_in response_address;
//                // Creating socket file descriptor
//                if ((server_listen_response_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//                    perror("socket failed");
//                    exit(EXIT_FAILURE);
//                }
//                response_address.sin_family = AF_INET;
//                response_address.sin_addr.s_addr = address.sin_addr.s_addr;
//                response_address.sin_port = htons(free_port);
//                if (bind(server_listen_response_socket, (struct sockaddr *) &response_address,
//                         sizeof(response_address))
//                    < 0) {
//                    perror("bind failed");
//                    exit(EXIT_FAILURE);
//                }
//                if (listen(server_listen_response_socket, 5) < 0) {
//                    perror("listen");
//                    exit(EXIT_FAILURE);
//                }
//                new_socket = server_listen_response_socket;
//            }
//        }
//        // out_packet(sysPacket);
//        socketValid[j] = true;
//        socketHandles[j] = new_socket;
//        numActiveSockets++;
//        if (numActiveSockets >= maxSockets) {
//            // ignore new accept()
//            socketValid[0] = false;
//        }
        break;
    }
    */
        }
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) previous_clock) / CLOCKS_PER_SEC;
        previous_clock = current_clock;
        cum_time_cost = cum_time_cost + time_cost;
        cum_time_cost_session = cum_time_cost_session + time_cost;
        if (cum_time_cost * 1000 >= (double) server_argument.stat) {
            printf("Elapsed [%.2fms] ThreadPool [%d|%d] TCP Clients [%d] UDP Clients [%d]\n",
                   cum_time_cost_session * 1000, threadPoolBusyNumber(pool), threadPoolAliveNumber(pool),
                   getTcpNum(pool), getUdpNum(pool));
            cum_time_cost_session = 0;
        }
    }
}

void client_send_tcp(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int socket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    //out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addTcpNum(pool);
    int cum_packet_number = 0, byte_recieve = 0, total_packet_number = 0;
    char *peer_data = (char *) malloc(MAX_BUFFER_RECV * sizeof(char));
    bool flag_exit = 0;

    while (!flag_exit) {
        while (byte_recieve < sysPacket.pktsize) {
            int ret = recv(socket, peer_data, sysPacket.pktsize - byte_recieve, 0);
            if ((ret < 0)) {
                printf("Recv failed with error code : %d\n", ret);
                //  Sleep(3000);
                break;
            } else {
                if (ret == 0) {
                    flag_exit = true; // peer close
                    break;
                } else {
                    byte_recieve = byte_recieve + ret;
                }
            }
        }
        byte_recieve = 0;
    }
    free(peer_data);
    deleteTcpNum(pool);
    close(socket);
//    printf("thread %ld is working, number = %d\n", pthread_self(), num);
}

void server_send_tcp(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int socket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    //out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addTcpNum(pool);
    int message_length = sysPacket.pktsize;
    printf("message length is %d\n", message_length);
    char *message = generate_message(message_length, -1);
    int p_num_index = 0;
    clock_t current_clock;
    clock_t previous_clock = clock();
    double cum_time_cost = 0;
    double cum_time_cost_session = 0;
    double cum_bytes_sent = 0;
    int cum_packet_number = 0;
    double single_iter_pkt_threshold = sysPacket.pktrate * (500.0 / 1000) * 100000;

    while ((p_num_index < sysPacket.pktnum) || sysPacket.pktnum == 0) {
        if ((cum_bytes_sent < single_iter_pkt_threshold) || (single_iter_pkt_threshold == 0)) { // in Mbps
            int bytes_sent = 0;
            while (bytes_sent < message_length) {
                int ret = send(socket, message + bytes_sent, message_length - bytes_sent, 0);
                if (ret > 0) {
                    bytes_sent = bytes_sent + ret;
                    // printf("send to %d\n",ret);
                } else {
                    printf("Send function fail, with error : %d\n", ret);
                    free(message);
                    deleteTcpNum(pool);
                    close(socket);
                    return;
                }
            }
            //  printf("have send a packet\n");
            p_num_index++;
            cum_packet_number++;
            cum_bytes_sent = cum_bytes_sent + bytes_sent;
        }
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) previous_clock) / CLOCKS_PER_SEC;
        previous_clock = current_clock;
        cum_time_cost = cum_time_cost + time_cost;
        cum_time_cost_session = cum_time_cost_session + time_cost;
        if (cum_time_cost * 1000 >= 500.0) {
            // double throughput = (cum_bytes_sent * 8) / (cum_time_cost * 1000000);
            //  printf("send %.2fms\n", cum_time_cost_session* 1000);
//            printf("Sender: [Elapsed] %.2f ms, [Pkts] %d, [Rate] %.2f Mbps\n", cum_time_cost_session * 1000, cum_packet_number, throughput);
            cum_packet_number = 0;
            cum_time_cost = 0;
            cum_bytes_sent = 0;
        }
    }
    free(message);
    deleteTcpNum(pool);
    close(socket);
    return;
}

void client_send_udp(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int tcpSocket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addUdpNum(pool);
    addTcpNum(pool);
    //  server should send a free port to client for sending udp
    int free_port = get_free_port();
    char free_port_buf[6];
    memset(free_port_buf, '\0', 6);
    std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
    strcpy(free_port_buf, port_str.c_str());
    write(tcpSocket, free_port_buf, 5);
    int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in new_udp_socket_server_address;
    memset(&new_udp_socket_server_address, 0, sizeof(struct sockaddr_in));
    new_udp_socket_server_address.sin_family = AF_INET;
    new_udp_socket_server_address.sin_port = htons(free_port);
    new_udp_socket_server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    int res = bind(new_udp_socket, (struct sockaddr *) &new_udp_socket_server_address,
                   sizeof(new_udp_socket_server_address));
    if (res == -1) printf("bind error %d\n", res);
    fd_set fdReadSet;
    FD_ZERO(&fdReadSet);
    FD_SET(tcpSocket, &fdReadSet);
    FD_SET(new_udp_socket, &fdReadSet);
    int max = new_udp_socket > tcpSocket ? new_udp_socket : tcpSocket;
    int ret = 0;
    char *buf = (char *) malloc(MAX_BUFFER_RECV * sizeof(char));
    socklen_t len = sizeof(new_udp_socket_server_address);
    while (true) {
        FD_ZERO(&fdReadSet);
        FD_SET(tcpSocket, &fdReadSet);
        FD_SET(new_udp_socket, &fdReadSet);

        if ((ret = select(max + 1, &fdReadSet, NULL, NULL, NULL)) <= 0) {
            if (ret == 0) {}
            printf("time out\n");
            printf("select error %d\n", ret);
        }
        if (FD_ISSET(tcpSocket, &fdReadSet)) {
            break;
        }
        if (FD_ISSET(new_udp_socket, &fdReadSet)) {
            int recv_size = recvfrom(new_udp_socket, buf, sysPacket.pktsize, 0,
                                     (struct sockaddr *) &new_udp_socket_server_address, &len);
            if (recv_size < 0) {
                printf("Recv failed with error code : %d\n", recv_size);
                break;
            } else {
                // printf("recv packet size is %d\n", recv_size);
            }
        }
    }
    deleteTcpNum(pool);
    deleteUdpNum(pool);
    close(tcpSocket);
    close(new_udp_socket);
    free(buf);
}

void server_send_udp(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int tcpSocket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    //out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addTcpNum(pool);
    addUdpNum(pool);
    int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in new_udp_socket_client_address;
    memset(&new_udp_socket_client_address, 0, sizeof(struct sockaddr_in));
    new_udp_socket_client_address.sin_family = AF_INET;
    new_udp_socket_client_address.sin_port = htons(sysPacket.client_port);
    new_udp_socket_client_address.sin_addr.s_addr = inet_addr(sysPacket.client_ip);
    int max = new_udp_socket > tcpSocket ? new_udp_socket : tcpSocket;
    fd_set fdReadSet;
    fd_set fdWriteSet;
    int ret = 0;
    int p_num_index = 0;
//    double temp = ((double)sysPacket.pktsize / sysPacket.pktrate) * 10;
    while (true) {
        FD_ZERO(&fdReadSet);
        FD_SET(tcpSocket, &fdReadSet);
        FD_SET(new_udp_socket, &fdWriteSet);
        if ((ret = select(max + 1, &fdReadSet, &fdWriteSet, NULL, NULL)) <= 0) {
            printf("select error %d\n", ret);
        }
        if (FD_ISSET(tcpSocket, &fdReadSet)) {
            break;
        }
        if (FD_ISSET(new_udp_socket, &fdWriteSet)) {
//            usleep(temp );
            if (p_num_index > sysPacket.pktnum && sysPacket.pktnum != 0) {
                break;
            }
            char *message = generate_message(sysPacket.pktsize, p_num_index);
            int recv_size = sendto(new_udp_socket, message, sysPacket.pktsize, 0,
                                   (struct sockaddr *) &new_udp_socket_client_address,
                                   sizeof(new_udp_socket_client_address));
            if (recv_size < 0) {
                printf("Recv failed with error code : %d\n", recv_size);
                break;
            } else {
//                printf("send one udp\n");
            }
            p_num_index++;
        }
    }
    close(new_udp_socket);
    close(tcpSocket);
    deleteTcpNum(pool);
    deleteUdpNum(pool);

}

void server_response_tcp(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int sysTcpSocket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    //out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addTcpNum(pool);
    int free_port = get_free_port();
    char free_port_buf[6];
    memset(free_port_buf, '\0', 6);
    std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
    strcpy(free_port_buf, port_str.c_str());
    write(sysTcpSocket, free_port_buf, 5);
    fd_set fdReadSet;
    int ret = 0;
    int server_listen_response_socket;
    struct sockaddr_in response_address;
    // Creating socket file descriptor
    if ((server_listen_response_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    response_address.sin_family = AF_INET;
    response_address.sin_addr.s_addr = INADDR_ANY;
    response_address.sin_port = htons(free_port);
    int addrlen = sizeof(response_address);
    if (bind(server_listen_response_socket, (struct sockaddr *) &response_address,
             sizeof(response_address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_listen_response_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (true) {
        FD_ZERO(&fdReadSet);
        FD_SET(sysTcpSocket, &fdReadSet);
        FD_SET(server_listen_response_socket, &fdReadSet);
        int max = sysTcpSocket > server_listen_response_socket ? sysTcpSocket : server_listen_response_socket;
        if ((ret = select(max + 1, &fdReadSet, NULL, NULL, NULL)) < 0) {
            printf("select error %d\n", ret);
        }
        if (FD_ISSET(sysTcpSocket, &fdReadSet)) {
            break;
        }
        if (FD_ISSET(server_listen_response_socket, &fdReadSet)) {
            int new_tcp_response_socket = accept(server_listen_response_socket, (struct sockaddr *) &response_address,
                                                 (socklen_t * ) & addrlen);
            addTcpNum(pool);
            int yes = 1;
            int result = setsockopt(new_tcp_response_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes,
                                    sizeof(int));
            if (result < 0) {
                printf("set socket tcp no delay fail!\n");
                return;
            }
            char *recvBuf = (char *) malloc(16 * sizeof(char));
            memset(recvBuf, '\0', 16);
            int num_read = read(new_tcp_response_socket, recvBuf, 15);
            if (num_read != 15) {
//                printf("[%d] [Stopped] response to %s port %d client send using tcp\n", i,
//                       inet_ntoa(address.sin_addr), address.sin_port);
                printf("server receive response wrong! only %d\n", num_read);
            }
            write(new_tcp_response_socket, recvBuf, 15);
            free(recvBuf);
            close(new_tcp_response_socket);
            deleteTcpNum(pool);
        }
    }
    close(sysTcpSocket);
    deleteTcpNum(pool);

}

void server_response_udp_multi_thread(void *arg) {
    multiThreadFunArg funArg = *(multiThreadFunArg *) arg;
    int sysTcpSocket = funArg.socket;
    sys_packet sysPacket = funArg.sys_packet;
    //out_packet(sysPacket);
    ThreadPool *pool = funArg.pool;
    addTcpNum(pool);
    addUdpNum(pool);
    int free_port = get_free_port();
    char free_port_buf[6];
    memset(free_port_buf, '\0', 6);
    std::string port_str = add_zero(my_int_to_string(free_port), 5, '0');
    strcpy(free_port_buf, port_str.c_str());
    write(sysTcpSocket, free_port_buf, 5);
    int my_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_address, client_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    memset(&client_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(free_port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address.sin_family = AF_INET;;
    client_address.sin_port = htons(sysPacket.client_port);
    client_address.sin_addr.s_addr = inet_addr(sysPacket.client_ip);

    int bind_res = bind(my_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (bind_res == -1) {
        printf("bind error\n");
        deleteTcpNum(pool);
        deleteUdpNum(pool);
        close(sysTcpSocket);
        close(my_socket);
        return;
    }
    int ret = 0;
    int max = sysTcpSocket > my_socket ? sysTcpSocket : my_socket;
    fd_set fdReadSet;
    while (true) {
        FD_ZERO(&fdReadSet);
        FD_SET(sysTcpSocket, &fdReadSet);
        FD_SET(my_socket, &fdReadSet);
        if ((ret = select(max + 1, &fdReadSet, NULL, NULL, NULL)) <= 0) {
            printf("select error %d\n", ret);
        }
        if (FD_ISSET(sysTcpSocket, &fdReadSet)) {
            break;
        }
        if (FD_ISSET(my_socket, &fdReadSet)) {
            char *buf = (char *) malloc(16 * sizeof(char));
            memset(buf, '\0', 16 * sizeof(char));
            int recv_size = 0;
            socklen_t len = sizeof(server_address);
            recv_size = recvfrom(my_socket, buf, 15, 0, (struct sockaddr *) &server_address, &len);
            // printf("recv response is  %s\n",buf);
            if (recv_size <= 0) {
                return;
            }
            int sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
            int ret = sendto(sendSocket, buf, 15, 0, (struct sockaddr *) &client_address, sizeof(client_address));
            if (ret != -1) {
                // printf("server send response udp %d\n", ret);
                //  sleep(20);
                free(buf);
            } else {
                printf("Sendto fail, with error : %d\n", ret);
                // sleep(10);
                free(buf);
            }
        }
    }
    deleteTcpNum(pool);
    deleteUdpNum(pool);
    close(sysTcpSocket);
    close(my_socket);
}

void fun(void *argv) {
    printf("begin my function\n");
    printf("thread %ld is working\n", pthread_self());
    sleep(30);

}

void serverProject4(int argc, char **argv) {
    Server_argument server_argument;
    argument_parse_server(argc, argv, &server_argument);

    printf("the following content is the server arguments\n\n");
    printf("stat is %d\n", server_argument.stat);
    printf("lhost is %s\n", server_argument.lhost);
    printf("http port is %s\n", server_argument.httpPort);;
    printf("https port is %s\n", server_argument.httpsPort);;
    if (server_argument.serverModel == 0) {
        printf("server model is using thread pool\n");
        printf("poolsize is %d\n", server_argument.poolSize);
    } else if (server_argument.serverModel == 1) {
        printf("server model is using single thread\n");
    }
    int socket_https_listen = 0, socket_http_listen = 0;
    socket_https_listen = create_socket(atoi(server_argument.httpsPort));
    socket_http_listen = create_socket(atoi(server_argument.httpPort));
    if (socket_https_listen != 0 && socket_http_listen != 0) {
        printf("Create a socket successfully\n");
    }
    int max_listen_socket = socket_https_listen > socket_http_listen ? socket_https_listen : socket_http_listen;
    ThreadPool *pool = NULL;
    if (server_argument.poolSize >= 1) {
        pool = threadPollCreate(server_argument.poolSize, 128, 100);

    } else {
        pool = threadPollCreate(2, 128, 100);
    }
    fd_set fdReadSet;
    FD_ZERO(&fdReadSet);
    clock_t current_clock;
    clock_t previous_clock = clock();
    double cum_time_cost = 0;
    double cum_time_cost_session = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int httpClientNum = 0;
    int httpsClientNum = 0;
    while (true) {
//        printf("go to while\n");
        // set up the fd_set
        FD_ZERO(&fdReadSet);
        FD_SET(socket_https_listen, &fdReadSet);
        FD_SET(socket_http_listen, &fdReadSet);
        int ret = 0;
        struct timeval timeout_select;
        timeout_select.tv_sec = 0;
        timeout_select.tv_usec = server_argument.stat * 1000;
        if ((ret = select(max_listen_socket + 1, &fdReadSet, NULL, NULL, &timeout_select)) <= 0) {
//            printf("time out\n");
            FD_ZERO(&fdReadSet);
        }
        // process the active sockets
        // read the sys packet and first check the socket for accept()
        if (ret > 0) {
            if (FD_ISSET(socket_https_listen, &fdReadSet)) {
                int new_https_socket = accept(socket_https_listen, (struct sockaddr *) &address,
                                              (socklen_t * ) & addrlen);
                int *funArg = (int *) malloc(sizeof(int));
                memset(funArg, 0, sizeof(int));
                *funArg = new_https_socket;
                threadPoolAdd(pool, https_server, funArg);
                httpsClientNum++;
            }
            if (FD_ISSET(socket_http_listen, &fdReadSet)) {
                int new_http_socket = accept(socket_http_listen, (struct sockaddr *) &address,
                                             (socklen_t * ) & addrlen);
                int *funArg = (int *) malloc(sizeof(int));
                memset(funArg, 0, sizeof(int));
                *funArg = new_http_socket;
                threadPoolAdd(pool, http_server, funArg);
                httpClientNum++;
            }

        }
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) previous_clock) / CLOCKS_PER_SEC;
        previous_clock = current_clock;
        cum_time_cost = cum_time_cost + time_cost;
        cum_time_cost_session = cum_time_cost_session + time_cost;
        if (cum_time_cost * 1000 >= (double) server_argument.stat) {
            printf("Elapsed [%.2fms]  HTTP Clients [%d] HTTPS Clients [%d]\n",
                   cum_time_cost_session * 1000,
                   httpClientNum, httpsClientNum);
            cum_time_cost_session = 0;
            httpClientNum = 0;
            httpsClientNum = 0;
        }
    }
//    https_server(server_argument);
}

void https_server(void *arg) {
    char success[44];
    sprintf(success,
            "HTTP/1.1 200 OK\r\n"
            "Server: CUHK IERG4180 \r\n\r\n");
    char notFound[55];
    sprintf(notFound,
            "HTTP/1.1 404 Not Found\r\n"
            "Server: CUHK IERG4180 \r\n\r\n");
    int haveReadFirstLine = 0;
    char fileName[50];
    int hasFile = 0;
    int fileEndIndex = 0;
    int fileStartIndex = 4;
    memset(fileName, 0, 50 * sizeof(char));
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_certificate_file(ctx, "domain.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    } else printf("loading domain.crt successfully!\n");
    if (SSL_CTX_use_PrivateKey_file(ctx, "domain.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    } else printf("loading domain.key successfully!\n");
    int socket = *(int *) arg;
//        struct sockaddr_in addr;
//        unsigned int len = sizeof(addr);
    SSL *ssl;
    const char reply[] = "test\n";
    printf("the accept socket is %d\n", socket);
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket);
    char buff[8192];
    int len = 0;
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        do {
            len = SSL_read(ssl, buff, sizeof(buff));
            if (len > 0 && haveReadFirstLine == 0) {
                haveReadFirstLine = 1;
                if (buff[4] == '/') {
                    hasFile = 1;
                    for (int i = 4; i < sizeof(buff); i++) {
                        if (buff[i + 1] == ' '){
                            fileEndIndex = i;
                            break;
                        }
                    }
                }
                strncpy(fileName, buff + fileStartIndex + 1, fileEndIndex - fileStartIndex);
            }
        }while (len > 0);

        if (hasFile == 0) {
            SSL_write(ssl, notFound, strlen(notFound));
        }
        else {
//            printf("file is %s\n", fileName);
            FILE* fp;
            if ((fp = fopen(fileName, "rt")) == NULL) {
                SSL_write(ssl, notFound, strlen(notFound));
            }
            else {
                FILE *f = fopen(fileName, "rb");
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
                char *string = (char *)malloc(fsize + 1);
                fread(string, fsize, 1, f);
                fclose(f);
                string[fsize] = 0;
                strcat(success,string);
                SSL_write(ssl, success, 43 + fsize);
            }
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socket);
    SSL_CTX_free(ctx);
}

void http_server(void *arg) {
    int socket = *(int *) arg;
    char success[44];
    sprintf(success,
            "HTTP/1.1 200 OK\r\n"
            "Server: CUHK IERG4180 \r\n\r\n");
    char notFound[55];
    sprintf(notFound,
           "HTTP/1.1 404 Not Found\r\n"
           "Server: CUHK IERG4180 \r\n\r\n");
    int len = 0;
    int ret = 0;
    struct timeval timeout_select;
    timeout_select.tv_sec = 0;
    timeout_select.tv_usec = 1000 * 1000;
    fd_set fdReadSet;
    FD_ZERO(&fdReadSet);
    FD_SET(socket, &fdReadSet);
    int haveReadFirstLine = 0;
    char fileName[50];
    int hasFile = 0;
    int fileEndIndex = 0;
    int fileStartIndex = 4;
    memset(fileName, 0, 50 * sizeof(char));
    do {
        if ((ret = select(socket + 1, &fdReadSet, NULL, NULL, &timeout_select)) <= 0) {
            FD_ZERO(&fdReadSet);
            break;
        }
        FD_SET(socket, &fdReadSet);
        char buff[2000];
        len = read(socket, buff, sizeof(buff));
        if (len > 0 && haveReadFirstLine == 0) {
            haveReadFirstLine = 1;
            if (buff[4] == '/') {
                hasFile = 1;
                for (int i = 4; i < sizeof(buff); i++) {
                    if (buff[i + 1] == ' '){
                        fileEndIndex = i;
                        break;
                    }
                }
            }
            strncpy(fileName, buff + fileStartIndex + 1, fileEndIndex - fileStartIndex);

        }
        if (len > 0) fwrite(buff, len, 1, stdout);
    } while (len > 0);

    if (hasFile == 0)
    send(socket, notFound, strlen(notFound), 0);
    else {
//        printf("file is %s\n", fileName);
        FILE* fp;
        if ((fp = fopen(fileName, "rt")) == NULL) {
            send(socket, notFound, strlen(notFound), 0);
        }
        else {
            FILE *f = fopen(fileName, "rb");
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
            char *string = (char *)malloc(fsize + 1);
            fread(string, fsize, 1, f);
            fclose(f);
            string[fsize] = 0;
            strcat(success,string);
            send(socket, success, 43 + fsize, 0);
        }
    }
    close(socket);

}

int create_socket(int port) {
    int s;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    printf("server port is %d\n", port);
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }
    return s;
}
