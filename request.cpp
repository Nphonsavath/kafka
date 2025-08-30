#include "request.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>

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

Request::request(std::vector<char> bytes) {

	//TODO: Error check bytes < expected size of header
	// if (bytes.size() < ...) {
	//
	// }
	
	char* data = bytes.data();
	int offset = 0;

	requestMessageSize = convertToBigEndian<int32_t>(data + offset);
	offset += sizeof(requestMessageSize);
	
	requestHeader.requestAPIKey = convertToBigEndian<int16_t>(data + offset);
	offset += sizeof(requestHeader.requestAPIKey);
	
	requestHeader.requestAPIVersion = convertToBigEndian<int16_t>(data + offset);
	offset += sizeof(requestHeader.requestAPIVersion);
	
	requestHeader.correlationId = convertToBigEndian<int32_t>(data + offset);
	offset += sizeof(requestHeader.correlationId);
	
	int16_t clientIdLength = convertToBigEndian<int16_t>(data + offset);
	if (clientIdLength > 0) {
		if (offset + clientIdLength > bytes.size()) {
			std::runtime_error("Error clientIdLength greater than bytes remaining");
		}
		requestHeader.clientIdNullable = std::string(data + offset, clientIdLength);
		offset += sizeof(clientIdLength);
	} else {
		requestHeader.clientIdNullable = "";
	}

	if (offset < bytes.size()) {
		requestHeader.tagBuffer = static_cast<int8_t>(data[offset]);
		offset += 1;
	}


}
