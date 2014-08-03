#ifndef LOGGING_HXX
#define LOGGING_HXX

#include <string>
#include <map>
#include <memory>

enum TraceLevel
{
    LEVEL_TRACE,
    LEVEL_DEBUG
};

enum LogLevel
{
    LEVEL_INFO = LEVEL_DEBUG + 1,
    LEVEL_WARNING,
    LEVEL_ERROR,
    LEVEL_FATAL
};

// forward declaration for use in the LogSentry
// template <
//   typename Target,          // logging target
//   bool trace                // tracing enabled?
//     > class Logger;

/**
 * Log sentry object guarding start and finish of a log message
 */
template <
    typename Target,
    bool outputEnabled,  // Is the output enabled?
    typename LoggerType
    > class LogSentry
{
    friend LoggerType;
    
    Target &mTarget;
    LoggerType const &mSource;
    bool mEnabled;
    
    /**
     * constructor for starting a log message with a trace level
     * 
     * \param target The target to log to
     * \param source the logger, that caused the creation of this sentry
     * \param tl The trace level of this message
     * \param enabled indicates, whether the sentry should actually output a message
     */
    LogSentry(Target &target, LoggerType const &source, TraceLevel tl, bool enabled)
        : mTarget(target), mSource { source }, mEnabled { enabled }
    {
        if (enabled) {
            mTarget.startMessage(source, tl);
        }
    }

    /**
     * constructor for starting a log message with a log level
     * 
     * \param target The target to log to
     * \param source the logger, that caused the creation of this sentry
     * \param ll The log level of this message
     * \param enabled flag indicating, whether the output of this sentry is actually enabled
     */
    LogSentry(Target &target, LoggerType const &source, LogLevel ll, bool enabled)
        : mTarget(target), mSource { source }, mEnabled { enabled }
    {
        if (enabled) {
            mTarget.startMessage(source, ll);
        }
    }
    
public:
    
    /**
     * delete the copy constructor
     */
    //LogSentry(LogSentry const &) = delete;
    
    //LogSentry &operator=(LogSentry const &) = delete;
    
    ~LogSentry()
    {
        if (mEnabled) {
            mTarget.endMessage(mSource);
        }
    }

    template <typename ValueT> inline LogSentry &operator<<(ValueT const &v)
    {
        if (mEnabled) {
            mTarget.put(mSource, v);
        }
        return *this;
    }

    /**
     * enable output like "<< std::endl" etc.
     */
    LogSentry &operator<<(std::basic_ostream<typename LoggerType::TargetTraitsType::char_type, 
                                             typename LoggerType::TargetTraitsType::char_traits_type> &(*manip)(
                                                 std::basic_ostream<typename LoggerType::TargetTraitsType::char_type, 
                                                 typename LoggerType::TargetTraitsType::char_traits_type> &))
    {
        if (mEnabled) {
            mTarget.put(mSource, manip);
        }
        return *this;
    }
};

/**
 * Log sentry object guarding start and finish of a log message
 * This class is just an empty shell to be optimized away
 */
template <
    typename Target,
    typename LoggerType
    > class LogSentry<Target, false, LoggerType>
{
    friend LoggerType;
    
    /**
     * constructor for starting a log message with a trace level
     * 
     * \internal This class only needs the trace level constructor, as it will never be created with
     *            a log level input
     */
    LogSentry(Target &, LoggerType const &, TraceLevel, bool)
    {
    }

    
public:

    template <typename ValueT> inline LogSentry &operator<<(ValueT const &)
    {
        return *this;
    }
    
};

/**
 * generic traits type specifying some information about the
 * target.
 */
template <typename TargetType> struct TargetTraits;

/**
 * Logger class
 */
