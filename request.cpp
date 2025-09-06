#include "request.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace
{
	template <typename T>
	T convertToBigEndian(char* bytes) {
		T ret = 0;
		memcpy(&ret, bytes, sizeof(ret));
		if constexpr (sizeof(T) > 1) {
			if constexpr (std::endian::native == std::endian::little) {
				ret = std::byteswap(ret);
			}
		}
		return ret;
	}

	template <typename T>
	T readBigEndian(char* bytes, int& offset) {
		T val = convertToBigEndian<T>(bytes + offset);
		offset += sizeof(T);
		return val;
	}
}


std::vector<char> Request::readRequest(int clientFD) {
	int expectedMessageLength = 0;
	int totalReadBytes = 0;
	if (recv(clientFD, &expectedMessageLength, sizeof(expectedMessageLength), 0) == -1) {
		throw std::runtime_error("Error reading from client fd");
	}
	expectedMessageLength = ntohl(expectedMessageLength);
	totalReadBytes += sizeof(expectedMessageLength);
	
	std::vector<char> buffer(expectedMessageLength);
	while (totalReadBytes < expectedMessageLength) {
		int currentReadBytes = recv(clientFD, 
				buffer.data() + totalReadBytes - sizeof(expectedMessageLength), 
				expectedMessageLength - totalReadBytes,
			       	0);
		std::cout << "Current read bytes: " << currentReadBytes << std::endl;
		std::cout << "Total read bytes: " << totalReadBytes << std::endl;
		if (currentReadBytes == -1) {
			throw std::runtime_error("Error reading from socket");
		} else if (currentReadBytes == 0) {
			std::cout << "Completed reading from socket" << std::endl;
			break;
		}
		totalReadBytes += currentReadBytes;
	}
	return buffer;
}

Request::Request(std::vector<char> bytes) : requestMessageSize(bytes.size() - sizeof(requestMessageSize)) {

	//TODO: Error check bytes < expected size of header
	// if (bytes.size() < ...) {
	//
	// }
	
	char* data = bytes.data();
	int offset = 0;

	requestHeader.requestAPIKey = readBigEndian<int16_t>(data, offset);
	
	requestHeader.requestAPIVersion = readBigEndian<int16_t>(data, offset);
	
	requestHeader.correlationId = readBigEndian<int32_t>(data, offset);
	
	int16_t clientIdLength = readBigEndian<int16_t>(data, offset);
	if (clientIdLength > 0) {
		if (offset + clientIdLength > bytes.size()) {
			std::runtime_error("Error clientIdLength greater than bytes remaining");
		}
		requestHeader.clientIdNullable = std::string(data + offset, clientIdLength);
		offset += clientIdLength;
	} else {
		requestHeader.clientIdNullable = "";
	}

	/*if (offset < requestMessageSize) {
		std::cout << "OFFSET = " << offset << " bytes.size() = " << bytes.size() << std::endl;
		requestHeader.tagBuffer = static_cast<int8_t>(data[offset]);
		offset += 1;
	} else {
		requestHeader.tagBuffer = 0;
	}*/
	requestHeader.tagBuffer = 0;
}

void Request::toString() {
	std::cout << "Request Message Size: " << requestMessageSize << '\n';
	std::cout << "Request API Key: " << requestHeader.requestAPIKey << '\n';
	std::cout << "Request API Version: " << requestHeader.requestAPIVersion << '\n';
	std::cout << "Request Correlation Id: " << requestHeader.correlationId << '\n';
	std::cout << "Request Client Id: " << requestHeader.clientIdNullable << '\n';
	std::cout << "Request Tag Buffer: " << static_cast<int>(requestHeader.tagBuffer) << '\n';
}
