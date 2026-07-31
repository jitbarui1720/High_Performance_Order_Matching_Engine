#include "OrderBook.hpp"
#include <algorithm>
#include <stdexcept>

std::optional<Price> OrderBook::bestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}
std::optional<Price> OrderBook::bestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

void OrderBook::rest(Order order) {
    if (order.remainingQuantity == 0) return;
    if (locations_.contains(order.id)) throw std::invalid_argument("duplicate order id");
    if (order.side == Side::BUY) {
        auto& level = bids_[order.price];
        level.push_back(order);
        auto it = std::prev(level.end());
        locations_.emplace(order.id, Location{Side::BUY, order.price, it});
    } else {
        auto& level = asks_[order.price];
        level.push_back(order);
        auto it = std::prev(level.end());
        locations_.emplace(order.id, Location{Side::SELL, order.price, it});
    }
}

bool OrderBook::canFullyFill(const Order& order) const {
    std::uint64_t available = 0;
    if (order.side == Side::BUY) {
        for (const auto& [price, queue] : asks_) {
            if (order.type != OrderType::MARKET && price > order.price) break;
            for (const auto& resting : queue) available += resting.remainingQuantity;
            if (available >= order.remainingQuantity) return true;
        }
    } else {
        for (const auto& [price, queue] : bids_) {
            if (order.type != OrderType::MARKET && price < order.price) break;
            for (const auto& resting : queue) available += resting.remainingQuantity;
            if (available >= order.remainingQuantity) return true;
        }
    }
    return available >= order.remainingQuantity;
}

std::vector<Trade> OrderBook::matchBuy(Order& incoming) {
    std::vector<Trade> trades;
    while (incoming.remainingQuantity && !asks_.empty()) {
        auto levelIt = asks_.begin();
        Price executionPrice = levelIt->first;
        if (incoming.type != OrderType::MARKET && executionPrice > incoming.price) break;
        auto& queue = levelIt->second;
        while (incoming.remainingQuantity && !queue.empty()) {
            auto restingIt = queue.begin();
            Quantity qty = std::min(incoming.remainingQuantity, restingIt->remainingQuantity);
            trades.push_back(Trade{incoming.id, restingIt->id, executionPrice, qty});
            incoming.remainingQuantity -= qty;
            restingIt->remainingQuantity -= qty;
            if (restingIt->remainingQuantity == 0) {
                locations_.erase(restingIt->id);
                queue.erase(restingIt);
            }
        }
        if (queue.empty()) asks_.erase(levelIt);
    }
    return trades;
}

std::vector<Trade> OrderBook::matchSell(Order& incoming) {
    std::vector<Trade> trades;
    while (incoming.remainingQuantity && !bids_.empty()) {
        auto levelIt = bids_.begin();
        Price executionPrice = levelIt->first;
        if (incoming.type != OrderType::MARKET && executionPrice < incoming.price) break;
        auto& queue = levelIt->second;
        while (incoming.remainingQuantity && !queue.empty()) {
            auto restingIt = queue.begin();
            Quantity qty = std::min(incoming.remainingQuantity, restingIt->remainingQuantity);
            trades.push_back(Trade{restingIt->id, incoming.id, executionPrice, qty});
            incoming.remainingQuantity -= qty;
            restingIt->remainingQuantity -= qty;
            if (restingIt->remainingQuantity == 0) {
                locations_.erase(restingIt->id);
                queue.erase(restingIt);
            }
        }
        if (queue.empty()) bids_.erase(levelIt);
    }
    return trades;
}

std::vector<Trade> OrderBook::submit(Order order) {
    if (order.id == 0 || order.quantity == 0) throw std::invalid_argument("invalid order");
    if (locations_.contains(order.id)) throw std::invalid_argument("duplicate order id");
    if (order.type == OrderType::FOK && !canFullyFill(order)) return {};

    auto trades = order.side == Side::BUY ? matchBuy(order) : matchSell(order);
    const bool mayRest = order.type == OrderType::LIMIT;
    if (order.remainingQuantity && mayRest) rest(order);
    // MARKET and IOC discard unfilled quantity. FOK is pre-validated and therefore fully fills.
    return trades;
}

bool OrderBook::cancel(OrderId id) {
    auto locIt = locations_.find(id);
    if (locIt == locations_.end()) return false;
    auto loc = locIt->second;
    if (loc.side == Side::BUY) {
        auto levelIt = bids_.find(loc.price);
        levelIt->second.erase(loc.it);
        if (levelIt->second.empty()) bids_.erase(levelIt);
    } else {
        auto levelIt = asks_.find(loc.price);
        levelIt->second.erase(loc.it);
        if (levelIt->second.empty()) asks_.erase(levelIt);
    }
    locations_.erase(locIt);
    return true;
}

std::vector<Trade> OrderBook::modify(OrderId id, Price newPrice, Quantity newQuantity) {
    auto locIt = locations_.find(id);
    if (locIt == locations_.end() || newQuantity == 0) return {};
    Order replacement = *(locIt->second.it);
    replacement.price = newPrice;
    replacement.quantity = newQuantity;
    replacement.remainingQuantity = newQuantity;
    cancel(id); // modification loses time priority
    return submit(replacement);
}
