
//// \\Mac\Home\Documents\GitHub\IERG4180_Project1\NetProbe\x64\Debug
//// \\Mac\Home\Documents\GitHub\example

# include "server.h"

int main(int argc, char** argv) {
    print_prompt_information_server();
    server(argc, argv);
    return 0;
}

// this function is for receive
void server(int argc, char** argv) {
    Server_argument  server_argument;
    argument_parse_server(argc, argv, &server_argument);

    printf("the following content is the server arguments\n\n");
    printf("lhost is %s\n", server_argument.lhost);
    printf("lport is %s\n", server_argument.lport);;
    printf("sbufsize is %ld\n", server_argument.sbufsize);
    printf("rbufsize is %ld\n", server_argument.rbufsize);
    printf("\n\n\n");
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
    }
    else address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(atoi(server_argument.lport));

    // Forcefully attaching socket to the port
    if (bind(server_listen_socket, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);

    }
    char * realIp = (char*)malloc(20 * sizeof (char));
    memset(realIp, '\0', 20 * sizeof(char));
    get_ip(realIp);
    printf("server listen to real ip : %s the network port %d (host port %s)\n", realIp, (address.sin_port), server_argument.lport);
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
    int  numActiveSockets = 1;
    struct sockaddr_in client_address[maxSockets];
    for (int i = 1; i < maxSockets; i++) {
        socketValid[i] = false;
        socketConfigList[i].mode = 0; // default is recv mode
    }
    socketHandles[0] = server_listen_socket;
    socketValid[0] = true;
    fd_set fdReadSet;
    fd_set fdWriteSet;
    while(1) {
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
                }
                else if (socketConfigList[i].mode == 1) {
//                    printf("put it into fw %d\n", i);
                    FD_SET(socketHandles[i], &fdWriteSet);
                    // specially for server send udp , also add it to the fdReadset
                    if (socketConfigList[i].proto == 0) {
                        FD_SET(socketTcpForUdp[i], &fdReadSet);
                    }
                }
                if (socketHandles[i] > topActiveSocket) {
                    topActiveSocket = socketHandles[i];
                }
            }
        }
        // block on select()
        int ret;
        double min_timeout_val = 1.0;
        struct timeval  timeout_select;
        timeout_select.tv_sec = 2;
        timeout_select.tv_usec =  timeout_select.tv_sec * 1000000;
//        if ((ret = select(topActiveSocket + 1, &fdReadSet, &fdWriteSet, NULL, &timeout_select)) <= 0) {
        if ((ret = select(topActiveSocket + 1, &fdReadSet, &fdWriteSet, NULL, &timeout_select)) <= 0) {
//            printf("\nselect() block timeout with select return value %d\n", ret);
            FD_ZERO(&fdReadSet);
            FD_ZERO(&fdWriteSet);
//            continue;
//            return;
        }
