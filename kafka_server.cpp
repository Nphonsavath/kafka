#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <vector>
#include <cstdint>
#include <cstring>

#include "kafka_protocol.h"

struct APIVersionsResponse {
	int32_t messageSize;
	int32_t correlationId;
	int16_t errorCode;
};

void convertKafkaHeaderNTOH(v2KafkaRequestHeader& header) {
	header.messageSize = ntohl(header.messageSize);
	header.requestAPIKey = ntohs(header.requestAPIKey);
	header.requestAPIVersion = ntohs(header.requestAPIVersion);
	header.correlationId = ntohl(header.correlationId);
	return;
}
int main(int argc, char* argv[]) {
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

	int expectedMessageLength = 0;
	int totalReadBytes = 0;
	recv(clientFD, &expectedMessageLength, sizeof(expectedMessageLength), 0);
	expectedMessageLength = ntohl(expectedMessageLength);
	//recv(clientFD, &totalReadBytes, sizeof(totalReadBytes), 0);
	//std::cout << ntohl(totalReadBytes) << std::endl;
	std::cout << expectedMessageLength << std::endl;
	std::vector<char> buffer(expectedMessageLength);
	while (totalReadBytes < expectedMessageLength) {
		int currentReadBytes = recv(clientFD, buffer.data() + totalReadBytes, expectedMessageLength - totalReadBytes, 0);
		std::cout << "Currentreadbytes: " << currentReadBytes << std::endl;
		std::cout << "Totalreadbytes: " << totalReadBytes << std::endl;
		if (currentReadBytes == -1) {
			std::cout << "Error reading data" << std::endl;
			return 1;
		} else if (currentReadBytes == 0) {
			std::cout << "Completed reading data" << std::endl;
			break;
		}
		totalReadBytes += currentReadBytes;
	}
	
	v2KafkaRequestHeader header;
	memcpy(&header, buffer.data(), sizeof(header));
	convertKafkaHeaderNTOH(header);	
	std::cout << "CorrelationId received: " << header.correlationId << std::endl;
	std::cout << "RequestAPIVersion received: " << header.requestAPIVersion << std::endl;
	
	APIVersionsResponse response;
	response.messageSize = htonl(sizeof(response));
	response.correlationId = htonl(header.correlationId);
	if (header.requestAPIVersion < 0 || header.requestAPIVersion > 4) {
		response.errorCode = htons(UNSUPPORTED_VERSION); 
	} else {
		response.errorCode = htons(0);
	}
	send(clientFD, &response.messageSize, sizeof(response.messageSize), 0);
	send(clientFD, &response, sizeof(response), 0);

	close(clientFD);	
	close(serverFD);
	return 0;
}
