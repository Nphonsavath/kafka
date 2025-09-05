#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstring>
#include <unordered_map>

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
	
	int32_t messageSize = htonl(0);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&messageSize), 
			reinterpret_cast<char*>(&messageSize) + sizeof(messageSize));
	

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

	std::string headerClientId = "kafka-cli";
    	int16_t headerClientIdLength = htons(static_cast<int16_t>(headerClientId.size()));
    	header.insert(header.end(), 
			reinterpret_cast<char*>(&headerClientIdLength), 
			reinterpret_cast<char*>(&headerClientIdLength) + sizeof(headerClientIdLength));
    	header.insert(header.end(),
			headerClientId.begin(),
			headerClientId.end());
				  
	APIVersionRequestBodyV4 body {
		"kafka-cli",
		"0.1",
		0
	};

	int8_t bodyClientIdLength = static_cast<int8_t>(body.clientIdCompact.size());
	header.insert(header.end(),
			reinterpret_cast<char*>(&bodyClientIdLength),
			reinterpret_cast<char*>(&bodyClientIdLength) + sizeof(bodyClientIdLength));
	header.insert(header.end(),
			body.clientIdCompact.begin(),
			body.clientIdCompact.end());

	int8_t bodyClientIdSoftwareVerLength = static_cast<int8_t>(body.clientIdSoftwareVerCompact.size());
	header.insert(header.end(),
			reinterpret_cast<char*>(&bodyClientIdSoftwareVerLength),
			reinterpret_cast<char*>(&bodyClientIdSoftwareVerLength) + sizeof(bodyClientIdSoftwareVerLength));
	header.insert(header.end(),
			body.clientIdSoftwareVerCompact.begin(),
			body.clientIdSoftwareVerCompact.end());

	header.insert(header.end(),
			reinterpret_cast<char*>(&body.tagBuffer),
			reinterpret_cast<char*>(&body.tagBuffer) + sizeof(body.tagBuffer));

	int32_t totalMessageSize = htonl(header.size());
	std::cout << ntohl(totalMessageSize) << std::endl;
	memcpy(header.data(), &totalMessageSize, sizeof(totalMessageSize));	
	//std::cout << "totalMessageSize: " << totalMessageSize << std::endl;
	//Request request(header);

	//std::cout << sizeof(request) << std::endl;	
	//int messageSize = htonl(sizeof(request));
	//std::cout << "MessageSize: " << messageSize << std::endl;
	//send(clientFD, &totalMessageSize, sizeof(totalMessageSize), 0);
	send(clientFD, header.data(), header.size(), 0);
	std::unordered_map<int32_t, int16_t> correlationToAPIKey;
	correlationToAPIKey[correlationId] = requestAPIKey;

	std::vector<char> responseBuffer = Response::readResponse(clientFD);
	/*	
	int expectedMessageLength = 0;
	int totalReadBytes = 0;
	if (recv(clientFD, &expectedMessageLength, sizeof(expectedMessageLength), 0) == -1) {
		std::cerr << "Error reading from client fd" << std::endl;
		return 1;	
	}
	expectedMessageLength = ntohl(expectedMessageLength);
	totalReadBytes += sizeof(expectedMessageLength);

	std::cout << "Expected message length: " << expectedMessageLength << std::endl;
	std::vector<char> buffer(expectedMessageLength);
	while (totalReadBytes < expectedMessageLength) {
		int currentReadBytes = recv(clientFD, 
				buffer.data() + totalReadBytes - sizeof(expectedMessageLength), 
				expectedMessageLength - totalReadBytes, 
				0);
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
	*/

	Response response(responseBuffer);
	int16_t responseAPIKey = correlationToAPIKey[response.getCorrelationId()];
	response.parseResponse(responseBuffer, responseAPIKey);	
	//if (response.correlationId == 18) {
//
//	}
	response.toString();

	close(clientFD);
	return 0;

}
