#pragma once
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>
#include "Order.hpp"
#include "Trade.hpp"

class OrderBook {
public:
    using OrderList = std::list<Order>;
    using BidBook = std::map<Price, OrderList, std::greater<Price>>;
    using AskBook = std::map<Price, OrderList>;

    std::vector<Trade> submit(Order order);
    bool cancel(OrderId id);
    std::vector<Trade> modify(OrderId id, Price newPrice, Quantity newQuantity);

    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;
    std::size_t activeOrders() const { return locations_.size(); }
    const BidBook& bids() const { return bids_; }
    const AskBook& asks() const { return asks_; }

private:
    struct Location {
        Side side;
        Price price;
        OrderList::iterator it;
    };

    BidBook bids_;
    AskBook asks_;
    std::unordered_map<OrderId, Location> locations_;

    bool canFullyFill(const Order& order) const;
    void rest(Order order);
    std::vector<Trade> matchBuy(Order& incoming);
    std::vector<Trade> matchSell(Order& incoming);
};
