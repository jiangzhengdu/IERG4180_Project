//
// this is for util
//
#include "util.h"

// this function is for convert from hostname to ip
int solve_hostname_to_ip_linux(const char *hostname, char *ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
    if ((he = gethostbyname(hostname)) == NULL) {
        // get the host info
#ifdef __linux__
        herror("gethostbyname");
#endif
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }
    return 1;
}

// this function is to set sender buffer size
long SetSendBufferSizeLinux(long size, int sockfd) {
    int Size = size;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,
                   (char *) (&Size), sizeof(Size)) < 0)
        return -1;
    else return Size;
}

// this function is to set receive buffer size
long SetReceiveBufferSizeLinux(long size, int sockfd) {
    int Size = size;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,
                   (char *) (&Size), sizeof(Size)) < 0)
        return -1;
    else return Size;
}

// this function is to generate the message with pakage number
// param message_length : the length of message
// param sequence_number : if -1 then generate random message else input the sequence_number
char *generate_message(int message_length, int sequence_number) {
    char *message = (char *) malloc(message_length * sizeof(char));
    memset(message, '\0', message_length * sizeof(char));
    if (sequence_number == -1) { // generate the random message
        int index = 0;
        for (int i = 0; i < message_length; i++) {
            message[i] = '0' + index;
            index++;
            if (index == 10) {
                index = 0;
            }
        }
        message[message_length - 1] = '\0';
        return message;
    } else {
        //itoa(sequence_number, message, 10);
        sprintf(message, "%d", sequence_number);
        // printf("sequence_number is %d\n", sequence_number);
        // printf("mess is %s\n", message);
        int flag = 0;
        for (int i = 0; i < message_length - 1; i++) {
            if (flag == 1) {
                message[i] = '0';
            }
            if (message[i] == '\0') {
                flag = 1;
                message[i] = '#';
                continue;
            }
        }
        return message;
    }

}


// this function is to extract the sequence number from a message
int get_sequence_number(char *message) {
    int num = 0;
    int length = 0;
    for (length = 0;; length++) {
        if (message[length] == '#') {
            break;
        }
    }
    char *sequence = (char *) malloc((length + 1) * sizeof(char));
    strncpy(sequence, message, length);
    num = strtol(sequence, NULL, 10);
    return num;
}

// this function is to calculate the average n bits from a list
double calculate_average_value(double *list, int n) {
    long double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += list[i];
    }
    return sum / n;
}

// this function is to get a free port

int get_free_port() {
#ifdef __linux__
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        printf("socket error\n");
        return 0;
    }
   // printf("Opened %d\n", sock);

    struct sockaddr_in serv_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        if(errno == EADDRINUSE) {
            printf("the port is not available. already to other process\n");
            return 0;
        } else {
            printf("could not bind to process (%d) %s\n", errno, strerror(errno));
            return 0;
        }
    }
    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, (struct sockaddr *)&serv_addr, &len) == -1) {
        perror("getsockname");
        return 0;
    }
    //printf("port number %d\n", ntohs(serv_addr.sin_port));
    if (close (sock) < 0 ) {
        printf("did not close: %s\n", strerror(errno));
        return 0;
    }
    return ntohs(serv_addr.sin_port);
#elif _WIN32
    InitializeWinsock();
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket error\n");
        return 0;
    }
    printf("Opened %d\n", sock);

    struct sockaddr_in serv_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
//        if(errno == EADDRINUSE) {
//            printf("the port is not available. already to other process\n");
//            return 0;
//        } else {
//            printf("could not bind to process (%d) %s\n", errno, strerror(errno));
//            return 0;
//        }
        return 0;
    }
    int len = sizeof(serv_addr);
    if (getsockname(sock, (struct sockaddr *) &serv_addr, &len) == -1) {
        perror("getsockname");
        return 0;
    }
//printf("port number %d\n", ntohs(serv_addr.sin_port));
    if (closesocket(sock) < 0) {
        printf("did not close: %s\n", strerror(errno));
        return 0;
    }
    return ntohs(serv_addr.sin_port);
#endif

}

