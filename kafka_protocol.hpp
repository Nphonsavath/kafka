#ifndef KAFKA_PROTOCOL_HPP
#define KAFKA_PROTOCOL_HPP

#include <string>

constexpr int16_t ERROR_NONE = 0;
constexpr int16_t UNSUPPORTED_VERSION = 35;

struct APIVersionRequestBodyV4 {
	std::string clientIdCompact;
	std::string clientIdSoftwareVerCompact;
	int8_t tagBuffer;
};

struct APIKeyVersion {
	int16_t APIKey;
	int16_t minVersion;
	int16_t maxVersion;
	int8_t tagBuffer;
};

struct APIVersionsResponseBodyV4 {
	int16_t errorCode;
	std::vector<APIKeyVersion> APIKeys;
	int32_t throttleTimeMs;
	int8_t tagBuffer;
};


#endif
