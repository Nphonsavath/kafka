#include "response.hpp"
#include "kafka_protocol.hpp"

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
		if constexpr (std::endian::native == std::endian::little) {
			ret = std::byteswap(ret);
		}
		return ret;
	}
}

std::vector<char> Response::readResponse(int clientFD) {
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

void Response::parseAPIVersionsResponse(std::vector<char> bytes) {
	char* data = bytes.data();
	int offset = 0;

	APIVersionsResponseBodyV4 body;
	body.errorCode = convertToBigEndian<int16_t>(data + offset);
	offset += sizeof(body.errorCode);
	
	int APIVersionsArrayLength = convertToBigEndian<int8_t>(data + offset);
	offset += sizeof(APIVersionsArrayLength);
	for (int i = 0; i < APIVersionsArrayLength; i++) {
		APIKeyVersion api;

		api.APIKey = convertToBigEndian<int16_t>(data + offset);
	       	offset += sizeof(api.APIKey);
		
		api.minVersion = convertToBigEndian<int16_t>(data + offset);
		offset += sizeof(api.minVersion);

		api.maxVersion = convertToBigEndian<int16_t>(data + offset);
		offset += sizeof(api.maxVersion);

		api.tagBuffer = convertToBigEndian<int8_t>(data + offset);
		offset += sizeof (api.tagBuffer);
		
		body.APIKeys.push_back(api);	
	}
	
	body.throttleTimeMs = convertToBigEndian<int32_t>(data + offset);
	offset += sizeof(body.throttleTimeMs);

	body.tagBuffer = convertToBigEndian<int8_t>(data + offset);
	offset += sizeof(body.tagBuffer);
	responseData = body;
}

void Response::parseResponse(std::vector<char> bytes, int APIKey) {
	if (APIKey == 18) {
		parseAPIVersionsResponse(bytes);
	}
}

Response::Response(std::vector<char> bytes) {
	char* data = bytes.data();
	int offset = 0;

	responseHeader.correlationId = convertToBigEndian<std::int32_t>(data + offset);
	offset += sizeof(responseHeader.correlationId);
	
	//std::variant<APIVersionsResponseBodyV4> responseDaa;

	//responseHeader.errorCode = convertToBigEndian<std::int16_t>(data + offset);
	//offset += sizeof(responseHeader.errorCode);
}



void Response::toString() {
	std::cout << "Response Correlation Id: " << responseHeader.correlationId << '\n';
	if (auto APIData = std::get_if<APIVersionsResponseBodyV4>(&responseData)) {
		std::cout << "Error code: " << APIData->errorCode << '\n';
		for (auto& api : APIData->APIKeys) {
			std::cout << "APIKey: " << api.APIKey
				<< " versions( " << api.minVersion
				<< " to " << api.maxVersion << ")\n";
		}
		std::cout << "Throttle Time (ms): " << APIData->throttleTimeMs << '\n';
		std::cout << "Tag Buffer: " << APIData->tagBuffer << '\n';
	}
}