//int get_free_port() {
//    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//    if (sock < 0) {
//        printf("socket error\n");
//        return 0;
//    }
//    printf("Opened %d\n", sock);
//
//    struct sockaddr_in serv_addr;
//    memset((char *) &serv_addr, 0, sizeof(serv_addr));
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = INADDR_ANY;
//    serv_addr.sin_port = 0;
//    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
////        if(errno == EADDRINUSE) {
////            printf("the port is not available. already to other process\n");
////            return 0;
////        } else {
////            printf("could not bind to process (%d) %s\n", errno, strerror(errno));
////            return 0;
////        }
//        return 0;
//    }
//    int len = sizeof(serv_addr);
//    if (getsockname(sock, (struct sockaddr *) &serv_addr, &len) == -1) {
//        perror("getsockname");
//        return 0;
//    }
//    //printf("port number %d\n", ntohs(serv_addr.sin_port));
//    if (closesocket(sock) < 0) {
//        printf("did not close: %s\n", strerror(errno));
//        return 0;
//    }
//    return ntohs(serv_addr.sin_port);
//}
//


// this function is to parse sys_packet_buf to struct sys_packet
void parse_sys_packet(const char *sys_packet_buf, Sys_packet *sys_packet) {
    // 0 0 0000000000  0000001000  0000001000  0000000000  000000000000000
    // 0 1 2        11 12       21 22       31 32       41 42            56
//    std :: cout << "reciev buff is" << sys_packet_buf << std :: endl;
    sys_packet->mode = sys_packet_buf[0] - '0';
    sys_packet->proto = sys_packet_buf[1] - '0';
    char temp[11];
    memset(temp, '\0', 11 * sizeof(char));
    for (int i = 2, j = 0; i <= 11; i++, j++) {
        temp[j] = sys_packet_buf[i];
    }
    sys_packet->pktnum = atoi(temp);
    memset(temp, '\0', 11 * sizeof(char));
    for (int i = 12, j = 0; i <= 21; i++, j++) {
        temp[j] = sys_packet_buf[i];
    }
    sys_packet->pktsize = atoi(temp);
    memset(temp, '\0', 11 * sizeof(char));
    for (int i = 22, j = 0; i <= 31; i++, j++) {
        temp[j] = sys_packet_buf[i];
    }
    sys_packet->pktrate = atoi(temp);
    memset(temp, '\0', 11 * sizeof(char));
    for (int i = 32, j = 0; i <= 41; i++, j++) {
        temp[j] = sys_packet_buf[i];
    }
    sys_packet->client_port = atoi(temp);
    char temp1[20];
    memset(temp1, '\0', 20 * sizeof(char));
    int index = 42;
    while (index <= 56) {
        if (sys_packet_buf[index] == '0') { index++; }
        else break;
    }
    for (int i = index, j = 0; i <= 56; i++, j++) {
        temp1[j] = sys_packet_buf[i];
    }
    sys_packet->client_ip = (char *) malloc(20 * sizeof(char));
    strcpy(sys_packet->client_ip, temp1);
}

// this function is to get ip in linux
int get_ip(char *outip) {
#ifdef __linux__
    int i=0;
    int sockfd;
    struct ifconf ifconf;
    char buf[512];
    struct ifreq *ifreq;
    char* ip;
    //初始化ifconf
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        return -1;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf);    //获取所有接口信息
    close(sockfd);
    //接下来一个一个的获取IP地址
    ifreq = (struct ifreq*)buf;
    for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
    {
        ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);

        if(strcmp(ip,"127.0.0.1")==0)  //排除127.0.0.1，继续下一个
        {
            ifreq++;
            continue;
        }
        strcpy(outip,ip);
        return 0;
    }
    return -1;
#elif _WIN32
    InitializeWinsock();
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
        std::cerr << "Error " << WSAGetLastError() <<
                  " when getting local host name." << std::endl;
        return 1;
    }
    //std :: cout << "Host name is " << ac << "." << std ::endl;

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        std::cerr << "Yow! Bad host lookup." << std::endl;
        return 1;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        strcpy(outip, inet_ntoa(addr));
        std::cout << "Address " << i << ": " << inet_ntoa(addr) << std::endl;
        break;
    }

    return 0;
#endif
}

// this function is to get ip in win
//int get_ip(char * outip)
//{
//    char ac[80];
//    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
//        std ::cerr << "Error " << WSAGetLastError() <<
//                   " when getting local host name." << std ::endl;
//        return 1;
//    }
//    //std :: cout << "Host name is " << ac << "." << std ::endl;
//
//    struct hostent *phe = gethostbyname(ac);
//    if (phe == 0) {
//        std :: cerr << "Yow! Bad host lookup." << std ::endl;
//        return 1;
//    }
//
//    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
//        struct in_addr addr;
//        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
//        strcpy(outip, inet_ntoa(addr));
//        std ::cout << "Address " << i << ": " << inet_ntoa(addr) << std ::endl;
//        break;
//    }
//
//    return 0;
//}

