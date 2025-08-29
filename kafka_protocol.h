#ifndef KAFKA_PROTOCOL_H
#define KAFKA_PROTOCOL_H

#include <string>

constexpr int16_t ERROR_NONE = 0;
constexpr int16_t UNSUPPORTED_VERSION = 35;

struct v2KafkaRequestHeader {
	int32_t messageSize;
	int16_t requestAPIKey;
	int16_t requestAPIVersion;
	int32_t correlationId;
	std::string clientId;
	int8_t tagBuffer;
};



#endif
