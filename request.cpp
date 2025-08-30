#include "request.hpp"
#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
	int32_t convertToBigEndian(char* bytes) {
		int32_t ret = 0;
		memcpy(&ret, bytes, sizeof(ret));
		if constexpr (std::endian::native == std::endian::little) {
			ret = std::byteswap(ret);
		}
		
		return ret;
	}
}

Request::request(std::vector<char> bytes) {
	char* data = bytes.data();
	int offset = 0;

}