// this function is for tcp recv
// return 1 if the socket should be closed
int tcp_recv(int socket, int pktsize,
             clock_t *previous_clock, double *inter_arrival_list, double *J_i_list,
             int *cum_packet_number, double *cum_time_cost, double *cum_bytes_recv, int stat,
             double *cum_time_cost_session, int using_select) {
    // printf("is recieveing data!\n");
    int close_socket = 0;
    clock_t current_clock, current_message_arrival_time, previous_message_arrival_time = clock();
//    double  total_time_cost = 0;
//    double * inter_arrival_list = (double*) malloc(Jitter_max * sizeof(double));
//    double * J_i_list = (double*) malloc(Jitter_max * sizeof(double));
//    int cum_packet_number = 0, byte_recieve = 0;
    int byte_recieve = 0;
    char *peer_data = (char *) malloc(MAX_BUFFER_RECV * sizeof(char));
    bool flag_exit = 0;

    while (!flag_exit) {
        // recv a packet size using tcp
        while (byte_recieve < pktsize) {
            int ret = recv(socket, peer_data, pktsize - byte_recieve, 0);
            if ((ret < 0)) {
                printf("Recv failed \n");
                // sleep(3);
                break;
            } else {
                if (ret == 0) {
                    close_socket = 1;
                    flag_exit = 1; // peer close
                    break;
                } else {
                    // printf("recv data is %s\n", peer_data);
                    byte_recieve = byte_recieve + ret;
                }
            }
        }
        // if using select mode ,the recv a packet then exit
        if (using_select == 1) {
            flag_exit = 1;
        }
        //  printf("done once recv\n");


        *cum_bytes_recv = *cum_bytes_recv + byte_recieve;
        byte_recieve = 0;
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) (*previous_clock)) / CLOCKS_PER_SEC;
        inter_arrival_list[*cum_packet_number] = time_cost * 1000;
        //printf("%f, %f\n", inter_arrival_list[cum_packet_number], time_cost);
        (*cum_packet_number)++;
        double D = calculate_average_value(inter_arrival_list, *cum_packet_number);
        //printf("%d\n", D);
        J_i_list[*cum_packet_number - 1] = time_cost * 1000 - D;
        *previous_clock = current_clock;
        *cum_time_cost = (*cum_time_cost) + time_cost;
        *cum_time_cost_session = (*cum_time_cost_session) + time_cost;
        if (stat == 0) {
            stat = 500;
        }
        if ((*cum_time_cost) * 1000 >= stat) {
            double throughput = ((*cum_bytes_recv) * 8) / ((*cum_time_cost) * 1000000);
            double jitter = calculate_average_value(J_i_list, *cum_packet_number);
            if (using_select != 1)
                printf("Receiver: [Elapsed] %.2f ms, [Pkts] %d, [Rate] %.2f Mbps, [Jitter] %.6f ms\n",
                       *cum_time_cost_session * 1000, *cum_packet_number, throughput, jitter);
            *cum_packet_number = 0;
            *cum_time_cost = 0;
            *cum_bytes_recv = 0;
        }
    }
//    free(inter_arrival_list);
//    free(J_i_list);
    free(peer_data);
    return close_socket;
//    close(socket);
}

