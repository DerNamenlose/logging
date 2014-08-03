#ifndef OSTREAMLOGGER_HXX
#define OSTREAMLOGGER_HXX

#include <ostream>

#include <string>

#include "logging.hxx"
#include "locking.hxx"
#include "logging_utils.hxx"

/**
 * Logging target wrapping a standard C++ ostream.
 * This target takes a standard C++ ostream and wraps it for use with
 * the Logging framework.
 * 
 * \param OStreamT The type of ostream to use (e.g. std::ostream for things like std::cout, 
 *                                                  std::wostream for std::wcout etc.)
 * \param LockType The type of Lock adapter to use. This type must be default-constructible
 *                 and provide two functions:
 * 
 *                      void lock() - Lock the target object, so that no parallel access takes place.
 *                                      If the lock cannot be aquired, this function must block, until
 *                                      the lock is available.
 *                      void unlock() - release the lock on the target object. Other accesses may take place
 *                                      again.
 *                 Due to the interface required, std::mutex can be used as the LockType directly, if necessary.
 *                 Default is the NullLock, meaning that this target will not be thread-safe.
 * \see NullLock for an example of the LockType.
 */
template <typename OStreamT, typename LockType = NullLock> class OStreamTarget
    : public LockType
{
    OStreamT &mOs;
    
public:
    
    /// The type of output stream wrapped by this object. Used e.g. by TargetTraits
    typedef OStreamT ostream_type;
    
    /**
     * Constructor.
     * This constructs an object wrapping the given std::ostream.
     * 
     * \param os The std::ostream to wrap. The target object only wraps a reference to
     *           the given object. The std::ostream therefore <em>MUST</em> exist at least
     *           as long, as the OStreamTarget-object it is wrapped by.
     */
    OStreamTarget(OStreamT &os)
        : mOs ( os )
    {
    }
    
    /**
     * Start a message from the given logger with the given level.
     * 
     * \param source the logger object, which starts the message.
     * \param tl the level of this message
     * \note This method calls LockType::lock(). It might therefore block until the lock is available.
     */
    template <typename LoggerType> void startMessage(LoggerType const &source, TraceLevel tl)
    {
        LockType::lock();
        std::string const &logName = canonicalName(source);
        if (logName.size() > 0) {
            mOs << '(' << logName << ") ";
        }
        mOs << '[' << levelName(tl) << "] ";
    }
    
    /**
     * finish a message from the given source.
     * \note calls LockType::unlock(), releasing the target for other accesses.
     */
    template <typename LoggerType> void endMessage(LoggerType const &)
    {
        LockType::unlock();
    }
    
    /**
     * output a value to the underlying stream.
     * 
     * \param v The value. This method has the same effect as <tt>os << v</tt> (with os being the underlying
     *          std::ostream.
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
 * traits class for OStreamTarget
 */
template <typename OStreamT, typename LockType> struct TargetTraits<OStreamTarget<OStreamT, LockType>>
{
    /// The character type of the OStreamTarget
    typedef typename OStreamT::char_type char_type;
    /// The character traits type of the underlying OStreamTarget
    typedef typename OStreamT::traits_type char_traits_type;
};

#endif // OSTREAMLOGGER_HXX
