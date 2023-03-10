#include "error_handle.h"

#include <iostream>

void __m_assert(
	const char* expected_str,
	bool expected,
	const char* file,
	int line,
	const char* function,
	const char* message
) {
	if (expected) {
		return;
	}

	if(message) {
		std::cerr << "M_ASSERT FAILED: "
			<< function << " [ERROR::" << expected_str << "] at [" << file << ":" << line << "]\n"
			<< "Message: " << message << "\n";
	} else {
		std::cerr << "M_ASSERT FAILED: "
			<< function << " [ERROR::" << expected_str << "] at [" << file << ":" << line << "]\n"; 
	}

#ifdef _WIN32
		__debugbreak();
#else
		abort();
#endif
}
