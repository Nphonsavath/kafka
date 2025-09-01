#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <vector>
#include <string>

class Request {
public:
	explicit Request(std::vector<char> bytes);
	explicit Request(int clientFD);
	int32_t getRequestMessageSize() { return requestMessageSize; }
	int16_t getRequestAPIKey() { return requestHeader.requestAPIKey; }
	int16_t getrequestAPIVersion() { return requestHeader.requestAPIVersion; }
	int32_t getCorrelationId() { return requestHeader.correlationId; }
	std::string getClientId() { return requestHeader.clientIdNullable; }
	int8_t getTagBuffer() { return requestHeader.tagBuffer; }
	void toString();

private:
	int32_t requestMessageSize;
	struct kafkaRequestHeaderV2 {
		int16_t requestAPIKey;
		int16_t requestAPIVersion;
		int32_t correlationId;
		std::string clientIdNullable;
		int8_t tagBuffer = 0;
	};
	kafkaRequestHeaderV2 requestHeader;
};

#endif
