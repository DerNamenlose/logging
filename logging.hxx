#ifndef LOGGING_HXX
#define LOGGING_HXX

/*
Copyright (c) 2014, Markus Brueckner <namenlos@geekbetrieb.de>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither my name nor the names of any contributors may be used to endorse
      or promote products derived from this software without specific prior
      written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL I, THE AUTHOR OF THIS SOFTWARE, BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string>
#include <map>
#include <memory>

namespace Logging {
    /**
    * The two trace levels. Those two messages are optimized out in
    * release code.
    */
    enum TraceLevel
    {
        LEVEL_TRACE,
        LEVEL_DEBUG
    };

    /**
    * Log levels. Messages with those levels will never be optimized out,
    * even in release code.
    */
    enum LogLevel
    {
        LEVEL_INFO = LEVEL_DEBUG + 1,
        LEVEL_WARNING,
        LEVEL_ERROR,
        LEVEL_FATAL
    };

#ifndef NDEBUG
#define TRACING false
#else
#define TRACING true
#endif

    /**
    * Log sentry object guarding start and finish of a log message
    *
    * \tparam Target The log target to forward the messages to.
    * \tparam outputEnabled Define whether this sentry outputs anything at all. The
    *                      specialization of this class for <tt>outputEnabled == false</tt>
    *                      is an empty shell for being optimized out.
    * \tparam LoggerType The type of the logger that created this sentry.
    */
    template <
        typename Target,
        bool outputEnabled,  // Is the output enabled?
        typename LoggerType
        > class LogSentry
    {
        template <typename TargetType, bool trace, typename TargetTraits> friend class Logger;

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

        LogSentry(LogSentry const &) = default;

        ~LogSentry()
        {
            if (mEnabled) {
                mTarget.endMessage(mSource);
            }
        }

        /**
        * Output a value to the logger.
        *
        * \param v The value to output.
        */
        template <typename ValueT> inline LogSentry &operator<<(ValueT const &v)
        {
            if (mEnabled) {
                mTarget.put(mSource, v);
            }
            return *this;
        }

        /**
        * enable output like "<< std::endl" etc.
        *
        * \param manip The output manipulator to be output to the target.
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
        template <typename TargetType, bool trace, typename TargetTraits> friend class Logger;

        /**
        * constructor for starting a log message with a trace level
        *
        * \internal
        *            This class only needs the trace level constructor, as it will never be created with
        *            a log level input
        * \endinternal
        */
        LogSentry(Target &, LoggerType const &, TraceLevel, bool)
        {
        }


    public:

        /**
         * output anything. This method is just an empty shell to be optimized away.
         */
        template <typename ValueT> inline LogSentry &operator<<(ValueT const &)
        {
            return *this;
        }

        /**
        * enable output like "<< std::endl" etc.
        */
        LogSentry &operator<<(std::basic_ostream<typename LoggerType::TargetTraitsType::char_type,
                                                typename LoggerType::TargetTraitsType::char_traits_type> &(*)(
                                                    std::basic_ostream<typename LoggerType::TargetTraitsType::char_type,
                                                    typename LoggerType::TargetTraitsType::char_traits_type> &))
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
    *
    * \tparam Target the type of log target to use. See OStreamTarget for an example.
    * \tparam trace indicate, whether tracing is enabled. If this parameter is false, all
    *              messages with levels TRACE and DEBUG will be compiled in a way, that
    *              modern compilers will be able to optimize them out. You will need to
    *              enable optimization in you compilation process (e.g. at least -O for
    *              GCC or Clang).
    *              The logging framework will try to define the value of Tracing based on
    *              whether NDEBUG is set (i.e. we're being compiled in release mode) or
    *              not. You can therefore use this variable to enable/disable Tracing
    *              at compilation time like this:
    *              \code
    *                   Logging::Logger<TargetType, !TRACING>
    *              \endcode
    * \tparam TargetTraits The traits object defining some necessary information on the
    *              log target. Defaults to TargetTraits<Target>.
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
        *
        * \param name the name of this log object
        * \param parent the parent of this object
        * \param level the initial minimum message of this logger
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
        * Start a new log message.
        *
        * \param ll The log level of this message.
        * \return A LogSentry-object forwarding the rest of the message to the log target
        *          (depending on the message level)
        */
        inline LogSentry<Target, true, Logger> operator<<(LogLevel ll)
        {
            return LogSentry<Target,
                            true,  // Log messages are always forwarded
                            Logger>(*mTarget, *this, ll, ll >= mLevel);
        }

        /**
        * Start a new trace message. Depending on you compilation configuration
        * this call will be optimized away (with tracing disabled).
        *
        * \param tl The trace level of this message.
        * \return A LogSentry-object forwarding the rest of the message to the log target
        *          (depending on the message level)
        */
        inline LogSentry<Target, trace, Logger> operator<<(TraceLevel tl)
        {
            return LogSentry<Target,
                            trace,  // trace output is decided in the sentry
                            Logger>(*mTarget, *this, tl, tl >= mLevel);
        }

        /**
        * Get the name of this logger.
        *
        * \return The name of this logger relative to its parent.
        */
        inline std::string const &name() const
        {
            return mName;
        }

        /**
        * Return the parent logger of this object (if any).
        *
        * \return The parent logger of this object or nullptr, if this is a
        *          root logger.
        */
        Logger *parent() const
        {
            return mParent;
        }

        /**
        * Set the minimum log level of this object. Messages have to at least have this
        * level to be forwarded to the log target. This call will <em>also set the levels
        * of all child-loggers!</em>
        *
        * \param level The log level to set must be one of the values from TraceLevel or
        *              LogLevel.
        */
        void setLevel(unsigned char level)
        {
            mLevel = level;
            for (auto i : mChildren) {
                i.second->setLevel(level);
            }
        }

        /**
         * get the current minimum log level of this logger object
         *
         * \return the currently configured level of the logger
         */
        unsigned char level() const
        {
            return mLevel;
        }

        /**
         * Check whether a message of the given level would be output by the logger (i.e.
         *  the given level is bigger, than the configured minimum of this object). Use this
         *  to check, whether a specific log message would be output before preparing big
         *  objects for output. If you, for example, would output a whole std::vector of complex
         *  objects in a loop, you could beforehand check, whether there will be any output
         *  at all and skip the loop, when necessary.
         *
         * \param level The level to check
         * \return true, if a message of this level would be logged, false otherwise.
         */
        bool isEnabled(LogLevel level) const
        {
            return level >= mLevel;
        }

        /**
         * Check whether a message of the given trace level would be output by the logger (i.e.
         *  the given level is bigger, than the configured minimum of this object)
         *
         * \param level The level to check
         * \return true, if a message of this level would be logged, false otherwise.
         * \note This will always return false in release code.
         */
        bool isEnabled(TraceLevel level) const
        {
            return (level >= mLevel) && trace;
        }

        /**
        * Get a child logger of the current logger. This method will return a
        * logger object with the same configuration as the object this is called on
        * apart from the name and the parent and child loggers (obviously). If no child
        * logger with this name already exists, a new object is created and stored
        * as a child of this logger.
        *
        * \param name The name of this sub-logger. If a logger with this name does already exist,
        *             the object is returned. The name cannot be empty.
        * \return A logger object with the given name and the same configuration as the
        *          parent logger.
        */
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

        /**
         * get the target currently associated with this Logger
         *
         * \return a shared_ptr to the target object associated with this logger
         */
        std::shared_ptr<Target> const &target() const
        {
            return mTarget;
        }

        /**
         * set the target object associated with this logger <em>and all its children</em>
         */
        void setTarget(std::shared_ptr<Target> const &t)
        {
            mTarget = t;
            for (auto &child : mChildren) {
                child.second->setTarget(t);
            }
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
                -> decltype((*l) << ll)
    {
        return (*l) << ll;
    }
}

#endif // LOGGING_HXX
