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

template <typename T>
void appendValue(T value, std::vector<char>& buffer) {
	if constexpr (sizeof(T) == 1) {
		buffer.push_back(static_cast<char>(value));
	} else {
		if constexpr (sizeof(T) == 2) { value = htons(static_cast<T>(value)); }
		if constexpr (sizeof(T) == 4) { value = htonl(static_cast<T>(value)); }
		buffer.insert(buffer.end(),
				reinterpret_cast<char*>(&value),
				reinterpret_cast<char*>(&value) + sizeof(value));
	}
}

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

	std::vector<char> requestBuffer = Request::readRequest(clientFD);
	Request request(requestBuffer);
	request.toString();

	std::vector<APIKeyVersion> supportedAPIs = {
		{18, 0, 4, 0}
	};

	std::vector<char> header;
	
	int32_t messageSize = 0;
	appendValue(messageSize, header);

	int32_t correlationId = request.getCorrelationId();
	appendValue(correlationId, header);

	int16_t errorCode;	
	if (request.getRequestAPIKey() == 18) {
		if (request.getRequestAPIVersion() >= 0 && request.getRequestAPIVersion() <= 4) {
			errorCode = ERROR_NONE;
			appendValue(errorCode, header);
			
			int8_t supportedAPIsLength = supportedAPIs.size() + 1;
			appendValue(supportedAPIsLength, header);
			for (const APIKeyVersion& api : supportedAPIs) {
				appendValue(api.APIKey, header);
				std::cout << "api.APIKey: " << api.APIKey << std::endl;
				appendValue(api.minVersion, header);
				std::cout << "api.minVersion: " << api.minVersion << std::endl;
				appendValue(api.maxVersion, header);
				std::cout << "api.maxVersion: " << api.maxVersion << std::endl;
				appendValue(api.tagBuffer, header);	
				std::cout << "api.tagBuffer: " << static_cast<int>(api.tagBuffer) << std::endl;
			}

			int32_t throttleTime = 0;
			appendValue(throttleTime, header);	

			int8_t tagBuffer = 0;
			header.push_back(static_cast<char>(tagBuffer));
		} else {
			errorCode = UNSUPPORTED_VERSION;
			appendValue(errorCode, header);
		}
	}

	int32_t totalMessageSize = htonl(header.size());
	std::cout << ntohl(totalMessageSize) << std::endl;
	memcpy(header.data(), &totalMessageSize, sizeof(totalMessageSize));	
	//send(clientFD, &totalMessageSize, sizeof(totalMessageSize), 0);
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
