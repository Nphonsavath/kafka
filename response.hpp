#ifdef RESPONSE_HPP
#define RESPONSE_HPP

class Response {
public:
	explicit Response(std::vector<char> bytes);
	void toString();

private:
	int32_t responseMessageSize;
	struct kafkaResponseHeaderV0 {
		int32_t correlationId;
	};
	kafkaResponseHeaderV0 responseHeader;

};
#endif