template <
    typename Target,          // logging target
    bool trace,                   // tracing enabled?
    typename TargetTraits = TargetTraits<Target>
        > class Logger
{
    std::string mName;
    std::shared_ptr<Target> mTarget;
    Logger *mParent;
    std::map<std::string, std::shared_ptr<Logger>> mChildren;
    unsigned char mLevel;
    
    friend Target;
    
    /**
     * constructor for child loggers
     */
    Logger(std::string const &name, std::shared_ptr<Target> &t, Logger *parent, unsigned char level)
        : mName(name), mTarget(t), mParent(parent), mLevel(level)
    {
    }
  
public:
    
    /// The typedef exposing the target type of this logger
    typedef Target TargetType;
    /// the typedef exposing the traits to this target
    typedef TargetTraits TargetTraitsType;

    /**
     * constructor for creating a root logger
     * 
     * \param t The target where the log output is to be redirected
     * \param name The name of the root logger (default: no name)
     */
    explicit Logger(std::shared_ptr<Target> const &t, std::string const &name = std::string())
        : mName { name }, mTarget(t), mParent { nullptr }, mLevel { LEVEL_INFO }
    {
    }
    
    /**
     * default constructor. This constructor initializes the logger, creating a TargetType
     * object with its standard constructor
     */
    Logger()
        : mName { "" }, mTarget { new Target() }, mParent { nullptr }, mLevel { LEVEL_INFO }
    {
    }
    
    /**
     * starte a new log message
     */
    inline LogSentry<Target, true, Logger> operator<<(LogLevel ll)
    {
        return LogSentry<Target, 
                         true,  // Log messages are always forwarded
                         Logger>(*mTarget, *this, ll, ll >= mLevel);
    } 

    /**
     * start a new trace message
     */
    inline LogSentry<Target, trace, Logger> operator<<(TraceLevel tl)
    {
        return LogSentry<Target, 
                         trace,  // trace output is decided in the sentry
                         Logger>(*mTarget, *this, tl, tl >= mLevel);
    } 
    
    inline std::string const &name() const
    {
        return mName;
    }
    
    std::string canonicalName() const
    {
        if (mParent && mParent->name().size() > 0) {
            return mParent->canonicalName()+"."+mName;
        }
        else {
            return mName;
        }
    }
    
    Logger *parent() const
    {
        return mParent;
    }
    
    void setLevel(unsigned char level)
    {
        mLevel = level;
        for (auto i : mChildren) {
            i.second->setLevel(level);
        }
    }
    
    std::shared_ptr<Logger> child(std::string const &name)
    {
        if (name.size() == 0) {
            throw std::invalid_argument("name must not be empty");
        }
        auto child = mChildren.find(name);
        if (child == mChildren.end()) {
            child = mChildren.insert(std::make_pair(name, std::shared_ptr<Logger>(new Logger(name, mTarget, this, mLevel)))).first;
        }
        return child->second;
    }
};

/**
 * output operator to allow direct output to a shared_ptr (trace version)
 * 
 * \note This relies heavily on return value optimization. If you experience weird effects like
 *         empty log messages being output, your compiler might be creating and destroying real objects here.
 *         Throw it away then.
 */
template <typename Target, bool trace, typename TargetTraitsType> inline auto
        operator<<(std::shared_ptr<Logger<Target, trace, TargetTraitsType>> const &l, TraceLevel tl) 
            -> decltype(*l.get() << tl)
{
    // using .get() here instead of the normal dereference, as this is an unchecked operation and can be optimized out
    return *l.get() << tl;
}

/**
 * output operator to allow direct output to a shared_ptr (log version)
 * 
 * \note This relies heavily on return value optimization. If you experience weird effects like
 *         empty log messages being output, your compiler might be creating and destroying real objects here.
 *         Throw it away then.
 */
template <typename Target, bool trace, typename TargetTraitsType> inline auto
        operator<<(std::shared_ptr<Logger<Target, trace, TargetTraitsType>> const &l, LogLevel ll) 
            -> decltype((*l) << l)
{
    return (*l) << ll;
}

#endif // LOGGING_HXX
