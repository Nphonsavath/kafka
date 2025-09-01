#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstring>

#include "kafka_protocol.hpp"
#include "request.hpp"
#include "response.hpp"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage ./client <IPv4 address> <port #>" << std::endl;
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
	
	std::vector<char> header;
	
	/*int32_t messageSize = htonl(0);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&messageSize), 
			reinterpret_cast<char*>(&messageSize) + sizeof(messageSize));
	*/

	int16_t requestAPIKey = htons(18);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&requestAPIKey), 
			reinterpret_cast<char*>(&requestAPIKey) + sizeof(requestAPIKey));

	int16_t requestAPIVersion = htons(4);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&requestAPIVersion), 
			reinterpret_cast<char*>(&requestAPIVersion) + sizeof(requestAPIVersion));

	int32_t correlationId = htonl(7);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&correlationId), 
			reinterpret_cast<char*>(&correlationId) + sizeof(correlationId));

	std::string clientId = "kafka-cli";
    	int16_t clientIdLength = htons(static_cast<int16_t>(clientId.size()));
    	header.insert(header.end(), 
			reinterpret_cast<char*>(&clientIdLength), 
			reinterpret_cast<char*>(&clientIdLength) + sizeof(clientIdLength));
    	header.insert(header.end(),
			clientId.begin(),
			clientId.end());
				  
	//std::string clientIdNullable = "09kafka-cli";
	//header.insert(header.end(), reinterpret_cast<char*>(&clientIdNullable), reinterpret_cast<char*>(&clientIdNullable) + sizeof(clientIdNullable));

	int32_t totalMessageSize = htonl(header.size());
	std::cout << ntohl(totalMessageSize) << std::endl;
	//memcpy(header.data(), &totalMessageSize, sizeof(totalMessageSize));	
	//std::cout << "totalMessageSize: " << totalMessageSize << std::endl;
	Request request(header);

	//std::cout << sizeof(request) << std::endl;	
	//int messageSize = htonl(sizeof(request));
	//std::cout << "MessageSize: " << messageSize << std::endl;
	send(clientFD, &totalMessageSize, sizeof(totalMessageSize), 0);
	std::cout << sizeof(request) << std::endl;
	send(clientFD, header.data(), header.size(), 0);

	
	int expectedMessageLength = 0;
	int totalReadBytes = 0;
	recv(clientFD, &expectedMessageLength, sizeof(expectedMessageLength), 0);
	expectedMessageLength = ntohl(expectedMessageLength);
	
	std::cout << "Expected message length: " << expectedMessageLength << std::endl;
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

	Response response(buffer);
	response.toString();

	close(clientFD);
	return 0;

}
