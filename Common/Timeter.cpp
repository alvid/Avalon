#include "Timeter.hpp"

std::tuple<seconds, milliseconds, microseconds> split_duration(duration<double> const& dt)
{
    microseconds us = duration_cast<microseconds>(dt);
    milliseconds ms = duration_cast<milliseconds>(us % seconds(1));
    seconds ss = duration_cast<seconds>(us % minutes(1));
    us = duration_cast<microseconds>(us % milliseconds(1));
    return std::make_tuple(ss, ms, us);
}

