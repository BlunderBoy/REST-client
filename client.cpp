#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include "helpers.hpp"
#include "json.hpp"
#include "requests.h"
#include <time.h>

using namespace std; //i'm biting the bullet this time
using namespace nlohmann; //yep this too, scary stuff

#define null NULL //nu se poate fara dupa java

class Sesiune{
    public:
    string cookie;
    string JWTtoken;
    bool isLoggedIn = false;
    bool gotToken = false;
};

void preety_print(string raspuns_de_la_server_care_arata_foarte_urat)
{
    cout << "--------------Raspuns---------------" << endl;
    int pozitie;
    string token;
    string newtoken;

    //header
    cout << "Header: " << endl; 
    pozitie = raspuns_de_la_server_care_arata_foarte_urat.find("\n");
    token = raspuns_de_la_server_care_arata_foarte_urat.substr(0, pozitie);
    cout << token << endl;

    //maybe cookies!!!!
    pozitie = raspuns_de_la_server_care_arata_foarte_urat.find("Set-Cookie");
    if(pozitie != -1)  
    {
         cout << "Cookie:" << endl; 
        token = raspuns_de_la_server_care_arata_foarte_urat.substr(pozitie, raspuns_de_la_server_care_arata_foarte_urat.size());
        int pozitie_endl = token.find(";");
        newtoken = token.substr(0, pozitie_endl);
        cout << newtoken << endl;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
    }

    //maybe some json
    pozitie = raspuns_de_la_server_care_arata_foarte_urat.find("[");
    if(pozitie != -1) //e lista de obiecte
    {
         cout << "JSON:" << endl; 
        token = raspuns_de_la_server_care_arata_foarte_urat.substr(pozitie, raspuns_de_la_server_care_arata_foarte_urat.size());
        json printer = json::parse(token);
        cout << printer.dump(2) << endl;
    }
    else //poate e un singur obiect sau nu e nimic
    {
        pozitie = raspuns_de_la_server_care_arata_foarte_urat.find("{");
        if(pozitie != -1) //e un singur obiect sau nu e nimic
        {
             cout << "JSON:" << endl; 
            token = raspuns_de_la_server_care_arata_foarte_urat.substr(pozitie, raspuns_de_la_server_care_arata_foarte_urat.size());
            json printer = json::parse(token);
            cout << printer.dump(2) << endl;
        }
    }
    cout << "------------------------------------"<<endl<<endl;
}

/*returneaza un string care reprezinta ip-ul serverului
printr-un DNS request */
string dnsLookup(string nume)
{
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
  	hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(nume.c_str(), NULL, &hints, &result);
    struct sockaddr *sal = ((struct sockaddr*)result->ai_addr);
	struct sockaddr_in *salut = (struct sockaddr_in *)sal;
    return inet_ntoa(salut->sin_addr);
}
/* ca sa nu ma car dupa mine cu asta prin argumente */
string ip = dnsLookup("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"); 

string compute_post_request(string host, string url, string content_type, json data, string cookie, string auth)
{
    string message;
    char* line = (char*)malloc(BUFLEN);
    //adaugam fisierul
    sprintf(line, "POST %s HTTP/1.1", url.c_str());
    compute_message(&message, line);
    
    //adaugam hostul
    message = message + "HOST: ";
    compute_message(&message, host);

    
    //content type si content-length
    message += "Content-Type: ";
    compute_message(&message, content_type);

    message += "Content-Length: ";
    compute_message(&message, to_string(data.dump().length()));
    
    //autorizatii
    if(auth.size() != 0)
    {
        message += "Authorization: Bearer ";
        compute_message(&message, auth);
    }

    // add cookies
    if (cookie.length() != 0) {
       message += "Cookie: ";
       compute_message(&message, cookie);
    }
    
    //finalul requestului
    compute_message(&message, std::string(""));

    //datele pe care le postuim
    message = message + data.dump();

    return message;
}

string compute_get_request(string host, string url, string cookie, string auth)
{
    string message;
    char* line = (char*)malloc(BUFLEN);
    //adaugam fisierul
    sprintf(line, "GET %s HTTP/1.1", url.c_str());
    compute_message(&message, line);
    
    //adaugam hostul
    message = message + "HOST: ";
    compute_message(&message, host);

    //autorizatii
    if(auth.size() != 0)
    {
        message += "Authorization: Bearer ";
        compute_message(&message, auth);
    }
    
    // add cookies
    if (cookie.length() != 0) {
       message += "Cookie: ";
       compute_message(&message, cookie);
    }
    
    //finalul requestului
    compute_message(&message, std::string(""));

    return message;
}

string compute_delete_request(string host, string url, string cookie, string auth)
{
    string message;
    char* line = (char*)malloc(BUFLEN);
    //adaugam fisierul
    sprintf(line, "DELETE %s HTTP/1.1", url.c_str());
    compute_message(&message, line);
    
    //adaugam hostul
    message = message + "HOST: ";
    compute_message(&message, host);

    //autorizatii
    if(auth.size() != 0)
    {
        message += "Authorization: Bearer ";
        compute_message(&message, auth);
    }
    
    // add cookies
    if (cookie.length() != 0) {
       message += "Cookie: ";
       compute_message(&message, cookie);
    }
    
    //finalul requestului
    compute_message(&message, std::string(""));

    return message;
}


