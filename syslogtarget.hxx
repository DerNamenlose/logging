#ifndef SYSLOGTARGET_HXX
#define SYSLOGTARGET_HXX

#include <sstream>

#include <syslog.h>

#include "logging.hxx"
#include "logging_utils.hxx"
#include "locking.hxx"

namespace Logging
{
    template <typename LockType = NullLock> class SyslogTarget : public LockType
    {
        int mOption;
        int mFacility;
        int mLevel;
        std::stringstream mOs;
        /**
         * store the logger names internally in order to make it a) more
         *  efficient and b) have them survive long enough from the 
         *  openlog to the syslog call.
         * The key type is void const * const, as this map cannot know anything about
         *  the actual logger type and it doesn't need to, as the pointer is just used
         *  as an index.
         */
        std::map<void const * const, std::string> mLoggerNames; 
        
        template <typename LoggerType> char const *loggerName(LoggerType const * const l)
        {
            auto i = mLoggerNames.find(l); // we just index by the pointer, as the logger instances should be constant anyway
            if (i == mLoggerNames.end()) {
                i = mLoggerNames.insert(std::make_pair(l, canonicalName(*l).c_str())).first;
            }
            return i->second.c_str();
        }
        
    public:
        
        SyslogTarget(int option = LOG_CONS, int facility = LOG_USER)
            : mOption(option), mFacility(facility)
        {
        }        
        
        template <typename LoggerType> void startMessage(LoggerType const &source, TraceLevel tl)
        {
            LockType::lock();
            openlog(loggerName(&source), mOption, mFacility);
            mOs << '[' << levelName(tl) << "] ";
            mLevel = tl;
        }
        
        template <typename LoggerType> void startMessage(LoggerType const &source, LogLevel ll)
        {
            LockType::lock();
            openlog(loggerName(&source), mOption, mFacility);
            mOs << '[' << levelName(ll) << "] ";
            mLevel = ll;
        }
        
        template <typename LoggerType> void endMessage(LoggerType const &)
        {
            int level = 0;
            switch (mLevel)
            {
            case Logging::LEVEL_TRACE:
                level = LOG_DEBUG;
                break;
            case Logging::LEVEL_DEBUG:
                level = LOG_DEBUG;
                break;
            case Logging::LEVEL_INFO:
                level = LOG_INFO;
                break;
            case Logging::LEVEL_WARNING:
                level = LOG_WARNING;
                break;
            case Logging::LEVEL_ERROR:
                level = LOG_ERR;
                break;
            case Logging::LEVEL_FATAL:
                level = LOG_EMERG;
                break;
            }
            syslog(level, "%s", mOs.str().c_str());
            mOs.str(std::string());
            LockType::unlock();
        }
        
        /**
        * output a value to the underlying stream.
        * 
        * \param v The value. This method has the same effect as <tt>os << v</tt> (with os being the underlying
        *          std::ostringstream.
        */
        template <typename LoggerType, typename ValueT> void put(LoggerType const &, ValueT const &v)
        {
            mOs << v;
        }
        
        /**
        * Output stuff like std::endl to the underlying stream.
        * 
        * \param manip The manipulator to output to the underlying stream.
        */
        void put(std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
        {
            mOs << manip;
        }
    };
    
    /**
     * traits specialization for the SyslogTarget
     */
    template <typename LockType> struct TargetTraits<SyslogTarget<LockType>>
    {
        typedef char char_type;
        typedef std::char_traits<char> char_traits_type;
    };
}
#endif // SYSLOGTARGET_HXX
