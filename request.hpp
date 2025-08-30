#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <vector>

class Request {
public:
	explicit Request(std::vector<char> bytes);
	std::int16_t getRequestAPIKey() { return header.requestAPIKey; }
	std::int16_t getrequestAPIVersion() { return header.requestAPIVersion; }
	std::int32_t getCorrelationId() { return header.correlationId; }
	std::int32_t getClientId() {}

private:
	int32_t messageSize;
	struct kafkaRequestHeaderV2 {
		int16_t requestAPIKey;
		int16_t requestAPIVersion;
		int32_t correlationId;
		std::string clientIdNullable;
		int8_t tagBuffer = 0;
	};
	kafkaRequestHeaderV2 header;
};

#endif
