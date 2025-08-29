#ifndef KAFKA_PROTOCOL_H
#define KAFKA_PROTOCOL_H

#include <string>

constexpr int16_t ERROR_NONE = 0;
constexpr int16_t UNSUPPORTED_VERSION = 35;

struct KafkaRequestHeaderV2 {
	int32_t messageSize; //extra
	int16_t requestAPIKey;
	int16_t requestAPIVersion;
	int32_t correlationId;
	std::string clientIdNullable;
	int8_t tagBuffer = 0;
};

struct APIVersionsRequestBodyV4 {
	std::string clientIdCompact;
	std::string clientIdSoftwareVerCompact;
	int8_t tagBuffer = 0;
};

struct APIVersionsResponseBodyV4 {
	int32_t messageSize; //extra
	int32_t correlationId;
	int16_t errorCode;
	std::vector<APIKeyVersion> APIKeys;
	int32_t throttleTimeMs;
	int8_t tagBuffer = 0;
}

struct APIKeyVersion {
	int16_t APIKey;
	int16_t minVersion;
	int16_t maxVersion;
	int8_t tagBuffer = 0;
}

struct 

#endif
