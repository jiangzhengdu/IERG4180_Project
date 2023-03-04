# IERG4180_Project4 README

This is for IERG 4180 Project4, which is also the final version for IEGR4180. I really learn a lot from this course. I am not fear the socket programming anymore (i think) after this course. And this course is also one of my best courses during the first term in CUHK.

## About the project4

### How to build and run it

Frist, go into the src directory, do the following command, which requires the Cmake and g++ compiler.

`make clean && make`

Then the /src folder will generate two executable program, one is server, the other is client.

#### Client side

This client is a kind of simulator for http and https. The optional running command is following

`./client https://www.google.com`

`./client https://localhost:4081`

`./client https://www.google.com -file google.html`

`./client https://localhost:4081/a.txt -file client_a.txt`

#### Server Side

This server's purpose is to provide https and http service for client. It only support 404 and 200 request currently.The following is server executable command:

`./server -stat 3000 -lhttpport 4080 -lhttpsport 4081 -server threadpool -poolsize 8`

`./server -stat 3000`

### Implementation Details

#### About server

The server support two server model: threadpool and single thread. I use the threadpool model in former project3. If the http file request is contained in http request header, then the server will look throught the directory , if the file exists then send back to the client as http response body. If the file not exist, then return http respinse with code 404.

#### About client

The client genereat http&& https request to server, then receive the response from server. Save the content into file or print them out. One hints is the dividing symbol for HTTP header and HTTP body is __\r\n\r\n__. The saved file shoudn't contain the HTTP response header.

#### About HTTPS

The both client and server have implement the HPPTS. First, the server should gain the certificate by CA. I use openssl tools to generate self-signed CA and server certificate. The command is listed in file generateCert.txt. The version of openssl is 3. The client store the CA certificate in its store. The server send its certificate throuth SSL tunel.The client also check the certificate domain name.

### Experience

#### Httperf

We use Httperf tool to check our server.

The install command:

`sudo apt-get install httperf`

Use httperf to send a request

`httperf --server localhost --port 4080`

Use httperf to send multi request

`httperf --server localhost --port 4080 --num-conns 10 --rate 1 --timeout 5`