// this function is for tcp send
// return 1 if the socket should be closed
int tcp_send(int socket, int pktsize, int *left_pktnum, int pktnum, int pktrate, int stat, int using_select) {
    // if using select mode, send a pkt each time and left_pktnum--; if left_pktnum == 0 never stop
    if (using_select == 1) {
        pktnum = 1;
        if ((*left_pktnum) == 2) { // if the left_pknum == 2 send two packet ont time
            pktnum = 2;
            (*left_pktnum) -= 1;
        }
        (*left_pktnum)--;
        if ((*left_pktnum) == 0) {
            return 1;
        }
    }

    int message_length = pktsize;
    char *message = generate_message(message_length, -1);
    //printf("Message is %s", message);
    int p_num_index = 0;
    clock_t current_clock;
    clock_t previous_clock = clock();
    double cum_time_cost = 0;
    double cum_time_cost_session = 0;
    double cum_bytes_sent = 0;
    int cum_packet_number = 0;
    //if (stat == 0) single_iter_pkt_threshold
    double single_iter_pkt_threshold = pktrate * ((double) stat / 1000);
    while ((p_num_index < pktnum) || pktnum == 0) {
        //printf("server send once\n");
        if ((cum_bytes_sent < single_iter_pkt_threshold) || (single_iter_pkt_threshold == 0)) { // in Mbps
            int bytes_sent = 0;
            while (bytes_sent < message_length) {
                int ret = send(socket, message + bytes_sent, message_length - bytes_sent, 0);
                if (ret > 0) {
                    bytes_sent = bytes_sent + ret;
                } else {
//                    printf("Send stop function fail, with error : %d\n", ret);
                    //sleep(1);
                    return 1;
                }
            }
            p_num_index++;
            cum_packet_number++;
            cum_bytes_sent = cum_bytes_sent + bytes_sent;
        }
        // if the using_select == 1 means only the server only send a packet then go on select
        if (using_select == 1) break;
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) previous_clock) / CLOCKS_PER_SEC;
        previous_clock = current_clock;
        cum_time_cost = cum_time_cost + time_cost;
        cum_time_cost_session = cum_time_cost_session + time_cost;

        if (cum_time_cost * 1000 >= (double) stat) {
            double throughput = (cum_bytes_sent * 8) / (cum_time_cost * 1000000);
            printf("Sender: [Elapsed] %.2f ms, [Pkts] %d, [Rate] %.2f Mbps\n", cum_time_cost_session * 1000,
                   cum_packet_number, throughput);
            cum_packet_number = 0;
            cum_time_cost = 0;
            cum_bytes_sent = 0;
        }
    }
    free(message);
    return 2;

}

// this function is for udp send
// return 1 if the socket should be closed
int udp_send(int socket, int *p_num_index, int pktsize, int *left_pktnum, int pktnum, int pktrate, int stat,
             struct sockaddr_in target_address, int using_select) {
    //printf("using udp send\n");
    if (using_select == 1) {
        pktnum = 1;
        if ((*left_pktnum) == 2) { // if the left_pknum == 2 send two packet ont time
            pktnum = 2;
            (*left_pktnum) -= 1;
        }
        (*left_pktnum)--;
        if ((*left_pktnum) == 0) {
            return 1;
        }
    }
    clock_t current_clock;
    clock_t previous_clock = clock();
    double cum_time_cost = 0;
    double cum_time_cost_session = 0;
    double cum_bytes_sent = 0;
    int cum_packet_number = 0;
    //int p_num_index = 0;
    if (stat == 0) stat = 500;
    double single_iter_pkt_threshold = pktrate * ((double) stat / 1000);
    while ((*p_num_index < (*left_pktnum)) || pktnum == 0 || (pktnum == 1 && using_select == 1)) {
        if ((cum_bytes_sent < single_iter_pkt_threshold) || (single_iter_pkt_threshold == 0)) {
            char *message = generate_message(pktsize, *p_num_index);
            // printf("Message is %s\n", message);
            int sendto_flag = 1;
            while (sendto_flag) {
                int ret = sendto(socket, message, pktsize, 0, (struct sockaddr *) &target_address,
                                 sizeof(target_address));
                if (ret != -1) {
                    sendto_flag = 0;
                } else {
                    printf("Sendto fail, with error : %d\n", ret);
                }
            }
            //printf("Send message SN is %d\n", got_sequence_number(message, para_pktsize));
            free(message);
            (*p_num_index)++;
            cum_packet_number++;
            cum_bytes_sent = cum_bytes_sent + pktsize;
        }
        if (using_select == 1) return 2;
        current_clock = clock();
        double time_cost = ((double) current_clock - (double) previous_clock) / CLOCKS_PER_SEC;
        previous_clock = current_clock;
        cum_time_cost = cum_time_cost + time_cost;
        cum_time_cost_session = cum_time_cost_session + time_cost;
        if (cum_time_cost * 1000 >= (double) stat) {
            double throughput = (cum_bytes_sent * 8) / (cum_time_cost * 1000000);
            printf("Sender: [Elapsed] %.2f ms, [Pkts] %d, [Rate] %.2f Mbps\n", cum_time_cost_session * 1000,
                   cum_packet_number, throughput);
            cum_packet_number = 0;
            cum_time_cost = 0;
            cum_bytes_sent = 0;
        }
    }
    return 1;
}

