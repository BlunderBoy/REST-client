#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#include<string>

// shows the current error
void error(const char *msg);

//facem mesajul aici
void compute_message(std::string *message, std::string line);
void compute_message(std::string *message, char* line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(std::string host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, std::string message);

// receives and returns the message from a server
std::string receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif
