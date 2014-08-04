#ifndef MULTITARGET_HXX
#define MULTITARGET_HXX

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

#include <iostream>
#include <stdexcept>
#include "logging.hxx"

namespace Logging 
{
    /**
    * Pseudo-target wrapping two or more other targets to support
    * target switching.
    * 
    * TODO explain in detail the use, esp. the fact, that SubTargets has to be a list of std::shared_ptr
    */
    template <typename... SubTargets> class MultiTarget
    {
        unsigned int mActiveTarget;
        
        /**
        * internal class implementing the target dispatch
        */
        template <unsigned int targetIndex, typename Target, typename... Remaining> class TargetHolder
            : public TargetHolder<targetIndex+1, Remaining...>
        {
            Target mTarget;
        public:
            
            TargetHolder(Target const &target, Remaining... remaining)
                : TargetHolder<targetIndex+1, Remaining...>(remaining...), mTarget(target)
            {
            }
            
            template <typename LoggerT> void startMessage(unsigned int index, LoggerT const &source, TraceLevel tl)
            {
                if (index == targetIndex) {
                    mTarget->startMessage(source, tl);
                }
                else {
                    TargetHolder<targetIndex+1, Remaining...>::startMessage(index, source, tl);
                }
            }

            template <typename LoggerT> void startMessage(unsigned int index, LoggerT const &source, LogLevel ll)
            {
                if (index == targetIndex) {
                    mTarget->startMessage(source, ll);
                }
                else {
                    TargetHolder<targetIndex+1, Remaining...>::startMessage(index, source, ll);
                }
            }            
            
            template <typename LoggerT> void endMessage(unsigned int index, LoggerT const &source)
            {
                if (index == targetIndex) {
                    mTarget->endMessage(source);
                }
                else {
                    TargetHolder<targetIndex+1, Remaining...>::endMessage(index, source);
                }
            }
            
            template <typename LoggerT, typename ValueT> void put(unsigned int index, LoggerT const &source, ValueT const &v)
            {
                if (index == targetIndex) {
                    mTarget->put(source, v);
                }
                else {
                    TargetHolder<targetIndex+1, Remaining...>::put(index, source, v);
                }
            }

            void put(unsigned int index, std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
            {
                if (index == targetIndex) {
                    mTarget->put(manip);
                }
                else {
                    TargetHolder<targetIndex+1, Remaining...>::put(index, manip);
                }
            }
        };
        
        /**
         * Specialization stopping the recursive instantiation of the TargetHolder template
         */
        template <unsigned int targetIndex, typename Target> class TargetHolder<targetIndex, Target>
        {
            Target mTarget;
            
        public:
            
            TargetHolder(Target const &target)
                : mTarget(target)
            {
            }
            
            template <typename LoggerT> void startMessage(unsigned int, LoggerT const &source, TraceLevel tl)
            {
                mTarget->startMessage(source, tl);
            }

            template <typename LoggerT> void startMessage(unsigned int, LoggerT const &source, LogLevel ll)
            {
                mTarget->startMessage(source, ll);
            }
            
            template <typename LoggerT> void endMessage(unsigned int, LoggerT const &source)
            {
                mTarget->endMessage(source);
            }

            template <typename LoggerT, typename ValueT> void put(unsigned int, LoggerT const &source, ValueT const &v)
            {
                mTarget->put(source, v);
            }

            void put(unsigned int, std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
            {
                mTarget->put(manip);
            }
            
            inline unsigned int maxIndex() const
            {
                return targetIndex;
            }
        };
        
        TargetHolder<0, SubTargets...> mSubtargets;
        
    public:
        
        MultiTarget(SubTargets... targets)
            : mActiveTarget(0), mSubtargets(targets...)
        {
        }
        
        template <typename LoggerType> void startMessage(LoggerType const &source, TraceLevel tl)
        {
            mSubtargets.startMessage(mActiveTarget, source, tl);
        }

        template <typename LoggerType> void startMessage(LoggerType const &source, LogLevel ll)
        {
            mSubtargets.startMessage(mActiveTarget, source, ll);
        }
        
        template <typename LoggerType> void endMessage(LoggerType const &source)
        {
            mSubtargets.endMessage(mActiveTarget, source);
        }

        template <typename LoggerType, typename ValueType> void put(LoggerType const &source, ValueType const &v)
        {
            mSubtargets.put(mActiveTarget, source, v);
        }
        
        /**
        * Output stuff like std::endl to the underlying stream.
        * 
        * \param manip The manipulator to output to the underlying stream.
        */
        void put(std::basic_ostream<char>& (*manip)(std::basic_ostream<char>&))
        {
            mSubtargets.put(mActiveTarget, manip);
        }
        
        void setActive(unsigned int index)
        {
            if (index > mSubtargets.maxIndex()) {
                throw std::runtime_error(std::string("Could not active target with index ")+std::to_string(index)+
                                        ". Maximum is "+std::to_string(mSubtargets.maxIndex()));
            }
            mActiveTarget = index;
        }
    };
    
    /**
     * specialization of the target traits for MultiTarget.
     * Due to the nature of MultiTarget, it only supports the common feature,
     * every target has to support: char-output.
     */
    template <typename... SubTargets> struct TargetTraits<MultiTarget<SubTargets...>> 
    {
        typedef char char_type;
        typedef std::char_traits<char> char_traits_type;
    };   
}

#endif // MULTITARGET_HXX