// this function is for udp recv
// return 1 if the socket should be closed
int udp_recv(int socket, int pktsize, clock_t *previous_clock, double *inter_arrival_list, double *J_i_list,
             int *cum_packet_number, double *cum_time_cost, double *cum_bytes_recv, int stat,
             double *cum_time_cost_session, int *previous_SN, int *total_packet_loss, int using_select) {

    struct sockaddr_in client_address;
    clock_t current_clock;
    char *buf = (char *) malloc(pktsize * sizeof(char) + 1);
    memset(buf, 0, pktsize * sizeof(char) + 1);
    //memset((char*) &server_address, 0, sizeof(server_address));
    int recv_size = 0;
    socklen_t len = sizeof(client_address);
    int curren_SN = 0;
    while (true) {
        recv_size = recvfrom(socket, buf, pktsize, 0, (struct sockaddr *) &client_address, &len);
        // printf("recieve one \n");
        if (recv_size <= 0) {
            if (using_select == 1) return 1;
            printf("Recv failed with error code : %d\n", recv_size);
            break;
        } else {
//            printf("recv_size is %d", recv_size);
//            printf("data is %s\n", buf);
            curren_SN = get_sequence_number(buf);

            // printf("Received packet from %s:%d, SN: %d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), curren_SN);
            if (curren_SN - (*previous_SN) > 1) {
                *total_packet_loss = *total_packet_loss + (curren_SN - *previous_SN - 1);
            }
            *previous_SN = curren_SN;
            current_clock = clock();
            double time_cost = ((double) current_clock - (double) (*previous_clock)) / CLOCKS_PER_SEC;
            inter_arrival_list[*cum_packet_number] = time_cost * 1000;
            (*cum_packet_number)++;
            double D = calculate_average_value(inter_arrival_list, *cum_packet_number);
            J_i_list[*cum_packet_number - 1] = time_cost * 1000 - D;
            *previous_clock = current_clock;
            *cum_time_cost = *cum_time_cost + time_cost;
            *cum_time_cost_session = *cum_time_cost_session + time_cost;
            *cum_bytes_recv = *cum_bytes_recv + recv_size;
            if (stat == 0) stat = 500;
            if ((*cum_time_cost) * 1000 >= (double) stat) {
                double throughput = (*cum_bytes_recv * 8) / (*cum_time_cost * 1000000);
                double jitter = calculate_average_value(J_i_list, *cum_packet_number);
                //printf("jitter is")
                float loss_rate = float(*total_packet_loss) / float(*cum_packet_number + *total_packet_loss);
                if (using_select != 1) {
                    printf("Receiver: [Elapsed] %.2f ms, [Pkts] %d, Lost [%d, %.2f], [Rate] %.2f Mbps, [Jitter] %.6f ms\n",
                           *cum_time_cost_session * 1000, *cum_packet_number, *total_packet_loss, loss_rate, throughput,
                           jitter);
                }
//                printf("Receiver: [Elapsed] %.2f ms, [Pkts] %d, Lost [%d, %.2f], [Rate] %.2f Mbps, [Jitter] %.6f ms\n",
//                       *cum_time_cost_session * 1000, *cum_packet_number, *total_packet_loss, loss_rate, throughput,
//                       jitter);
                *cum_packet_number = 0;
                *cum_time_cost = 0;
                *cum_bytes_recv = 0;
                *total_packet_loss = 0;
            }
            if (using_select == 1) {
                free(buf);
                return 2;
                break;
            }
        }
    }
    free(buf);
    return 1;
}

// convert int to string
std::string my_int_to_string(int value) {
    std::ostringstream ss;
    ss << value;
    std::string new_string(ss.str());
    return new_string;

}

// convert the old string to fixed size string with prefix '0'
std::string add_zero(std::string old_string, int n_zero, char ch) {
    std::string new_string = std::string(n_zero - old_string.length(), ch) + old_string;
    return new_string;
}

// init the win sock
int InitializeWinsock() {
#ifdef _WIN32
    WORD wVersionRequested = MAKEWORD(2, 2); // WinSock version, which is 2.2
    WSADATA wsaData;
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WinSock Initialization Fail\n");
        printf("Error number: %d\n", err);
        return 0;
    }
    if (LOBYTE(wsaData.wVersion) < 2 || HIBYTE(wsaData.wVersion) < 2) {
        printf("winsock.dll is not available in your system\n");
        WSACleanup();
        return 0;
    }
    return 1;
#endif
}

