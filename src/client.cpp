#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// \\Mac\Home\Documents\GitHub\IERG4180_Project1\NetProbe\x64\Debug
// \\Mac\Home\Documents\GitHub\example
#undef UNICODE

# include "client.h"

int main(int argc, char **argv) {
    if (argc < 1) {
        printf("your input arguments should be more\n");
        return 0;
    }
    Client_argument client_argument;
    argument_parse_client(argc, argv, &client_argument);
    if (strcmp("-send", argv[1]) != 0 && strcmp("-recv", argv[1]) != 0) {
        printf("input arguments wrong\n");
        print_prompt_information_server();
        return 0;
    }
    client(argc, argv);

    return 0;
}

// this function is for client
int client(int argc, char **argv) {
    Client_argument client_argument;
    argument_parse_client(argc, argv, &client_argument);

    printf("the following content is the sender arguments\n\n");
    printf("mode  is %d\n", client_argument.mode);
    printf("stat  is %ld\n", client_argument.stat);
    printf("rhost is %s\n", client_argument.rhost);
    printf("rport is %s\n", client_argument.rport);
    printf("proto is %d\n", client_argument.proto);
    printf("pktsize is %ld\n", client_argument.pktsize);
    printf("pktrate is %ld\n", client_argument.pktrate);
    printf("pktnum is %ld\n", client_argument.pktnum);
    printf("sbufsize is %ld\n", client_argument.sbufsize);
    printf("rbufsize is %ld\n", client_argument.rbufsize);
    printf("\n\n\n");

    send_sys_packet(client_argument);
    return 0;
}

//// this function is for receive
//int recv(int argc, char** argv) {
//    Receiver_argument  receiver_argument;
//    argument_parse_receiver(argc, argv, &receiver_argument);
//
//    printf("the following content is the recevier arguments\n\n");
//    printf("stat  is %ld\n", receiver_argument.stat);
//    printf("lhost is %s\n", receiver_argument.lhost);
//    printf("lport is %s\n", receiver_argument.lport);
//    printf("proto is %s\n", receiver_argument.proto);
//    printf("pktsize is %ld\n", receiver_argument.pktsize);
//    printf("rbufsize is %ld\n", receiver_argument.rbufsize);
//    printf("\n\n\n");
//
//    if (strcmp(receiver_argument.proto, "tcp") == 0) {
//        tcp_recv(receiver_argument);
//    }
//    else if (strcmp(receiver_argument.proto, "udp") == 0) {
//        udp_recv(receiver_argument);
//    }
//
//    return 0;
//}


// this function is to parse the sender argument through argv
void argument_parse_client(int argc, char **argv, Client_argument *client_argument) {
    client_argument->mode = 0;
    client_argument->stat = 500;
    client_argument->rhost = (char *) "localhost";
    client_argument->rport = (char *) "4180";
    client_argument->proto = 0;
    client_argument->pktsize = 1000;
    client_argument->pktrate = 1000;
    client_argument->pktnum = 0;
    client_argument->sbufsize = 65536;
    client_argument->rbufsize = 65536;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-send") == 0) {
            client_argument->mode = 0;
            continue;
        } else if (strcmp(argv[i], "-recv") == 0) {
            client_argument->mode = 1;
            continue;
        }
        if (i + 1 < argc && argv[i][0] == '-') {
            if (strcmp(argv[i], "-stat") == 0) {
                client_argument->stat = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-rhost") == 0) {
                client_argument->rhost = argv[i + 1];
                continue;
            } else if (strcmp(argv[i], "-rport") == 0) {
                client_argument->rport = argv[i + 1];
                continue;
            } else if (strcmp(argv[i], "-proto") == 0) {
                if (strcmp(argv[i + 1], "udp") == 0) {
                    client_argument->proto = 0;
                } else if (strcmp(argv[i + 1], "tcp") == 0) {
                    client_argument->proto = 1;
                }
                continue;
            } else if (strcmp(argv[i], "-pktsize") == 0) {
                client_argument->pktsize = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-pktrate") == 0) {
                client_argument->pktrate = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-pktnum") == 0) {
                client_argument->pktnum = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-sbufsize") == 0) {
                client_argument->sbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            } else if (strcmp(argv[i], "-rbufsize") == 0) {
                client_argument->rbufsize = strtol(argv[i + 1], NULL, 10);
                continue;
            }
        }
    }
}


