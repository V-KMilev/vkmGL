#pragma once

void __m_assert(
	bool expected,
	const char* expected_str,
	const char* file,
	int line,
	const char* message = nullptr
);

#define M_ASSERT(Expected, ...)                                      \
	__m_assert(Expected, #Expected, __FILE__, __LINE__, ##__VA_ARGS__)
