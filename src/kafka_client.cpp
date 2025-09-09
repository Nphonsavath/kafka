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

int parseArgs(const int numArgs, const char* argv[], std::string& serverIP, int& serverPort) {
	if (numArgs < 3) {
		std::cerr << "Usage ./client <IPv4 address> <port #>" << std::endl;
		return -1;
	}

	serverIP = argv[1];
	serverPort = std::atoi(argv[2]);
	return 0;
}

int createSocket() {
	int clientFD = socket(AF_INET, SOCK_STREAM, 0);
	if (clientFD  == -1) {
		std::cerr << "Error creating socket." << std::endl;
		return -1;
	}
	return clientFD;
}

bool connectSocket(int clientFD, const std::string& serverIP, int serverPort) {
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);

	int networkConversionResult = inet_pton(AF_INET, serverIP.c_str(), &server.sin_addr); 
	if (networkConversionResult == 0) {
		std::cerr << "Error reading IP Address" << std::endl;
		return false;
	} else if (networkConversionResult == -1) {
		std::cerr << "Error af does not contain a valid address family" << std::endl;
		return false;
	}	

	int connectionResult = connect(clientFD, (sockaddr*) &server, sizeof(server));
	if (connectionResult == -1) {
		std::cerr << "Error connecting client with server." << std::endl;
		return false;
	}

	std::cout << "Successfully connected with server.\n";
	return true;
}

int main(int argc, char* argv[]) {
	std::string serverIP;
	int serverPort;
	if (parseArgs(argc, argv, serverIP, serverPort) == -1) { 
		return -1; 
	}

	int clientFD = createSocket();
	if (clientFD == -1) { return -1; }	

	if (!connectSocket(clientFD, serverIP, serverPort)) { 
		close(clientFD);
		return -1; 
	}
		
	std::vector<char> header;
	
	int32_t messageSize = htonl(0);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&messageSize), 
			reinterpret_cast<char*>(&messageSize) + sizeof(messageSize));
	

	int16_t requestAPIKey = htons(18);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&requestAPIKey), 
			reinterpret_cast<char*>(&requestAPIKey) + sizeof(requestAPIKey));

	int16_t requestAPIVersion = htons(5);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&requestAPIVersion), 
			reinterpret_cast<char*>(&requestAPIVersion) + sizeof(requestAPIVersion));

	int32_t correlationId = htonl(7);
	header.insert(header.end(), 
			reinterpret_cast<char*>(&correlationId), 
			reinterpret_cast<char*>(&correlationId) + sizeof(correlationId));

	std::string headerClientId = "kafka-clitest";
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
	
	//send(clientFD, &totalMessageSize, sizeof(totalMessageSize), 0);
	send(clientFD, header.data(), header.size(), 0);
	std::unordered_map<int32_t, int16_t> correlationToAPIKey;
	correlationToAPIKey[ntohl(correlationId)] = ntohs(requestAPIKey);
	std::vector<char> responseBuffer = Response::readResponse(clientFD);

	Response response(responseBuffer);
	std::cout << "Response correlation Id: " << response.getCorrelationId() << std::endl;
	int16_t responseAPIKey = correlationToAPIKey[response.getCorrelationId()];
	response.parseResponse(responseBuffer, responseAPIKey);	

	response.toString();

	close(clientFD);
	return 0;

}
