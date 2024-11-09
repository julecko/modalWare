#pragma once

enum class StatusCode : uint8_t {
	SUCCESS = 0,
	FAILURE = 1,
};
struct ReturnData {
	StatusCode result;
	std::string value;
};

ReturnData processMessage(const std::string& message);