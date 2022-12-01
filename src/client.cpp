#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// \\Mac\Home\Documents\GitHub\IERG4180_Project1\NetProbe\x64\Debug
// \\Mac\Home\Documents\GitHub\example
#undef UNICODE

# include "client.h"
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/pem.h>
# include <openssl/x509.h>
# include <openssl/x509_vfy.h>
int main(int argc, char **argv) {
    if (argc < 1) {
        printf("your input arguments should be more\n");
        return 0;
    }
    Client_argument client_argument;
    argument_parse_client(argc, argv, &client_argument);
    client(argc, argv);
    return 0;
}

// this function is for client
int client(int argc, char **argv) {
    Client_argument client_argument;
    argument_parse_client(argc, argv, &client_argument);
    printf("url is %s\n", client_argument.url);
    printf("filename is %s\n", client_argument.fileName);
    printf("\n\n\n");
    char                dest_url[8192];
    X509                *cert = NULL;
    X509_NAME           *certName = NULL;
    const SSL_METHOD    *method;
    SSL_CTX             *ctx;
    SSL                 *ssl;
    int server = 0;
    int ret, i;
    char * ptr = NULL;
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    if (SSL_library_init() < 0) {
        printf("Could not initialize the OpenSSL library !\n");
    }
    method = TLS_method();
    if ((ctx = SSL_CTX_new(method)) == NULL) {
        printf("Unable to create a new SSL context structure.\n");
    }
    SSL_CTX_load_verify_locations(ctx, 0, "/etc/ssl/certs");
    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        printf("Unable to set default verify paths.\n");
    }
    X509_STORE * xStore;
    xStore = SSL_CTX_get_cert_store(ctx);
