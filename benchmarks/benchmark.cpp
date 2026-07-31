#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include "OrderBook.hpp"

int main(int argc, char** argv) {
    std::uint64_t n = 1'000'000;
    if (argc > 1) n = std::stoull(argv[1]);
    OrderBook book;
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> priceDist(-50, 50);
    std::uniform_int_distribution<int> qtyDist(1, 100);

    auto start = std::chrono::steady_clock::now();
    for (std::uint64_t i = 1; i <= n; ++i) {
        const auto side = sideDist(rng) ? Side::BUY : Side::SELL;
        const Price base = side == Side::BUY ? 10000 : 10001;
        book.submit(Order{i, side, OrderType::LIMIT, base + priceDist(rng), static_cast<Quantity>(qtyDist(rng))});
    }
    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "Orders: " << n << '\n'
              << "Seconds: " << seconds << '\n'
              << "Throughput: " << static_cast<std::uint64_t>(n / seconds) << " orders/sec\n"
              << "Active resting orders: " << book.activeOrders() << '\n';
}
