//
// Created by Aleksey Dorofeev on 07/04/2020.
//

#ifndef EMPTY_TYPES_HPP
#define EMPTY_TYPES_HPP

#include <chrono>
#include <future>

using Dur = std::chrono::duration<double>;
using Fut = std::future<Dur>;

#include "../../Common/Timeter.hpp"

#endif //EMPTY_TYPES_HPP
