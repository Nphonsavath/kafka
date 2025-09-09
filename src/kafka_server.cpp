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

int main() {
	//Create TCP socket using IPv4. 
	int serverFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFD  == -1) {
		std::cerr << "Error creating server socket." << std::endl;
		return -1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(9092); //default port # for kafka
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverFD, (struct sockaddr*) &server, sizeof(server)) == -1) {
		close(serverFD);
		std::cerr << "Error binding server socket to 9092." << std::endl;
		return -1;
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

	if (listen(serverFD, MAX_CONNECTION_REQUESTS) == -1) {
		close(serverFD);
		std::cerr << "Error listening for connections." << std::endl;
		return -1;	
	}

	while (true) {
		struct sockaddr_in client;
		socklen_t clientLength = sizeof(client);
		int clientFD = accept(serverFD, (struct sockaddr*) &client, &clientLength); 
		if (clientFD == -1) {
			close(serverFD);
			std::cerr << "Error connecting client with server." << std::endl;
			return -1;
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
		kafka::appendValue(messageSize, header);

		int32_t correlationId = request.getCorrelationId();
		kafka::appendValue(correlationId, header);

		int16_t errorCode;	
		if (request.getRequestAPIKey() == 18) {
			if (request.getRequestAPIVersion() >= 0 && request.getRequestAPIVersion() <= 4) {
				errorCode = ERROR_NONE;
				kafka::appendValue(errorCode, header);
				
				int8_t supportedAPIsLength = supportedAPIs.size() + 1;
				kafka::appendValue(supportedAPIsLength, header);
				for (const APIKeyVersion& api : supportedAPIs) {
					kafka::appendValue(api.APIKey, header);
					std::cout << "api.APIKey: " << api.APIKey << std::endl;
					kafka::appendValue(api.minVersion, header);
					std::cout << "api.minVersion: " << api.minVersion << std::endl;
					kafka::appendValue(api.maxVersion, header);
					std::cout << "api.maxVersion: " << api.maxVersion << std::endl;
					kafka::appendValue(api.tagBuffer, header);	
					std::cout << "api.tagBuffer: " << static_cast<int>(api.tagBuffer) << std::endl;
				}

				int32_t throttleTime = 0;
				kafka::appendValue(throttleTime, header);	

				int8_t tagBuffer = 0;
				header.push_back(static_cast<char>(tagBuffer));
			} else {
				errorCode = UNSUPPORTED_VERSION;
				kafka::appendValue(errorCode, header);
			}
		}

		int32_t totalMessageSize = htonl(header.size());
		std::cout << ntohl(totalMessageSize) << std::endl;
		memcpy(header.data(), &totalMessageSize, sizeof(totalMessageSize));	
		
		send(clientFD, header.data(), header.size(), 0);

		close(clientFD);
	}	
	
	close(serverFD);
	return 0;
}
