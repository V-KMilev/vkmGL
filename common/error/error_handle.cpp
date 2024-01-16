#include "error_handle.h"

#include <stdio.h>
#include <stdlib.h>

void __m_assert(
	bool expected,
	const char* expected_str,
	const char* file,
	int line,
	const char* message
) {
	if (expected) {
		return;
	}

	printf("[FAILED] M_ASSERT >>\t[%s:%d] Expected: '%s', Info: %s\n", file, line, expected_str, message);

#ifdef _WIN32
		__debugbreak();
#else
		abort();
#endif
}