//// this function is for tcp sender
//int tcp_send(Client_argument  sender_argument) {
//
//    WSADATA wsaData;
//    int iResult;
//    SOCKET ConnectSocket = INVALID_SOCKET;
//
//    // Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        printf("WSAStartup failed: %d\n", iResult);
//        return 1;
//    }
//
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    // Resolve the server address and port
//    iResult = getaddrinfo(sender_argument.rhost, sender_argument.rport, &hints, &result);
//    if (iResult != 0) {
//        printf("getaddrinfo failed: %d\n", iResult);
//        WSACleanup();
//        return 1;
//    }
//
//    // Attempt to connect to an address until one succeeds
//    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//
//        // Create a SOCKET for connecting to server
//        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
//                               ptr->ai_protocol);
//        if (ConnectSocket == INVALID_SOCKET) {
//            printf("socket failed with error: %ld\n", WSAGetLastError());
//            WSACleanup();
//            return 1;
//        }
//        SetSendBufferSizeLinux(sender_argument.sbufsize, ConnectSocket);
//        int optVal;
//        int optLen = sizeof(int);
//        if (getsockopt(ConnectSocket, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, &optLen) != SOCKET_ERROR) {
//            printf("SockOpt Value: %ld\n", optVal);
//        }
//        //---------------------------------------------
//        // Set up the RecvAddr structure with the IP address of
//        // the receiver and the specified port number.
//        // Connect to server.
//        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//        if (iResult == SOCKET_ERROR) {
//            closesocket(ConnectSocket);
//            ConnectSocket = INVALID_SOCKET;
//            continue;
//        }
//        break;
//    }
//    freeaddrinfo(result);
//    if (ConnectSocket == INVALID_SOCKET) {
//        printf("Unable to connect to server!\n");
//        WSACleanup();
//        return 1;
//    }
//
//    //  int recvbuflen = sender_argument.sbufsize;
//    // const char* sendbuf = generate_message(sender_argument.pktsize, -1);
//    // char recvbuf[DEFAULT_BUFLEN];
//    //char* SendBuf = generate_message(sender_argument.sbufsize, 10);
//    char* message = NULL;
//    wprintf(L"Sending a datagram to the receiver using tcp...\n");
//    clock_t current_clock, previous_clock = clock();
//    double cum_time_cost = 0, total_time_cost = 0, cum_bytes_send = 0;
//    int message_num = sender_argument.pktnum;
//    int cum_packet_num = 0, totol_packet_num = 0;
//    bool falg_exit = 0;
//    double single_iter_pkt_threshold = sender_argument.pktrate * ((double)sender_argument.stat / 1000);
//    printf("threshold is %f\n", single_iter_pkt_threshold);
//    while (!falg_exit) {
//        if (cum_bytes_send < single_iter_pkt_threshold || single_iter_pkt_threshold == 0) {
//            int bytes_sent = 0;
//            message = generate_message(sender_argument.pktsize, -1);
//            while (bytes_sent < sender_argument.pktsize) {
//                iResult = client(ConnectSocket, message + bytes_sent, sender_argument.pktsize - bytes_sent, 0);
//                if (iResult == SOCKET_ERROR) {
//                    wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
//                    closesocket(ConnectSocket);
//                    WSACleanup();
//                    return 1;
//                }
//                if (iResult > 0) {
//                    bytes_sent += iResult;
//                }
//            }
//            cum_bytes_send += bytes_sent;
//            cum_packet_num++;
//            totol_packet_num++;
//        }
//        // beacuse the sender_argument.pktnum is 0 (default)
//        // then it never be equal to cum_packet_num
//        if (totol_packet_num == sender_argument.pktnum) {
//            falg_exit = 1;
//        }
//        current_clock = clock();
//        double time_cost = ((double)current_clock - previous_clock) / CLOCKS_PER_SEC;
//        previous_clock = current_clock;
//        cum_time_cost += time_cost;
//        if (cum_time_cost * 1000 >= (double)sender_argument.stat) {
//            double throughput = (cum_bytes_send * 8) / (cum_time_cost * 1000000);
//            total_time_cost += cum_time_cost;
//            printf("Sender : [Elapsed] %10.2f ms  [Pkts] %7d   [Rate] %.2f Mbps\n",
//                   total_time_cost * 1000, cum_packet_num, throughput);
//            cum_time_cost = 0;
//            cum_packet_num = 0;
//            cum_bytes_send = 0;
//        }
//
//    }
//    // Send an initial buffer
//    /*int times = 20;
//    while (times >= 0) {
//        iResult = client(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
//        if (iResult == SOCKET_ERROR) {
//            printf("client failed: %d\n", WSAGetLastError());
//            closesocket(ConnectSocket);
//            WSACleanup();
//            return 1;
//        }
//        printf("Bytes Sent: %ld\n", iResult);
//        times--;
//    }*/
//
//
//
//
//
//    // shutdown the connection for sending since no more data will be sent
//    // the client can still use the ConnectSocket for receiving data
//    iResult = shutdown(ConnectSocket, SD_SEND);
//    if (iResult == SOCKET_ERROR) {
//        printf("shutdown failed: %d\n", WSAGetLastError());
//        closesocket(ConnectSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    // cleanup
//    closesocket(ConnectSocket);
//    WSACleanup();
//
//    return 0;
//
//}
//
//// this function is for tcp receiver
//int tcp_recv(Receiver_argument receiver_argument) {
//    WSADATA wsaData;
//    int iResult;
//
//    SOCKET ClientSocket = INVALID_SOCKET;
//    SOCKET ListenSocket = INVALID_SOCKET;
//
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        printf("WSAStartup failed: %d\n", iResult);
//        return 1;
//    }
//
//    ZeroMemory(&hints, sizeof(hints));
//
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    hints.ai_family = AF_INET;
//    hints.ai_flags = AI_PASSIVE;
//
//    // Resolve the local address and port to be used by the server
//
//    iResult = getaddrinfo(receiver_argument.lhost, receiver_argument.lport, &hints, &result);
//    if (iResult != 0) {
//        printf("getaddrinfo failed: %d\n", iResult);
//        WSACleanup();
//        return 1;
//    }
//    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//
//    //ListenSocket = socket(result->ai_family, result->ai_socktype, 0);
//    if (ListenSocket == INVALID_SOCKET) {
//        printf("Error at socket(): %ld\n", WSAGetLastError());
//        freeaddrinfo(result);
//        WSACleanup();
//        return 1;
//    }
//
//    // Setup the TCP listening socket
//    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//    if (iResult == SOCKET_ERROR) {
//        printf("bind failed with error: %d\n", WSAGetLastError());
//        freeaddrinfo(result);
//        closesocket(ListenSocket);
//        WSACleanup();
//        return 1;
//    }
//    freeaddrinfo(result);
//
//    iResult = listen(ListenSocket, SOMAXCONN);
//    if (iResult == SOCKET_ERROR) {
//        printf("listen failed with error: %d\n", WSAGetLastError());
//        closesocket(ListenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//
//    // Accept a client socket
//    ClientSocket = accept(ListenSocket, NULL, NULL);
//    if (ClientSocket == INVALID_SOCKET) {
//        printf("accept failed: %d\n", WSAGetLastError());
//        closesocket(ListenSocket);
//        WSACleanup();
//        return 1;
//    }
//    closesocket(ListenSocket);
//    SetReceiveBufferSizeLinux(receiver_argument.rbufsize, ClientSocket);
//    int optVal;
//    int optLen = sizeof(int);
//    if (getsockopt(ClientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen) != SOCKET_ERROR)
//        printf("SockOpt Value: %ld\n", optVal);
//    wprintf(L"Receiving datagrams using TCP...\n");
//    char* recvBuf = (char*)malloc((receiver_argument.pktsize + 1) * sizeof(char));
//    clock_t current_clock, previous_clock = clock();
//    //printf("previous clock1 is %f\n", (double)previous_clock);
//    //clock_t current_message_arrival_time, pervious_message_arrival_time = clock();
//    double cum_time_cost = 0, total_time_cost = 0, cum_byte_recv = 0;
//    int cum_packet_num = 0, total_packet_num = 0, byte_recieve = 0;
//    bool falg_exit = 0;
//    double* inter_arrival_list = (double*)malloc(Jitter_max * sizeof(double));
//    double* J_i_list = (double*)malloc(Jitter_max * sizeof(double));
//    while (!falg_exit) {
//        byte_recieve = 0;
//        while (byte_recieve < receiver_argument.pktsize) {
//            iResult = recv(ClientSocket, recvBuf, receiver_argument.pktsize - byte_recieve, 0);
//            //printf("previous clock3 is %f\n", (double)previous_clock);
//            if (iResult == SOCKET_ERROR) {
//                wprintf(L"recv function failed with error %d\n", WSAGetLastError());
//                closesocket(ClientSocket);
//                WSACleanup();
//                return 1;
//            }
//            else {
//                if (iResult == 0) {
//                    printf("Connection closing...\n");
//                    falg_exit = 1;
//                    break;
//                }
//                else {
//                    //break;
//                    byte_recieve += iResult;
//                    // printf("iResult is %d\n", iResult);
//                    //printf("recieve bytes: %d\n", byte_recieve);
//                }
//            }
//        }
//        cum_byte_recv += byte_recieve;
//        byte_recieve = 0;
//        // printf("after recieve bytes\n");
//        current_clock = clock();
//        long double time_cost = ((long double)current_clock - previous_clock) / CLOCKS_PER_SEC;
//        /* printf("current clock is %f\n", (double)current_clock);
//         printf("previous clock4 is %f\n", (double)previous_clock);
//         printf("time cost is %f\n", time_cost);*/
//        inter_arrival_list[cum_packet_num] = time_cost * 1000;
//        cum_packet_num++;
//        double D = calculate_average_value(inter_arrival_list, cum_packet_num);
//        J_i_list[cum_packet_num - 1] = time_cost * 1000 - D;
//        previous_clock = current_clock;
//        cum_time_cost += time_cost;
//        //printf(" %f ------%f\n ", cum_time_cost * 1000, receiver_argument.stat);
//        //if (!(cum_time_cost * 1000 >= (double)receiver_argument.stat)) printf("wrong!\n");
//        //else printf("true");
//        if (cum_time_cost * 1000 >= (double)receiver_argument.stat) {
//            double throughput = (cum_byte_recv * 8) / (cum_time_cost * 1000000);
//            double jitter = calculate_average_value(J_i_list, cum_packet_num);
//            total_time_cost += cum_time_cost;
//            printf("Receiver : [Elapsed] %10.2f ms  [Pkts] %7.d   [Rate] %.2f Mbps  [Jitter] %.6f ms\n",
//                   total_time_cost * 1000, cum_packet_num, throughput, jitter);
//            cum_time_cost = 0;
//            cum_packet_num = 0;
//            cum_byte_recv = 0;
//        }
//    }
//    // Receive until the peer shuts down the connection
//    /*do {
//
//        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
//        if (iResult > 0) {
//            printf("Bytes received: %d\n", iResult);
//        }
//        else if (iResult == 0)
//            printf("Connection closing...\n");
//        else {
//            printf("recv failed with error: %d\n", WSAGetLastError());
//            closesocket(ClientSocket);
//            WSACleanup();
//            return 1;
//        }
//
//    } while (iResult > 0);*/
//
//    // shutdown the client half of the connection since no more data will be sent
//    iResult = shutdown(ClientSocket, SD_SEND);
//    if (iResult == SOCKET_ERROR) {
//        printf("shutdown failed: %d\n", WSAGetLastError());
//        closesocket(ClientSocket);
//        WSACleanup();
//        return 1;
//    }
//    // cleanup
//    closesocket(ClientSocket);
//    WSACleanup();
//
//    return 0;
//}
//
//// this function is for udp sender
//int udp_send(Client_argument  sender_argument) {
//    int iResult;
//    WSADATA wsaData;
//
//    SOCKET SendSocket = INVALID_SOCKET;
//    sockaddr_in RecvAddr;
//    unsigned short Port = strtol(sender_argument.rport, NULL, 10);
//
//    // Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != NO_ERROR) {
//        wprintf(L"WSAStartup failed with error: %d\n", iResult);
//        return 1;
//    }
//    //---------------------------------------------
//    // Create a socket for sending data
//    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    if (SendSocket == INVALID_SOCKET) {
//        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
//        WSACleanup();
//        return 1;
//    }
//    SetSendBufferSizeLinux(sender_argument.sbufsize, SendSocket);
//    /*int optVal;
//    int optLen = sizeof(int);
//    if (getsockopt(SendSocket,
//        SOL_SOCKET,
//        SO_SNDBUF,
//        (char*)&optVal,
//        &optLen) != SOCKET_ERROR)
//        printf("SockOpt Value: %ld\n", optVal);*/
//    //---------------------------------------------
//    // Set up the RecvAddr structure with the IP address of
//    // the receiver and the specified port number.
//    RecvAddr.sin_family = AF_INET;
//    RecvAddr.sin_port = htons(Port);
//    char* ip = NULL;
//    RecvAddr.sin_addr.s_addr = inet_addr(get_ip_from_hostname(sender_argument.rhost, ip));
//    //---------------------------------------------
//    // Send  datagram to the receiver
//    char* SendBuf = generate_message(sender_argument.sbufsize, 10);
//    char* message = NULL;
//    wprintf(L"Sending a datagram to the receiver...\n");
//    clock_t current_clock, previous_clock = clock();
//    double cum_time_cost = 0, total_time_cost = 0;
//    int message_num = sender_argument.pktnum;
//    int cum_packet_num = 0, total_packet_num = 0;
//    bool falg_exit = 0;
//    double single_iter_pkt_threshold = sender_argument.pktrate * ((double)sender_argument.stat / 1000);
//    printf("threshold is %f\n", single_iter_pkt_threshold);
//    while (!falg_exit) {
//        if (cum_packet_num * sender_argument.pktsize < single_iter_pkt_threshold || abs(single_iter_pkt_threshold - 0) < 0.1) {
//            message = generate_message(sender_argument.pktsize, cum_packet_num);
//            iResult = sendto(SendSocket, message, sender_argument.pktsize, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
//            if (iResult == SOCKET_ERROR) {
//                wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
//                closesocket(SendSocket);
//                WSACleanup();
//                return 1;
//            }
//
//            cum_packet_num++;
//            total_packet_num++;
//        }
//        // beacuse the sender_argument.pktnum is 0 (default)
//        // then it never be equal to cum_packet_num
//        if (total_packet_num == sender_argument.pktnum) {
//            falg_exit = 1;
//        }
//        current_clock = clock();
//        double time_cost = ((double)current_clock - previous_clock) / CLOCKS_PER_SEC;
//        previous_clock = current_clock;
//        cum_time_cost += time_cost;
//        if (cum_time_cost * 1000 >= (double)sender_argument.stat) {
//            double throughput = (cum_packet_num * sender_argument.pktsize * 8) / (cum_time_cost * 1000000);
//            total_time_cost += cum_time_cost;
//            printf("Sender : [Elapsed] %10.2f ms  [Pkts] %7d   [Rate] %.2f Mbps\n",
//                   total_time_cost * 1000, cum_packet_num, throughput);
//            cum_time_cost = 0;
//            cum_packet_num = 0;
//        }
//
//    }
//    //---------------------------------------------
//    // When the application is finished sending, close the socket.
//    wprintf(L"Finished sending. Closing socket.\n");
//    iResult = closesocket(SendSocket);
//    if (iResult == SOCKET_ERROR) {
//        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
//        WSACleanup();
//        return 1;
//    }
//    //---------------------------------------------
//    // Clean up and quit.
//    wprintf(L"Exiting.\n");
//    WSACleanup();
//    return 0;
//}
//
//// this function is for udp receiver
//int udp_recv(Receiver_argument receiver_argument) {
//    int iResult = 0;
//    WSADATA wsaData;
//
//    SOCKET RecvSocket;
//    struct sockaddr_in RecvAddr;
//
//    unsigned short Port = strtol(receiver_argument.lport, NULL, 10);
//
//    struct sockaddr_in SenderAddr;
//    int SenderAddrSize = sizeof(SenderAddr);
//
//    //-----------------------------------------------
//    // Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != NO_ERROR) {
//        wprintf(L"WSAStartup failed with error %d\n", iResult);
//        return 1;
//    }
//    //-----------------------------------------------
//    // Create a receiver socket to receive datagrams
//    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    if (RecvSocket == INVALID_SOCKET) {
//        wprintf(L"socket failed with error %d\n", WSAGetLastError());
//        return 1;
//    }
//    SetReceiveBufferSizeLinux(receiver_argument.rbufsize, RecvSocket);
//    /* int optVal;
//    int optLen = sizeof(int);
//    if (getsockopt(RecvSocket,
//        SOL_SOCKET,
//        SO_RCVBUF,
//        (char*)&optVal,
//        &optLen) != SOCKET_ERROR)
//        printf("SockOpt Value: %ld\n", optVal);*/
//    //-----------------------------------------------
//    // Bind the socket to any address and the specified port.
//    RecvAddr.sin_family = AF_INET;
//    RecvAddr.sin_port = htons(Port);
//    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//
//    iResult = bind(RecvSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
//    if (iResult != 0) {
//        wprintf(L"bind failed with error %d\n", WSAGetLastError());
//        return 1;
//    }
//    //-----------------------------------------------
//    // Call the recvfrom function to receive datagrams
//    // on the bound socket.
//    wprintf(L"Receiving datagrams using UDP...\n");
//    char* RecvBuf = (char*)malloc(receiver_argument.pktsize * sizeof(char));
//    clock_t current_clock, previous_clock = clock();
//    //clock_t current_message_arrival_time, pervious_message_arrival_time = clock();
//    double cum_time_cost = 0, total_time_cost = 0, cum_loss = 0;
//    int message_num = 0, max_sequence = 0, min_sequence = 0, current_squence = 0;
//    int cum_packet_num = 0, total_pakck_num = 0;
//    bool falg_exit = 0;
//    double* inter_arrival_list = (double*)malloc(Jitter_max * sizeof(double));
//    double* J_i_list = (double*)malloc(Jitter_max * sizeof(double));
//
//    while (!falg_exit) {
//        iResult = recvfrom(RecvSocket, RecvBuf, receiver_argument.pktsize,
//                           0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
//        if (iResult == SOCKET_ERROR) {
//            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
//        }
//        current_clock = clock();
//        double time_cost = ((double)current_clock - previous_clock) / CLOCKS_PER_SEC;
//        current_squence = get_sequence_number(RecvBuf);
//        min_sequence = current_squence < min_sequence ? current_squence : min_sequence;
//        max_sequence = current_squence > max_sequence ? current_squence : max_sequence;
//        inter_arrival_list[cum_packet_num] = time_cost * 1000;
//        cum_packet_num++;
//        total_pakck_num++;
//        double D = calculate_average_value(inter_arrival_list, cum_packet_num);
//        J_i_list[cum_packet_num - 1] = time_cost * 1000 - D;
//        previous_clock = current_clock;
//        cum_time_cost += time_cost;
//        cum_loss = abs((max_sequence - min_sequence) - cum_packet_num + 1);
//
//        if (cum_time_cost * 1000 >= (double)receiver_argument.stat) {
//            double throughput = (cum_packet_num * receiver_argument.pktsize * 8) / (cum_time_cost * 1000000);
//            double jitter = calculate_average_value(J_i_list, cum_packet_num);
//            total_time_cost += cum_time_cost;
//            printf("Sender : [Elapsed] %10.2f ms  [Pkts] %7.d  Lost[%7.0f, %3.2f%%]  [Rate] %.2f Mbps  [Jitter] %.6f ms\n",
//                   total_time_cost * 1000, cum_packet_num, cum_loss,
//                   (cum_loss * 1.0 / abs(max_sequence - min_sequence + 1)) * 100, throughput, jitter);
//            cum_time_cost = 0;
//            cum_packet_num = 0;
//            max_sequence = -1;
//            min_sequence = INT_MAX;
//        }
//    }
//
//    //-----------------------------------------------
//    // Close the socket when finished receiving datagrams
//    wprintf(L"Finished receiving. Closing socket.\n");
//    iResult = closesocket(RecvSocket);
//    if (iResult == SOCKET_ERROR) {
//        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
//        return 1;
//    }
//
//    //-----------------------------------------------
//    // Clean up and exit.
//    wprintf(L"Exiting.\n");
//    WSACleanup();
//    return 0;
//}

