#ifndef LOGGING_UTILS
#define LOGGING_UTILS

#include "logging.hxx"

/**
 * Return a string representation of the given trace level
 * 
 * \param tl The trace level to transform.
 * \return The string representation of the given trace level.
 */
inline char const * const levelName(TraceLevel tl)
{
    if (tl == LEVEL_TRACE) {
        return "TRACE";
    }
    else {
        return "DEBUG";
    }
}

/**
 * Return a string representation of the given log level
 * 
 * \param ll The log level to transform.
 * \return The string representation of the given log level.
 */
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

    
/**
* Get the full name of a logger. The canonical name consists of the
* full chain up to the root logger separated by the given separator.
* 
* \param l The logger to return the canonical name on.
* \param separator The separator to join the canonical name parts with.
* \return The full canonical name of the logger
*/
template <typename LoggerType> std::string canonicalName(LoggerType const &l, std::string const &separator = "::")
{
    if (l.parent() && l.parent()->name().size() > 0) {
        return canonicalName(*(l.parent()), separator)+separator+l.name();
    }
    else {
        return l.name();
    }
}

#endif