#ifndef OSTREAMLOGGER_HXX
#define OSTREAMLOGGER_HXX

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

#include <ostream>
#include <chrono>
#include <ctime>
#include <string>

#include "logging.hxx"
#include "locking.hxx"
#include "logging_utils.hxx"

namespace Logging
{
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
        bool mPrintTime;
        bool mPrintDate;
        
        void printTimestamp()
        {
            if (mPrintDate || mPrintTime) {
                auto time = std::chrono::system_clock::now();
                auto tp = std::chrono::system_clock::to_time_t(time);
                char buf[128];
                char const *fmt;
                if (mPrintDate) {
                    if (mPrintTime) {
                        fmt = "%F %T";
                    }
                    else {
                        fmt = "%F";
                    }
                }
                else {
                    fmt = "%T";
                }
                std::strftime(buf, 128, "%F %T", std::localtime(&tp));
                mOs << "<" << buf;
                if (mPrintTime) {
                    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count() % 1000; // millisecond part of the equation
                    mOs << "." << millis;
                }
                mOs << "> ";
            }
        }
        
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
            : mOs ( os ), mPrintTime( false ), mPrintDate ( false)
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
        * Start a message from the given logger with the given level.
        * 
        * \param source the logger object, which starts the message.
        * \param ll the level of this message
        * \note This method calls LockType::lock(). It might therefore block until the lock is available.
        */
        template <typename LoggerType> void startMessage(LoggerType const &source, LogLevel tl)
        {
            LockType::lock();
            std::string const &logName = canonicalName(source);
            printTimestamp();
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
        
        /**
         * print the time a log message is started
         * 
         * \param p enable/disable printing. If true, printing is enabled.
         */
        void printTime(bool p)
        {
            mPrintTime = p;
        }

        /**
         * print the date a log message is started
         * 
         * \param p enable/disable printing. If true, printing is enabled.
         */
        void printDate(bool p)
        {
            mPrintDate = p;
        }
        
        /**
         * print time and date of a log message
         * 
         * \param p enable/disable printing. If true, printing is enabled.
         */
        void printTimestamp(bool p)
        {
            printTime(p);
            printDate(p);
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
}

#endif // OSTREAMLOGGER_HXX
