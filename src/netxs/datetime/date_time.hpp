// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_DATE_TIME_HPP
#define NETXS_DATE_TIME_HPP

#include "../text/utf.hpp"

#include <chrono>
#include <ctime>

namespace netxs::datetime
{
    using utf::text;
    using namespace std::chrono;
    // deprecated stuff

    inline text current_time_with_ms           ();
    inline text current_date_time_with_ms      ();
    inline text current_short_date_time_with_ms();
    inline text current_short_date_time_wo_ms  ();
    inline text elapsed_time_to_string         (time_t some_time);
    inline auto elapsed_time_sec               (time_t some_time);
    inline text current_short_date_time        (time_t some_time);
    inline text current_date_time              (time_t some_time);
    inline text current_time                   (time_t some_time);
    inline auto now_ms                         ();
    inline auto get_milliseconds_since_epoch   ();

    text padded_to_string(long number, size_t padding)
    {
        auto txt = std::to_string(number);
        auto len = txt.length() > padding ? 0_sz : padding - txt.length();
        return text(len, '0') + txt;
    }
    text current_time_with_ms()
    {
        auto currentTime = system_clock::now();
        auto time = system_clock::to_time_t(currentTime);
        auto currentTimeRounded = system_clock::from_time_t(time);
        if (currentTimeRounded > currentTime)
        {
            --time;
            currentTimeRounded -= seconds(1);
        }
        int milliseconds = duration_cast<duration<int, std::milli>>(currentTime - currentTimeRounded).count();
        return current_time(time) + '.' + padded_to_string(milliseconds, 3);
    }
    text current_date_time_with_ms()
    {
        auto currentTime = system_clock::now();
        auto time = system_clock::to_time_t(currentTime);
        auto currentTimeRounded = system_clock::from_time_t(time);
        if (currentTimeRounded > currentTime)
        {
            --time;
            currentTimeRounded -= seconds(1);
        }
        int milliseconds = duration_cast<duration<int, std::milli>>(currentTime - currentTimeRounded).count();
        return current_date_time(time) + '.' + padded_to_string(milliseconds, 3);
    }
    text current_short_date_time_with_ms()
    {
        auto currentTime = system_clock::now();
        auto time = system_clock::to_time_t(currentTime);
        auto currentTimeRounded = system_clock::from_time_t(time);
        if (currentTimeRounded > currentTime)
        {
            --time;
            currentTimeRounded -= seconds(1);
        }
        int milliseconds = duration_cast<duration<int, std::milli>>(currentTime - currentTimeRounded).count();
        return current_short_date_time(time) + '.' + padded_to_string(milliseconds, 3);
    }
    text current_short_date_time_wo_ms()
    {
        auto currentTime = system_clock::now();
        auto time = system_clock::to_time_t(currentTime);
        return current_short_date_time(time);
    }
    auto elapsed_time_sec(time_t some_time)
    {
        return std::time(0) - some_time;
    }
    text elapsed_time_to_string(time_t some_time)
    {
        auto elapsed = text{};
        const int cseconds_in_day = 86400;
        const int cseconds_in_hour = 3600;
        const int cseconds_in_minute = 60;

        auto elapsed_seconds = elapsed_time_sec(some_time);

        long long days = elapsed_seconds / cseconds_in_day;
        long hours = (elapsed_seconds % cseconds_in_day) / cseconds_in_hour;
        long minutes = ((elapsed_seconds % cseconds_in_day) % cseconds_in_hour) / cseconds_in_minute;
        long seconds = (((elapsed_seconds % cseconds_in_day) % cseconds_in_hour) % cseconds_in_minute);

        if (days)
        {
            elapsed = std::to_string(days) + " day" + ((days) != 1 ? ("s") : "") + " ";
        }
        elapsed += padded_to_string(hours,   2_sz) + ":"
                 + padded_to_string(minutes, 2_sz) + ":"
                 + padded_to_string(seconds, 2_sz);
        return elapsed;
    }

    text current_date_time(time_t some_time)
    {
        struct tm  tstruct;
        char       buf[80] = "--.--.---- --:--:--";
        if (some_time)
        {
            //tstruct = *localtime(&some_time);
            #if defined(_WIN32)
            localtime_s(&tstruct, &some_time);
            #else
            localtime_r(&some_time, &tstruct);
            #endif
            strftime(buf, sizeof(buf), "%d.%m.%Y %X", &tstruct);
        }
        return buf;
    }
    text current_short_date_time(time_t some_time)
    {
        struct tm  tstruct;
        char       buf[80] = "--.--.-- --:--:--";
        if (some_time)
        {
            //tstruct = *localtime(&some_time);
            #if defined(_WIN32)
            localtime_s(&tstruct, &some_time);
            #else
            localtime_r(&some_time, &tstruct);
            #endif
            strftime(buf, sizeof(buf), "%d.%m.%y %X", &tstruct);
        }
        return buf;
    }
    text current_time(time_t some_time)
    {
        struct tm  tstruct;
        char       buf[80] = "--:--:--";
        if (some_time)
        {
            //tstruct = *localtime(&some_time);
            #if defined(_WIN32)
            localtime_s(&tstruct, &some_time);
            #else
            localtime_r(&some_time, &tstruct);
            #endif
            strftime(buf, sizeof(buf), "%X", &tstruct);
        }
        return buf;
    }
    auto now_ms()
    {
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
    auto get_milliseconds_since_epoch()
    {
        return now_ms();
    }
}

#endif // NETXS_DATE_TIME_HPP