//        printf("skip the select\n");
        // process the active sockets
        for (int i = 0; i < maxSockets; i++) {
            if (!socketValid[i]) continue; // only check for the valid sockets
            if (FD_ISSET(socketHandles[i], &fdReadSet)) { // if the socket [i] is active
                if (i == 0) { // read the sys packet and first check the socket for accept()
                   // printf("need accept\n");
                    int new_socket = accept(server_listen_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
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
                                std :: string port_str = add_zero(my_int_to_string(free_port), 5, '0');
                                strcpy(free_port_buf, port_str.c_str());
                                write(new_socket, free_port_buf, 5);
                                socketConfigList[j].client_port = free_port;
                                printf("[%d] [Connected] connect to %s port %d client send using udp via server port %d %d Bps\n", j, sysPacket.client_ip, address.sin_port, free_port, sysPacket.pktrate);
                                int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
                                struct sockaddr_in new_udp_socket_server_address;
                                memset(&new_udp_socket_server_address, 0, sizeof(struct sockaddr_in));
                                new_udp_socket_server_address.sin_family = AF_INET;
                                new_udp_socket_server_address.sin_port = htons(free_port);
                                new_udp_socket_server_address.sin_addr.s_addr = inet_addr(ip);
                                int res = bind(new_udp_socket, (struct sockaddr*)&new_udp_socket_server_address, sizeof(new_udp_socket_server_address));
                                if (res == -1) printf("bind error %d\n",res);
                                socketTcpForUdp[j] = new_socket;
                                new_socket = new_udp_socket;

                            }
                            else if (sysPacket.mode == 0 && sysPacket.proto == 1) { // client send tcp
                                new_socket = new_socket;// no change
                                socketConfigList[j].client_port = address.sin_port;
                                printf("[%d] [Connected] connect to %s port %d client send using tcp %d Bps\n", j, inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                            }
                            else if (sysPacket.mode == 1 && sysPacket.proto == 1) { // server send tcp
                                printf("[%d] [Connected] connect to %s port %d server send using tcp %d Bps\n", j, inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                                new_socket = new_socket; // no change
                            }
                            else if (sysPacket.mode == 1 && sysPacket.proto == 0)  { // server send udp
//                                socketConfigList[j].client_port = sysPacket.client_port; // change port
//                                socketConfigList[j].client_ip = sysPacket.client_ip; // change ip
                                printf("[%d] [Connected] connect to %s port %d server send using udp %d Bps\n", j, inet_ntoa(address.sin_addr), address.sin_port, sysPacket.pktrate);
                                int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0); // replace tcp socket with new udp socket
                                socketTcpForUdp[j] = new_socket;
                                new_socket = new_udp_socket;
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
                }
                else { // socket for recv()
                    if (socketConfigList[i].mode == 0 && socketConfigList[i].proto == 0) { // client send udp
                        int using_select = 1;
                        int res = udp_recv(socketHandles[i], statisticsList[i].pktsize, &statisticsList[i].previous_clock,
                                           statisticsList[i].inter_arrival_list, statisticsList[i].J_i_list,
                                           &statisticsList[i].cum_packet_number, &statisticsList[i].cum_time_cost,
                                           &statisticsList[i].cum_bytes_recv, statisticsList[i].stat, &statisticsList[i].cum_time_cost_session,
                                           &statisticsList[i].previous_SN, &statisticsList[i].total_packet_loss,using_select);
                        if (res == 1) {
                            socketValid[i] = false;
                            numActiveSockets--;
                            printf("[%d] [Stop] server recv stop packet from client ip %s port %d using udp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                            if (numActiveSockets == (maxSockets - 1)) {
                                socketValid[0] = true;
                            }
                            close(socketHandles[i]);
                            init_statistics(&statisticsList[i]);
                        }

                    }
                    if (socketConfigList[i].mode == 0 && socketConfigList[i].proto == 1) { // client send tcp
                        int using_select = 1;
                        int res = tcp_recv(socketHandles[i], statisticsList[i].pktsize, &statisticsList[i].previous_clock,
                                 statisticsList[i].inter_arrival_list, statisticsList[i].J_i_list,
                                 &statisticsList[i].cum_packet_number, &statisticsList[i].cum_time_cost,
                                 &statisticsList[i].cum_bytes_recv, statisticsList[i].stat, &statisticsList[i].cum_time_cost_session,
                                 using_select);
                       // if (res != 1) printf("[%d] recv one packet from socket %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                        if (res == 1) {
                            socketValid[i] = false;
                            numActiveSockets--;
                            printf("[%d] [Stop] server recv stop packet from client ip %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                            if (numActiveSockets == (maxSockets - 1)) {
                                socketValid[0] = true;
                            }
                            close(socketHandles[i]);
                            init_statistics(&statisticsList[i]);
                        }
                    }
                }
                if (--ret == 0) break; // all active sockets processed
            }
            // if the client will stop the udp sending, then the former tcp will be block
            else if (FD_ISSET(socketTcpForUdp[i], &fdReadSet)) {
                socketValid[i] = false;
                numActiveSockets--;
                if (socketConfigList[i].mode == 0) {
                    printf("[%d] [Stop] client send data stop to %s port %d using udp\n", i,
                           socketConfigList[i].client_ip, socketConfigList[i].client_port);
                }
                else if (socketConfigList[i].mode == 1) {
                    printf("[%d] [Stop] server send data stop to %s port %d using udp\n", i,
                           socketConfigList[i].client_ip, socketConfigList[i].client_port);
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
                    int res = udp_send(socketHandles[i], &statisticsList[i].p_num_index, socketConfigList[i].pktsize, &socketConfigList[i].pktnum, socketConfigList[i].pktnum,socketConfigList[i].pktrate, 500,new_udp_socket_client_address, using_select);
//                    printf("after udp send the p_num_index is %d\n",statisticsList[i].p_num_index);
                    if (res == 1){
                            socketValid[i] = false;
                            numActiveSockets--;
                            printf("[%d] [Stop] server send data stop to %s port %d using udp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
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
                    int res = tcp_send(socketHandles[i], socketConfigList[i].pktsize, &socketConfigList[i].pktnum, socketConfigList[i].pktnum,
                                       socketConfigList[i].pktrate, 500, using_select);
                    //if (res != 1) printf("[%d] server send one packet from  %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
                    //printf("res is %d", res);

                    if (res == 1) {
                        socketValid[i] = false;
                        numActiveSockets--;
                        printf("[%d] [Stop] server send data stop to %s port %d using tcp\n", i, socketConfigList[i].client_ip, socketConfigList[i].client_port);
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
void argument_parse_server(int argc, char** argv, Server_argument* server_argument) {
    server_argument->lhost = (char*)"localhost";
    server_argument->lport = (char*)"4180";
    server_argument->rbufsize = 65536;
    server_argument->sbufsize = 65536;

    for (int i = 1; i < argc; i++) {
        if (i + 1 < argc && argv[i][0] == '-') {
            if (strcmp(argv[i], "-lhost") == 0) {
                server_argument->lhost = argv[i + 1];
                continue;
            }
            else if (strcmp(argv[i], "-lport") == 0) {
                server_argument->lport = argv[i + 1];
                continue;
            }
            else if (strcmp(argv[i], "-rbufsize") == 0) {
                server_argument->rbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            }
            else if (strcmp(argv[i], "-sbufsize") == 0) {
                server_argument->sbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            }
        }
    }
}

// this function is for print the prompt info (using -h)
void print_prompt_information_server() {
    printf(" welcome to use the NetProbe\n");
    printf("-lhost hostname” hostname to bind to. (Default late binding, i.e., IN_ADDR_ANY)\n");
    printf("-lport portnum” port number to bind to. (Default “4180”)\n");
    printf("-sbufsize bsize” set the outgoing socket buffer size to bsize bytes\n");
    printf("-rbufsize bsize” set the incoming socket buffer size to bsize bytes\n\n\n");
}
//// this function is to get sys_packet from client
//void sys_server_from_client(server_argument server_argument, Sys_packet *sys_packet) {
//    int server_listen_socket;
//    struct sockaddr_in address;
//    int addrlen = sizeof(address);
//    char ip[100];
//    solve_hostname_to_ip_linux(server_argument.lhost, ip);
//    printf("%s has been resolved to %s\n", server_argument.lhost, ip);
//
//    // Creating socket file descriptor
//    if ((server_listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//        perror("socket failed");
//        exit(EXIT_FAILURE);
//    }
//    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = INADDR_ANY;
//    address.sin_port = htons(atoi(server_argument.lport));
//
//    // Forcefully attaching socket to the port
//    if (bind(server_listen_socket, (struct sockaddr*)&address,
//             sizeof(address))
//        < 0) {
//        perror("bind failed");
//        exit(EXIT_FAILURE);
//    }
//    if (listen(server_listen_socket, 5) < 0) {
//        perror("listen");
//        exit(EXIT_FAILURE);
//    }
//    int new_socket = accept(server_listen_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
//    if (new_socket < 0) {
//        perror("accept");
//        exit(EXIT_FAILURE);
//    }
//    char sysPacketBuf[sys_packet_size + 1];
//    sysPacketBuf[sys_packet_size] = '\0';
//    int len = sys_packet_size;
//    int num_read = read(new_socket, sysPacketBuf, len);
//    if (num_read == 0) { // connection closed
//        close(new_socket);
//        close(server_listen_socket);
//    }
//    else {
//        Sys_packet sysPacket;
//        parse_sys_packet(sysPacketBuf, &sysPacket);
//        out_packet(sysPacket);
//    }
//    close(new_socket);
//    close(server_listen_socket);
//}

// this function is to print the sys packet
void  out_packet(Sys_packet sysPacket) {
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
void init_statistics(Statistics* statistics) {
    statistics->stat = 0;
    statistics->pktsize = 0;
    statistics->pktnum = 0;
    statistics->left_pktnum = 0;
    statistics->pktrate = 0;
    statistics->J_i_list = (double*) malloc(Jitter_max * sizeof(double));
    memset(statistics->J_i_list, 0, Jitter_max * sizeof(double));
    statistics->inter_arrival_list = (double*) malloc(Jitter_max * sizeof(double));
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