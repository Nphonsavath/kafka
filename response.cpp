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
		if constexpr (sizeof(T) > 1) {
			if constexpr (std::endian::native == std::endian::little) {
				ret = std::byteswap(ret);
			}
		}
		return ret;
	}

	template<typename T>
	T readBigEndian(char* bytes, int& offset) {
		T val = convertToBigEndian<T>(bytes + offset);
		offset += sizeof(T);
		return val;
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
	std::cout << "Bytes.size(): " << bytes.size() << std::endl;
	int offset = 4;

	APIVersionsResponseBodyV4 body;
	body.errorCode = readBigEndian<int16_t>(data, offset);
	
	int8_t APIVersionsArrayLength = readBigEndian<int8_t>(data, offset) - 1;
	std::cout << "APIVersionsArrayLength: " << static_cast<int>(APIVersionsArrayLength) << std::endl;
	for (int i = 0; i < APIVersionsArrayLength; i++) {
		APIKeyVersion api;

		api.APIKey = readBigEndian<int16_t>(data, offset);
		std::cout << "api.APIKey: " << api.APIKey << std::endl;
		
		api.minVersion = readBigEndian<int16_t>(data, offset);
		std::cout << "api.minVersion: " << api.minVersion << std::endl;

		api.maxVersion = readBigEndian<int16_t>(data, offset);
		std::cout << "api.maxVersion: " << api.maxVersion << std::endl;

		api.tagBuffer = readBigEndian<int8_t>(data, offset);
		std::cout << "api.tagBuffer: " << static_cast<int>(api.tagBuffer) << std::endl;
		
		body.APIKeys.push_back(api);	
	}
	
	body.throttleTimeMs = readBigEndian<int32_t>(data, offset);

	body.tagBuffer = readBigEndian<int8_t>(data, offset);
	responseData = body;
}

void Response::parseResponse(std::vector<char> bytes, int APIKey) {
	std::cout << "APIKey: " << APIKey << std::endl;
	if (APIKey == 18) {
		Response::parseAPIVersionsResponse(bytes);
	}
}

Response::Response(std::vector<char> bytes) {
	char* data = bytes.data();
	int offset = 0;

	responseHeader.correlationId = readBigEndian<int32_t>(data, offset);
	
	//std::variant<APIVersionsResponseBodyV4> responseDaa;

	//responseHeader.errorCode = convertToBigEndian<std::int16_t>(data + offset);
	//offset += sizeof(responseHeader.errorCode);
}



void Response::toString() {
	std::cout << "Response Correlation Id: " << responseHeader.correlationId << '\n';
	if (auto APIData = std::get_if<APIVersionsResponseBodyV4>(&responseData)) {
		std::cout << "Error code: " << APIData->errorCode << '\n';
		for (auto& api : APIData->APIKeys) {
			std::cout << "APIKey: " << api.APIKey << '\n';
			std::cout << "Supported Versions: (" << api.minVersion
				<< " to " << api.maxVersion << ")\n";
		}
		std::cout << "Throttle Time (ms): " << APIData->throttleTimeMs << '\n';
		std::cout << "Tag Buffer: " << static_cast<int>(APIData->tagBuffer) << '\n';
	}
}
