#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <vector>
#include <cstdint>
#include <cstring>

#include "kafka_protocol.hpp"
#include "request.hpp"
#include "response.hpp"

void convertKafkaHeaderNTOH(kafkaRequestHeaderV2& header) {
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

	Request request(clientFD);
	request.toString();


	/*for (int i = 0; i < totalReadBytes; i++) {
    		if (i != totalReadBytes - 1) {
			std::cout << std::hex
              << (static_cast<int>(static_cast<unsigned char>(buffer[i])) & 0xFF) << " ";
		} else {
			std::cout << std::hex
              << (static_cast<int>(static_cast<unsigned char>(buffer[i])) & 0x0F) << " ";
		}
	}*/

	//std::cout << std::endl;

	std::vector<char> header;
	int32_t correlationId = htonl(request.getCorrelationId());
	header.insert(header.end(), 
			reinterpret_cast<char*>(&correlationId), 
			reinterpret_cast<char*>(&correlationId) + sizeof(correlationId));

	int32_t totalMessageSize = htonl(header.size());
	std::cout << ntohl(totalMessageSize) << std::endl;
	send(clientFD, &totalMessageSize, sizeof(totalMessageSize), 0);
	send(clientFD, header.data(), header.size(), 0);
	//kafkaRequestHeaderV2 header;
	//memcpy(&header, buffer.data(), sizeof(header));
	//convertKafkaHeaderNTOH(header);	
	//std::cout << "CorrelationId received: " << header.correlationId << std::endl;
	//std::cout << "RequestAPIVersion received: " << header.requestAPIVersion << std::endl;
	/*
	APIVersionsResponseBodyV4 response;
	response.messageSize = htonl(sizeof(response));
	response.correlationId = htonl(header.correlationId);
	if (header.requestAPIVersion < 0 || header.requestAPIVersion > 4) {
		response.errorCode = htons(UNSUPPORTED_VERSION); 
	} else {
		response.errorCode = htons(0);
	}

	int messageSize = sizeof(response);	
	send(clientFD, &messageSize, sizeof(messageSize), 0);
	send(clientFD, &response, sizeof(response), 0);

	*/
	close(clientFD);	
	close(serverFD);
	return 0;
}
