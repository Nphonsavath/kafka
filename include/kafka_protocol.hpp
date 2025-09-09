#ifndef KAFKA_PROTOCOL_HPP
#define KAFKA_PROTOCOL_HPP

#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>

constexpr int16_t ERROR_NONE = 0;
constexpr int16_t UNSUPPORTED_VERSION = 35;
constexpr int MAX_CONNECTION_REQUESTS = 5;

namespace kafka
{
	template <typename T>
	T convertToBigEndian(char* bytes) {
		T ret = 0;
		memcpy(&ret, bytes, sizeof(ret));
		if constexpr (sizeof(T) > 1) {
			if constexpr (std::endian::native == std::endian::little) {
				ret = std::byteswap(ret);
			}
		}
		return ret;
	}

	template <typename T>
	T readBigEndian(char* bytes, int& offset) {
		T val = convertToBigEndian<T>(bytes + offset);
		offset += sizeof(T);
		return val;
	}

	template <typename T>
	void appendValue(T value, std::vector<char>& buffer) {
		if constexpr (sizeof(T) == 1) {
			buffer.push_back(static_cast<char>(value));
		} else {
			if constexpr (sizeof(T) == 2) { value = htons(static_cast<uint16_t>(value)); }
			if constexpr (sizeof(T) == 4) { value = htonl(static_cast<uint32_t>(value)); }
			buffer.insert(buffer.end(),
					reinterpret_cast<char*>(&value),
					reinterpret_cast<char*>(&value) + sizeof(value));
		}
	}

	inline void appendValue(const std::string& str, std::vector<char>& buffer) {
		buffer.insert(buffer.end(), str.begin(), str.end());
	}
}

struct IRequestBody {
	virtual ~IRequestBody() = default;
	virtual void appendToBuffer(std::vector<char>& buffer) = 0;
};

struct kafkaRequestHeaderV2 {
	int16_t requestAPIKey;
	int16_t requestAPIVersion;
	int32_t correlationId;
	std::string clientIdNullable;
	int8_t tagBuffer;
};

struct APIVersionRequestBodyV4 : public IRequestBody {
	std::string clientIdCompact;
	std::string clientIdSoftwareVerCompact;
	int8_t tagBuffer;

APIVersionRequestBodyV4 (std::string clientId, std::string clientIdSoftwareVer, int8_t tag) 
	: clientIdCompact(std::move(clientId)), 
	clientIdSoftwareVerCompact(std::move(clientIdSoftwareVer)), 
	tagBuffer(tag) {}
	void appendToBuffer(std::vector<char>& buffer) override {
		int8_t clientIdLength = clientIdCompact.size();
		kafka::appendValue(clientIdLength, buffer);
		kafka::appendValue(clientIdCompact, buffer);	

		int8_t clientIdSoftwareVerLength = clientIdSoftwareVerCompact.size();
		kafka::appendValue(clientIdSoftwareVerLength, buffer);
		kafka::appendValue(clientIdSoftwareVerCompact, buffer);

		kafka::appendValue(tagBuffer, buffer);
	}
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
