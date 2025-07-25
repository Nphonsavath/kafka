#include<iostream>
#include <sys/socket.h>



int main() {
    //Create TCP socket using IPv4. 
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD  == -1) {
        std::cerr << "Error creating server socket. " << std::endl;
        return 1;
    }

    struct sockadder_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(9092); //default port # for kafka
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverFD, (struct sockaddr*) &server), sizeof(server)) == -1) {
        close(serverFD);
        std::cerr << "Error binding server socket to 9092" << std::endl;
        return 1;
    }

    if (getsockname(serverFD, (struct sockaddr* &server), sizeof(server)) == -1-) {
        close(serverFD);
        std::cerr << "Error retrieving socket address" << std::endl;
    }

    std::cout << "IP Adress: " << server.sin_addr << ", Port #: " << server.sin_port << std::endl;
    
    int maxConnectionRequests = 5;
    
}