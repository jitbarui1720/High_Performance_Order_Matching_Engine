#pragma once
#include <cstdint>
#include <string>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET, IOC, FOK };
using OrderId = std::uint64_t;
using Price = std::int64_t;       // integer ticks/cents
using Quantity = std::uint32_t;

struct Order {
    OrderId id{};
    Side side{Side::BUY};
    OrderType type{OrderType::LIMIT};
    Price price{};
    Quantity quantity{};
    Quantity remainingQuantity{};

    Order() = default;
    Order(OrderId id_, Side side_, OrderType type_, Price price_, Quantity qty_)
        : id(id_), side(side_), type(type_), price(price_), quantity(qty_), remainingQuantity(qty_) {}
};