// this function is for print the prompt info (using -h)
void print_prompt_information_server() {
    printf("welcome to use the NetProbe\n");
    printf("-send means sending mode \n-recv means receiving mode\n\n");
    printf("-stat yyy set update of statistics to be once yyy ms. Default = 500 ms)\n");
    printf("-rhost hostname client data to host specified by hostname. (Default 'localhost')\n");
    printf("-rport portnum client data to remote host at port number portnum. (Default '4180')\n");
    printf("-proto[tcp / udp] client data using TCP or UDP. (Default 'UDP')\n");
    printf("-pktsize bsize client message of bsize bytes. (Default 1000 bytes including application header)\n");
    printf("-pktrate txrate client data at a data rate of txrate bytes per second, ‘0’ means as fast as possible. (Default 1000 bytes / second)\n");
    printf("-pktnum num client a total of num messages. (Default = '0' = infinite)\n");
    printf("-rbufsize bsize set the incoming socket buffer size to bsize bytes \n");
    printf("-sbufsize bsize set the outgoing socket buffer size to bsize bytes.\n\n");
}

// this function is to send sys_packet to server
void send_sys_packet(Client_argument client_argument) {
    InitializeWinsock();
    int new_port = get_free_port();
    std::string sys_packet = get_sys_packet_string(client_argument, new_port);
    std::cout << sys_packet << std::endl;
#ifdef _WIN32
    SOCKET sockfd, connfd;
#elif __linux__
    int sockfd, connfd;
#endif
    struct sockaddr_in servaddr, cli;
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    char ip[100];
    solve_hostname_to_ip_linux(client_argument.rhost, ip);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(atoi(client_argument.rport));

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else {
        printf("connected to the server.. sending the sys packet\n");
    }
    char buff[sys_packet_size + 1];
    strcpy(buff, sys_packet.c_str());
    int n;
#ifdef __linux__
    write(sockfd, buff, sys_packet_size);
#elif _WIN32
    send(sockfd,buff, sys_packet_size, 0);
#endif
    if (client_argument.mode == 0 && client_argument.proto == 1) { // client send tcp
        int using_select = 0;
        int * left_pktnum = (int *) malloc(sizeof(int));
        *left_pktnum = client_argument.pktnum;

        int closing_socket = tcp_send(sockfd,  client_argument.pktsize, left_pktnum, client_argument.pktnum,client_argument.pktrate, client_argument.stat,using_select);
        if (closing_socket == 1) {
#ifdef __linux__
            close(sockfd);
#elif _WIN32
            closesocket(sockfd);
#endif
        }

    }
    if (client_argument.mode == 1 && client_argument.proto == 1) { // server send tcp
        printf("client is receiving data !\n");
        int using_select = 0;
        clock_t previous_clock = clock();
        double * inter_arrival_list = (double*) malloc(Jitter_max * sizeof(double));
        double * J_i_list = (double*) malloc(Jitter_max * sizeof(double));
        int cum_packet_number = 0;
        double cum_time_cost = 0;
        double cum_bytes_recv = 0;
        double cum_time_cost_session = 0;
        int closing_socket =tcp_recv(sockfd, client_argument.pktsize, &previous_clock, inter_arrival_list, J_i_list,
                 &cum_packet_number, &cum_time_cost, &cum_bytes_recv, client_argument.stat,
                 &cum_time_cost_session, using_select);
        if (closing_socket == 1) {
#ifdef __linux__
            close(sockfd);
#elif _WIN32
            closesocket(sockfd);
#endif
        }
    }
    if (client_argument.mode == 0 && client_argument.proto == 0) {// client send udp
        char new_port_buf[6];
        memset(new_port_buf, '\0', 6);
#ifdef __linux__
        read(sockfd, new_port_buf, 5);
#elif _WIN32
        int ret = recv(sockfd, new_port_buf, 5, 0);
        if ((ret == SOCKET_ERROR)) {
            printf("Recv failed with error code : %d\n", WSAGetLastError());
            Sleep(3000);
        }
#endif
        int port = atoi(new_port_buf);
        printf("get new port %d\n", port);
        int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in new_udp_socket_client_address;
        memset(&new_udp_socket_client_address, 0, sizeof(struct sockaddr_in));
        new_udp_socket_client_address.sin_family = AF_INET;
        new_udp_socket_client_address.sin_port = htons(port);
        new_udp_socket_client_address.sin_addr.s_addr = servaddr.sin_addr.s_addr;
        int using_select = 0;
        int * left_pktnum = (int *) malloc(sizeof(int));
        *left_pktnum = client_argument.pktnum;
        int p_num_index = 0;
        int closing_socket = udp_send(new_udp_socket, &p_num_index, client_argument.pktsize, left_pktnum, client_argument.pktnum,client_argument.pktrate, client_argument.stat,new_udp_socket_client_address, using_select);
//        if (closing_socket == 1) {
//            close(new_udp_socket);
//            close(sockfd);
//        }
        if (closing_socket == 1) {
#ifdef __linux__
            close(new_udp_socket);
            close(sockfd);
#elif _WIN32
            closesocket(sockfd);
            closesocket(new_udp_socket);
#endif
        }
    }
    if (client_argument.mode == 1 && client_argument.proto == 0) {// server send udp
        int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in new_udp_socket_client_address;
        memset(&new_udp_socket_client_address, 0, sizeof(struct sockaddr_in));
        new_udp_socket_client_address.sin_family = AF_INET;
        new_udp_socket_client_address.sin_port = htons(new_port);

        char * ip = (char *)malloc(15 *sizeof(char *));
        get_ip(ip);
        printf("ip is %s\n",ip);
        printf("port is %d\n",new_port);
        new_udp_socket_client_address.sin_addr.s_addr = inet_addr(ip);
        SetReceiveBufferSizeLinux(new_udp_socket, 65533);
        bind(new_udp_socket, (struct sockaddr*)&new_udp_socket_client_address, sizeof(new_udp_socket_client_address));
        int using_select = 0;
        clock_t previous_clock = clock();
        double * inter_arrival_list = (double*) malloc(Jitter_max * sizeof(double));
        double * J_i_list = (double*) malloc(Jitter_max * sizeof(double));
        int cum_packet_number = 0;
        double cum_time_cost = 0;
        double cum_bytes_recv = 0;
        double cum_time_cost_session = 0;
        int previous_SN = 0;
        int total_packet_loss = 0;
        int res = udp_recv(new_udp_socket, client_argument.pktsize, &previous_clock,
                           inter_arrival_list, J_i_list,
                           &cum_packet_number, &cum_time_cost,
                           &cum_bytes_recv, client_argument.stat, &cum_time_cost_session,
                           &previous_SN, &total_packet_loss, using_select);
        if (res == 1) {
            free(inter_arrival_list);
            free(J_i_list);
        }
    }
#ifdef __linux__
    close(sockfd);
#elif _WIN32
    closesocket(sockfd);
#endif
}

// this function is for generate the sys packet string
std::string get_sys_packet_string(Client_argument client_argument, int out_new_port) {
    std::string mode = my_int_to_string(client_argument.mode);
    std::string proto = my_int_to_string(client_argument.proto);
    std::string pktnum = add_zero(my_int_to_string(client_argument.pktnum), 10, '0');
    std::string pktsize = add_zero(my_int_to_string(client_argument.pktsize), 10, '0');
    std::string pktrate = add_zero(my_int_to_string(client_argument.pktrate), 10, '0');
    std::string port = add_zero(my_int_to_string(out_new_port), 10, '0');
    char * ip = (char *)malloc(15 *sizeof(char *));
    get_ip(ip);
    std :: string temp_ip(ip);
    std::string string_ip = add_zero(temp_ip, 15, '0');
    std::string sys_packet_string = mode + proto + pktnum + pktsize + pktrate + port + string_ip;
    return sys_packet_string;
}

