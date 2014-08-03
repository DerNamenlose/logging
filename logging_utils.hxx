#ifndef LOGGING_UTILS
#define LOGGING_UTILS

#include "logging.hxx"

inline char const * const levelName(TraceLevel tl)
{
    if (tl == LEVEL_TRACE) {
        return "TRACE";
    }
    else {
        return "DEBUG";
    }
}

inline char const * const levelName(LogLevel ll)
{
    switch (ll) {
        case LEVEL_INFO:
            return "INFO";
            break;
        case LEVEL_WARNING:
            return "WARNING";
            break;
        case LEVEL_ERROR:
            return "ERROR";
            break;
        case LEVEL_FATAL:
            return "FATAL";
            break;
        default:
            return "UNKNOWN";
            break;
    }
}

#endif