/*
 * Copyright (c) 2016-2018 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ARM_COMPUTE_LOGGING_TYPES_H__
#define __ARM_COMPUTE_LOGGING_TYPES_H__

#include <string>

namespace arm_compute
{
namespace logging
{
/** Logging level enumeration */
enum class LogLevel : unsigned int
{
    VERBOSE, /**< All logging messages */
    INFO,    /**< Information log level */
    WARN,    /**< Warning log level */
    ERROR,   /**< Error log level */
    OFF      /**< No logging */
};

struct LogMsg
{
    LogMsg()
        : raw_(), log_level_(LogLevel::OFF)
    {
    }
    LogMsg(std::string msg, LogLevel log_level = LogLevel::OFF)
        : raw_(msg), log_level_(log_level)
    {
    }

    std::string raw_;
    LogLevel    log_level_;
};
} // namespace logging
} // namespace arm_compute
#endif /* __ARM_COMPUTE_TYPES_H__ */