void register_client(Sesiune &sesiune)
{
    int sockfd;
    string username, password;
    json send;
    string a;

    cout << "username : ";
    cin >> username;

    cout << "password : ";
    cin >> password;

    send["password"] = password;
    send["username"] = username;

    cout<< "Payload trimis:" << endl;
    cout<<send.dump(2);
    cout<<endl;

    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        "/api/v1/tema/auth/register", "application/json", 
                                        send, sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}

void login(Sesiune &sesiune)
{
    int sockfd;
    string username, password;
    json send;
    string a;

    cout << "username : ";
    cin >> username;

    cout << "password : ";
    cin >> password;

    send["password"] = password;
    send["username"] = username;

    cout<< "Payload trimis:" << endl;
    cout<<send.dump(2);
    cout<<endl;

    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        "/api/v1/tema/auth/login", "application/json", 
                                        send, sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);

    //setare cookie sesiune
    const char *aux = strstr(raspuns.c_str(), "connect.sid");
    if(aux != null){
        char *token = strtok((char*)aux, ";");
        sesiune.cookie = string(token);
        sesiune.isLoggedIn = true;
    }
}

void enter_library(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first." << endl;
        return;
    }


    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        "/api/v1/tema/library/access", sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);

    const char *aux = strstr(raspuns.c_str(), "{");
    json getToken = json::parse(aux);
    sesiune.JWTtoken = getToken["token"];
    sesiune.gotToken = true;
}

void get_books(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first." << endl;
        return;
    }

    if(sesiune.gotToken == false)
    {
        cout << "You have to get a token first." << endl;
        return;
    }


    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        "/api/v1/tema/library/books", sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}

void get_book(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first." << endl;
        return;
    }

    if(sesiune.gotToken == false)
    {
        cout << "You have to get a token first." << endl;
        return;
    }

    string id;
    cout<< "Care este id-ul cartii? ";
    cin >> id;


    string carte("/api/v1/tema/library/books/");
    carte += id;

    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        carte, sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}

void delete_book(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first.." << endl;
        return;
    }

    if(sesiune.gotToken == false)
    {
        cout << "You have to get a token first." << endl;
        return;
    }

    string id;
    cout<< "Care este id-ul cartii pe care o stergeti? ";
    cin >> id;


    string carte("/api/v1/tema/library/books/");
    carte += id;

    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        carte, sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}

void add_book(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first.." << endl;
        return;
    }

    if(sesiune.gotToken == false)
    {
        cout << "You have to get a token first." << endl;
        return;
    }

    json data;
    string buffer;

    //title
    cout << "title=";
    cin >> buffer;
    data["title"] = buffer;

    //author
    cout << "author=";
    cin >> buffer;
    data["author"] = buffer;

    //genre
    cout << "genre=";
    cin >> buffer;
    data["genre"] = buffer;

    //publisher
    cout << "publisher=";
    cin >> buffer;
    data["publisher"] = buffer;

    //page_count
    cout << "page_count=";
    cin >> buffer;
    data["page_count"] = buffer;

    cout<< "Payload trimis:" << endl;
    cout<<data.dump(2);
    cout<<endl;

    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        "/api/v1/tema/library/books", "application/json", data, sesiune.cookie, sesiune.JWTtoken);
    cout << mesaj << endl;
    send_to_server(sockfd, mesaj);
    
    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}

void logout(Sesiune &sesiune)
{
    int sockfd;
    if(sesiune.isLoggedIn == false)
    {
        cout << "Log in first." << endl;
        return;
    }



    sockfd = open_connection(ip, 8080, AF_INET, SOCK_STREAM, 0);
    string mesaj = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
                                        " /api/v1/tema/auth/logout", sesiune.cookie, sesiune.JWTtoken);
    //cout << mesaj << endl;
    send_to_server(sockfd, mesaj);

    string raspuns = receive_from_server(sockfd);
    //cout << endl << raspuns << endl;
    preety_print(raspuns);
}


int main(int argc, char *argv[])
{
    Sesiune sesiune;
    char buffer_citire[100];

    while(1)
    {
        cin >> buffer_citire;
        
        if(strcmp(buffer_citire, "exit") == 0)
        {
            return 0;
        }

        if(strcmp(buffer_citire, "register") == 0)
        {
            register_client(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "login") == 0)
        {
            login(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "enter_library") == 0)
        {
            enter_library(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "get_books") == 0)
        {
            get_books(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "get_book") == 0)
        {
            get_book(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "add_book") == 0)
        {
            add_book(sesiune);
            continue;
        }

       if(strcmp(buffer_citire, "delete_book") == 0)
        {
            delete_book(sesiune);
            continue;
        }

        if(strcmp(buffer_citire, "logout") == 0)
        {
            logout(sesiune);
            continue;
        }

        cout<< "Nu ai introdus comanda bine." <<endl;
    }
    return -1;
}
