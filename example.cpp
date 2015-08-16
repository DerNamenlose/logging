#include <iostream>
#include <memory>

#include "logging.hxx"
#include "ostreamtarget.hxx"

// type definition of a simple, non-threadsafe target logging to std::ostream
typedef Logging::OStreamTarget<std::ostream> LogTarget;
// type definition of a logger logging to the target above
//   the TRACING-flag is derived from the NDEBUG-flag set by many compilers in release mode
//   It indicates, whether the logger sould output TRACE and DEBUG messages. Used like
//   here, it disables tracing, when NDEBUG is set on compilation
typedef Logging::Logger<LogTarget, TRACING> Logger;

int main()
{
    auto target = std::make_shared<LogTarget>(std::cout);    // log target logging messages to std::cout
    auto logger = std::make_shared<Logger>(target, "root");  // logger using the target object above

    logger->setLevel(Logging::LEVEL_DEBUG);  // the logger will output messages DEBUG and higher

    // logging two different levels to the output
    logger << Logging::LEVEL_DEBUG << "This message should not be visible in release mode\n";
    logger << Logging::LEVEL_INFO << "This should be visible\n";

    // creating a child logger
    auto child = logger->child("child");
    child->setLevel(Logging::LEVEL_ERROR);  // set a different config for the child logger

    child << Logging::LEVEL_INFO << "This messages should never be visible.\n";
    child << Logging::LEVEL_ERROR << "This should be.\n";

    if (!logger->isEnabled(Logging::LEVEL_DEBUG)) {
    	logger << Logging::LEVEL_INFO << "Release mode. Will not output any trace messages.\n";
    }
    else {
    	logger << Logging::LEVEL_DEBUG << "Debug mode. Will output trace messages.\n";
    }

    auto cerrTarget = std::make_shared<LogTarget>(std::cerr);
    child->setTarget(cerrTarget);
    child->setLevel(Logging::LEVEL_INFO);

    logger << Logging::LEVEL_INFO << "This should be on cout\n";
    child << Logging::LEVEL_INFO << "And this on cerr\n";

    return 0;
}
