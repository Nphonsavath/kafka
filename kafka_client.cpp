#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <cstring>

struct v2KafkaHeader {
	int32_t messageSize;
	int16_t requestAPIKey;
	int16_t requestAPIVersion;
	int32_t correlationId;
};

struct APIVersionsResponse {
	int32_t messageSize;
	int32_t correlationId;
	int16_t errorCode;
};

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
	v2KafkaHeader header;
	header.messageSize = htonl(sizeof(header));
	std::cout << "Header message size: " << ntohl(header.messageSize) << std::endl;
	header.requestAPIKey = htons(18);
	header.requestAPIVersion = htons(0);
	header.correlationId = htonl(1333056139);

	send(clientFD, &header.messageSize, sizeof(header.messageSize), 0);
	send(clientFD, &header, sizeof(header), 0);
	
	int expectedMessageLength = 0;
	int totalReadBytes = 0;
	recv(clientFD, &expectedMessageLength, sizeof(expectedMessageLength), 0);
	expectedMessageLength = ntohl(expectedMessageLength);
	//recv(clientFD, &totalReadBytes, sizeof(totalReadBytes), 0);
	//std::cout << ntohl(totalReadBytes) << std::endl;
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

	APIVersionsResponse response;
	memcpy(&response, buffer.data(), sizeof(response));
	std::cout << "Response message size: " << ntohl(response.messageSize) << std::endl;
	std::cout << "Response correlation Id: " << ntohl(response.correlationId) << std::endl;
	std::cout << "Response error code: " << ntohs(response.errorCode) << std::endl;
	
	close(clientFD);
	return 0;

}
