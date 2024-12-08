#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

	// Initialize function
	int32_t initialize(const char* code, const char* path);

	// Start the bot
	void start_bot();

	// Get the next message
	const char* get_message();

	// Send a message
	int32_t send_message(const char* content);

#ifdef __cplusplus
}
#endif