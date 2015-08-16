#ifndef LOGGING_UTILS
#define LOGGING_UTILS

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

#include "logging.hxx"

namespace Logging
{
    /**
    * Return a string representation of the given trace level
    *
    * \param tl The trace level to transform.
    * \return The string representation of the given trace level.
    */
    inline char const *levelName(TraceLevel tl)
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
    inline char const *levelName(LogLevel ll)
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
/*            default:
                return "UNKNOWN";
                break;*/
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
}

#endif
