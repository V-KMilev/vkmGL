#pragma once

/**
 * @brief Private function called only by the macro 
 * m_assert used for printing the assert
 * 
 * @note Assert print is been called only if the given 
 * expectation is wrong. There are two print formats. 
 * The first is without a comment message and the 
 * second is with comment message. __FILE__, __LINE__ 
 * and __FUNCTION__ are equal of the file, line and 
 * funtion the M_ASSERT is being executed. __VA_ARGS__ 
 * is the representation of the comment message 
 * 
 * @param[in] expected_str 
 * @param[in] expected 
 * @param[in] file 
 * @param[in] line 
 * @param[in] function 
 * @param[in] message 
 */
void __m_assert(
	const char* expected_str,
	bool expected,
	const char* file,
	int line,
	const char* function,
	const char* message = nullptr
);

#ifdef NDEBUG
	#define M_ASSERT(Expected, ...)
#else
	#define M_ASSERT(Expected, ...)                                                     \
		__m_assert(#Expected, Expected, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif
