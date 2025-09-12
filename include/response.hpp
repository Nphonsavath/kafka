#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <vector>
#include <cstdint>
#include <variant>

#include "kafka_protocol.hpp"

class Response {
public:
	explicit Response(std::vector<char> bytes);
	static std::vector<char> readResponse(int clientFD);
	void parseResponse(std::vector<char> bytes, int APIKey);
	void parseAPIVersionsResponse(std::vector<char> bytes);
	void parseTopicPartitionResponse(std::vector<char> bytes);
	int32_t getCorrelationId() { return responseHeader.correlationId; }
	void toString();

private:
	int32_t responseMessageSize;
	struct kafkaResponseHeaderV0 {
		int32_t correlationId;
	};
	kafkaResponseHeaderV0 responseHeader;
	std::variant<APIVersionsResponseBodyV4> responseData;
};
#endif