//    SSL_CTX_set_default_verify_store(ctx);
//    SSL_CTX_set_default_verify_file(ctx);
    ssl = SSL_new(ctx);
    server = create_socket(client_argument.url);
    SSL_set_fd(ssl, server);
    if (server != 0) {
        printf("Successfully made the tcp connection to %s\n", client_argument.url);
    }
    SSL_set_tlsext_host_name(ssl, client_argument.url);

    if (SSL_connect(ssl) != 1) {
        printf("Error: Could not build a SSL session to %s.\n", client_argument.url);
    }
    else {
        printf("Successfully enabled  a SSL/TLS session to %s.\n", client_argument.url);
    }
    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
        printf("Error: Could not get a certificate from %s.\n", client_argument.url);
    }
    else {
        printf("Retrieved the server's certificate from %s.\n", client_argument.url);
    }
    ret = SSL_get_verify_result(ssl);
    if (ret != X509_V_OK) {
        printf("Warning : Validation failed for certificate from %s. res is %d\n", client_argument.url, ret);
    }
    else {
        printf("Successfully Validated the server certificate from %s.\n", client_argument.url);
    }
    ret = X509_check_host(cert, client_argument.url, strlen(client_argument.url), 0, &ptr);
    if (ret == 1) {
        printf("Successfully validated the server's hostname matched to %s.\n", ptr);
        OPENSSL_free(ptr);
    }
    else if (ret == 0) {
        printf("Server's hostname validation validation %s. ret is %d\n", client_argument.url, ret);
    }
    else {
        printf("Hostname validation internal error %s. ret is %d\n", client_argument.url, ret);
    }


    return 0;

}

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
    client_argument->url = (char *) "https://www.mclab.org";
    client_argument->https = 0;
    client_argument->fileName = (char *)"filename";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-send") == 0) {
            client_argument->mode = 0;
            continue;
        }
        else if (strcmp(argv[i], "-recv") == 0) {
            client_argument->mode = 1;
            continue;
        }
        else if (strcmp(argv[i], "-response") == 0) {
            client_argument->mode = 2;
            client_argument->pktrate = 10;
            continue;
        }
        else if (i == 1) {
            int index = 0;
            char* domainName = (char *)malloc(30 *sizeof (char));
            memset(domainName, '\0', 30 * sizeof(char));
            if (argv[i][4] == 's') {
                client_argument->https = 1;
                int k = 8;
                while (argv[i][k] != '\0') {
                    domainName[index] = argv[i][k];
                    k++;
                    index++;
                }
            }
            else {
                client_argument->https = 0;
                int k = 7;
                while (argv[i][k] != '\0') {
                    domainName[index] = argv[i][k];
                    k++;
                    index++;
                }
            }
//            strncpy(client_argument->url, domainName, index);
            client_argument->url = domainName;
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
            }else if (strcmp(argv[i], "-file") == 0) {
                    client_argument->fileName = argv[i + 1];
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
// this function is for print the prompt info (using -h)
void print_prompt_information_server() {
    printf("welcome to use the NetProbe\n");
    printf("-send means sending mode \n-recv means receiving mode\n-response means response time mode \n");
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
    int sockfd;
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
        printf("clock is %f %fs\n",(double)previous_clock, ((double)previous_clock) / CLOCKS_PER_SEC);
        double * inter_arrival_list = (double*) malloc(Jitter_max * sizeof(double));
        double * J_i_list = (double*) malloc(Jitter_max * sizeof(double));
        int cum_packet_number = 0;
        double cum_time_cost = 0;
        double cum_bytes_recv = 0;
        double cum_time_cost_session = 0;
        int closing_socket = tcp_recv(sockfd, client_argument.pktsize, &previous_clock, inter_arrival_list, J_i_list,
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
#ifdef __linux__
        int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        //printf("server send udp in linux\n");
#elif _WIN32
        //printf("server send udp in windows\n");
        SOCKET new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);;
#endif
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
    if (client_argument.mode == 2 && client_argument.proto == 0) { // the client sends response to server using udp
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
#ifdef __linux__
        int new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
#elif _WIN32
        SOCKET new_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);;
#endif
        struct sockaddr_in new_udp_socket_client_address, server_address;
        memset(&new_udp_socket_client_address, 0, sizeof(struct sockaddr_in));
        memset(&server_address, 0, sizeof(struct sockaddr_in));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(atoi(new_port_buf));
        char server_ip[100];
        solve_hostname_to_ip_linux(client_argument.rhost, server_ip);
        printf("server ip is %s\n",server_ip);
        printf("server port is %d\n",atoi(new_port_buf));
        server_address.sin_addr.s_addr = inet_addr(server_ip);
        new_udp_socket_client_address.sin_family = AF_INET;
        new_udp_socket_client_address.sin_port = htons(new_port);

        char * my_ip = (char *)malloc(15 *sizeof(char *));
        get_ip(my_ip);
        printf("ip is %s\n",my_ip);
        printf("port is %d\n",new_port);
        new_udp_socket_client_address.sin_addr.s_addr = inet_addr(my_ip);
        SetReceiveBufferSizeLinux(new_udp_socket, 65533);
        bind(new_udp_socket, (struct sockaddr*)&new_udp_socket_client_address, sizeof(new_udp_socket_client_address));
        client_response_udp(new_udp_socket,new_udp_socket_client_address, client_argument, server_address);

    }
    // the client sends response to server using tcp , client send tcp to a new port
    if (client_argument.mode == 2 && client_argument.proto == 1) { // the client sends response to server using tcp
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
        int new_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in new_tcp_response_server_address;
        memset(&new_tcp_response_server_address, 0, sizeof(struct sockaddr_in));
        new_tcp_response_server_address.sin_family = AF_INET;
        new_tcp_response_server_address.sin_port = htons(port);
        new_tcp_response_server_address.sin_addr.s_addr = servaddr.sin_addr.s_addr;
        client_response_tcp(new_tcp_response_server_address, client_argument);
#ifdef __linux__
//            close(new_tcp_socket);
            close(sockfd);
#elif _WIN32
            closesocket(sockfd);
//            closesocket(new_udp_socket);
#endif
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


// this function is for create a socket for connect server
int create_socket(char* url) {

    int sockfd;
    int port = 443;
    struct sockaddr_in dest_addr;
    char* ip = (char*)malloc(20 * sizeof(char));
    solve_hostname_to_ip_linux(url, ip);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    if (inet_aton(ip, (struct in_addr *) &dest_addr.sin_addr.s_addr) == 0)
    {
        perror("Socket Init Fail!");
        exit(errno);
    }
    printf("Socket Created\n");
    if (connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) != 0)
    {
        perror("Socket Connect Fail!");
        exit(errno);
    }
    printf("Socket Connected\n");
    return sockfd;
}
