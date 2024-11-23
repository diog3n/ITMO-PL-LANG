#pragma once

enum blaise_debug_level {
    BLAISE_DEBUG_FULL = 3,
    BLAISE_DEBUG_INTERNAL_INFO = 2,
    BLAISE_DEBUG_ERRORS_ONLY = 1,
};

#define DEBUG   0

#define ON_DEBUG(__level) \
    if (DEBUG >= __level)

#define DEBUG__BEGIN(__level) \
    ON_DEBUG(__level) std::cout << "In function " << __func__ << std::endl

#define DEBUG__PRINT_VAL(__level, __val) \
    ON_DEBUG(__level) std::cout << #__val << " = " << __val << std::endl

#define DEBUG_OUT(__level) \
    ON_DEBUG(__level) std::cout << __func__ << ": "