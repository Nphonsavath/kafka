#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc, char* argv[]) {

	if (argc < 3) {
		std::cerr << "Usage ./client <IPv4 addresse> <port #>" << std::endl;
		return 1;
	}

	char* serverIP = argv[1];
	int serverPort = std::atoi(argv[2]);

	int clientFD = socket(AF_INET, SOCK_STREAM, 0);
	if (clientFD  == -1) {
		std::cerr << "Error creating client socket." << std::endl;
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);

	
}
