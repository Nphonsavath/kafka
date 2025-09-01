#include "response.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>

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

Response::Response(std::vector<char> bytes) {
	char* data = bytes.data();
	int offset = 0;

	responseHeader.correlationId = convertToBigEndian<std::int32_t>(data + offset);
	offset += sizeof(responseHeader.correlationId);

	responseHeader.errorCode = convertToBigEndian<std::int16_t>(data + offset);
	offset += sizeof(responseHeader.errorCode);
}

void Response::toString() {
	std::cout << "Response Correlation Id: " << responseHeader.correlationId << '\n';
	std::cout << "Response Error Code: " << responseHeader.errorCode << '\n';
}
