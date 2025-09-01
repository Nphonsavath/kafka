#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <vector>
#include <cstdint>

class Response {
public:
	explicit Response(std::vector<char> bytes);
	void toString();

private:
	int32_t responseMessageSize;
	struct kafkaResponseHeaderV0 {
		int32_t correlationId;
		int16_t errorCode;
	};
	kafkaResponseHeaderV0 responseHeader;

};
#endif
