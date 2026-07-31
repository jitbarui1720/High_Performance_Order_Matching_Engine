#pragma once
#include "Order.hpp"
struct Trade {
    OrderId buyOrderId{};
    OrderId sellOrderId{};
    Price price{};
    Quantity quantity{};
};
