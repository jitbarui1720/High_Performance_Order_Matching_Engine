#include <iomanip>
#include <iostream>
#include "OrderBook.hpp"

static double money(Price p) { return static_cast<double>(p) / 100.0; }

int main() {
    OrderBook book;
    book.submit(Order{201, Side::SELL, OrderType::LIMIT, 50100, 20});
    book.submit(Order{202, Side::SELL, OrderType::LIMIT, 50200, 40});
    book.submit(Order{203, Side::SELL, OrderType::LIMIT, 50300, 30});

    auto trades = book.submit(Order{301, Side::BUY, OrderType::LIMIT, 50200, 50});
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& t : trades) {
        std::cout << "TRADE buy=" << t.buyOrderId << " sell=" << t.sellOrderId
                  << " price=" << money(t.price) << " qty=" << t.quantity << '\n';
    }
    std::cout << "Best ask: " << (book.bestAsk() ? money(*book.bestAsk()) : 0.0) << '\n';
    std::cout << "Active orders: " << book.activeOrders() << '\n';
}
