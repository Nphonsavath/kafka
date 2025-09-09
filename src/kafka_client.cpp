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

int parseArgs(int numArgs, char* argv[], std::string& serverIP, int& serverPort) {
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

kafkaRequestHeaderV2 makeHeader(int16_t requestAPIKey, int16_t requestAPIVersion, int32_t correlationId,
		std::string clientIdNullable, int8_t tagBuffer) {
	return {requestAPIKey, requestAPIVersion, correlationId, clientIdNullable, tagBuffer}; 
}

APIVersionRequestBodyV4 makeAPIVersionBody(std::string clientIdCompact, std::string clientIdSoftwareVerCompact,
		int8_t tagBuffer) {
	return {clientIdCompact, clientIdSoftwareVerCompact, tagBuffer};
}

std::vector<char> buildRequest(const kafkaRequestHeaderV2& header) {
	std::vector<char> buffer;
	
	int32_t messageSize = 0;
	kafka::appendValue(messageSize, buffer);
	
	kafka::appendValue(header.requestAPIKey, buffer);
	
	kafka::appendValue(header.requestAPIVersion, buffer);
	
	kafka::appendValue(header.correlationId, buffer);
	
	int16_t clientIdLength = header.clientIdNullable.size();
	kafka::appendValue(clientIdLength, buffer);
    	kafka::appendValue(header.clientIdNullable, buffer);
	return buffer;
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
		
	kafkaRequestHeaderV2 requestHeader = makeHeader(18, 4, 7, "Noah", 0);
	std::vector<char> header = buildRequest(requestHeader);
		
	//int32_t messageSize = 0;
	//kafka::appendValue(messageSize, header);

	//int16_t requestAPIKey = 18;
	//kafka::appendValue(requestAPIKey, header);

	//int16_t requestAPIVersion = 5;
	//kafka::appendValue(requestAPIVersion, header);
	
	//int32_t correlationId = 7;
	//kafka::appendValue(correlationId, header);

	//std::string headerClientId = "kafka-clitest";
    	//int16_t headerClientIdLength = headerClientId.size();
	//kafka::appendValue(headerClientIdLength, header);
    	//kafka::appendValue(headerClientId, header);
				  
	APIVersionRequestBodyV4 body {
		"kafka-cli",
		"0.1",
		0
	};

	int8_t bodyClientIdLength = body.clientIdCompact.size();
	kafka::appendValue(bodyClientIdLength, header);
	kafka::appendValue(body.clientIdCompact, header);	

	int8_t bodyClientIdSoftwareVerLength = body.clientIdSoftwareVerCompact.size();
	kafka::appendValue(bodyClientIdSoftwareVerLength, header);
	kafka::appendValue(body.clientIdSoftwareVerCompact, header);
	kafka::appendValue(body.tagBuffer, header);

	int32_t totalMessageSize = htonl(header.size());
	std::cout << ntohl(totalMessageSize) << std::endl;
	memcpy(header.data(), &totalMessageSize, sizeof(totalMessageSize));	
	
	send(clientFD, header.data(), header.size(), 0);
	std::unordered_map<int32_t, int16_t> correlationToAPIKey;
	correlationToAPIKey[requestHeader.correlationId] = requestHeader.requestAPIKey;
	std::vector<char> responseBuffer = Response::readResponse(clientFD);

	Response response(responseBuffer);
	std::cout << "Response correlation Id: " << response.getCorrelationId() << std::endl;
	int16_t responseAPIKey = correlationToAPIKey[response.getCorrelationId()];
	response.parseResponse(responseBuffer, responseAPIKey);	

	response.toString();

	close(clientFD);
	return 0;

}
