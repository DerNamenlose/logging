#ifndef OSTREAMLOGGER_HXX
#define OSTREAMLOGGER_HXX

#include <ostream>

#include <string>

#include "logging.hxx"
#include "locking.hxx"
#include "logging_utils.hxx"

template <typename OStreamT, typename LockType> class OStreamTarget
    : public LockType
{
    OStreamT &mOs;
    
public:
    
    typedef OStreamT ostream_type;
    
    OStreamTarget(OStreamT &os)
        : mOs ( os )
    {
    }
    
    template <typename LoggerType> void startMessage(LoggerType const &source, TraceLevel tl)
    {
        LockGuard<OStreamTarget> g(*this);
        std::string const &logName = source.canonicalName();
        if (logName.size() > 0) {
            mOs << '(' << source.canonicalName() << ") ";
        }
        mOs << '[' << levelName(tl) << "] ";
    }
    
    template <typename LoggerType> void endMessage(LoggerType const &)
    {
    }
    
    template <typename LoggerType, typename ValueT> void put(LoggerType const &source, ValueT const &v)
    {
        mOs << v;
    }
    
    void put(std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
    {
        mOs << manip;
    }
};

/**
 * traits class for OStreamTarget
 */
template <typename OStreamT, typename LockType> struct TargetTraits<OStreamTarget<OStreamT, LockType>>
{
    typedef typename OStreamT::char_type char_type;
    typedef typename OStreamT::traits_type char_traits_type;
};

#endif // OSTREAMLOGGER_HXX
