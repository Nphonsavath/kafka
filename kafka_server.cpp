#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
	//Create TCP socket using IPv4. 
	int serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD  == -1) {
		std::cerr << "Error creating server socket." << std::endl;
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(9092); //default port # for kafka
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFD, (struct sockaddr*) &server, sizeof(server)) == -1) {
		close(serverFD);
		std::cerr << "Error binding server socket to 9092." << std::endl;
		return 1;
	}

	socklen_t serverLength = sizeof(server);
	if (getsockname(serverFD, (struct sockaddr*) &server, &serverLength) == -1) {
		close(serverFD);
		std::cerr << "Error retrieving socket address." << std::endl;
		return -1;
	}

	char ipAsString[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(server.sin_addr), ipAsString, INET_ADDRSTRLEN);
	std::cout << "IP Adress: " << ipAsString << ", Port #: " << ntohs(server.sin_port) << std::endl;

	int maxConnectionRequests = 5;
	if (listen(serverFD, maxConnectionRequests) == -1) {
		close(serverFD);
		std::cerr << "Error listening for connections." << std::endl;
		return 1;	
	}

	struct sockaddr_in client;
	socklen_t clientLength = sizeof(client);
	int clientFD = accept(serverFD, (struct sockaddr*) &client, &clientLength); 
	if (clientFD == -1) {
		close(serverFD);
		std::cerr << "Error connecting client with server." << std::endl;
		return 1;
	}

	std::cout << "Client connected\n";
	close(clientFD);
	close(serverFD);
	return 0;
}
