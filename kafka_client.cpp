#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdint>

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

	if (inet_pton(AF_INET, serverIP, &server.sin_addr) == 0) {
		close(clientFD);
		std::cerr << "Error reading IP Address" << std::endl;
		return -1;
	}

	int connectionResult = connect(clientFD, (sockaddr*) &server, sizeof(server));
	if (connectionResult == -1) {
		close(clientFD);
		std::cerr << "Error connecting client with server." << std::endl;
		return 1;
	}

	std::cout << "Successfully connected with server.\n";
	int32_t messageSize = htonl(8);
	int32_t correlationId = htonl(7);
	send(clientFD, &messageSize, sizeof(messageSize), 0);
	send(clientFD, &correlationId, sizeof(correlationId), 0);

	close(clientFD);
	return 0;

}
