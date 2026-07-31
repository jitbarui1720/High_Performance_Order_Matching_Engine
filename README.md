# High_Performance_Order_Matching_Engine


A compact exchange-style limit order book and matching engine written in **C++20**. It is designed as a systems/low-latency learning project: correctness first, then measurable performance.

## Features
- Price-time priority (FIFO within each price level)
- Limit and market orders
- IOC (Immediate-or-Cancel) and FOK (Fill-or-Kill)
- Partial fills and multi-level matching
- Execution at the resting order's price
- O(1)-average order-ID lookup plus iterator-based cancellation
- Order modification (cancel/replace semantics, losing time priority)
- Integer price ticks (no floating-point price comparisons)
- Unit tests and a 1M-order benchmark target
- CMake build
- Thread-safe queue primitive included for the networking/concurrency extension

## Core design

```
Incoming Order
     |
     v
+------------------+
|    OrderBook     |
| matching engine  |
+--------+---------+
         |
   +-----+-----+
   |           |
 BIDS         ASKS
(high->low)  (low->high)
   |           |
   +-----+-----+
         |
         v
       Trades
```

Price levels use `std::map`; each level uses `std::list<Order>` to preserve FIFO and permit O(1) erasure when an iterator is known. An `std::unordered_map<OrderId, Location>` indexes active orders for average O(1) lookup.

## Complexity
| Operation | Complexity |
|---|---|
| Best bid / ask | O(1) iterator access |
| Add new/resting order | O(log P) |
| Order-ID lookup | O(1) average |
| Cancel after lookup | O(log P) price-level lookup + O(1) list erase |
| Matching | proportional to price levels/orders consumed |

`P` is the number of active price levels.

## Build

### Linux/macOS
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/matching_engine
ctest --test-dir build --output-on-failure
./build/orderbook_benchmark 1000000
```

### Windows (Visual Studio CMake)
```powershell
cmake -S . -B build
cmake --build build --config Release
.\\build\\Release\\matching_engine.exe
.\\build\\Release\\orderbook_tests.exe
.\\build\\Release\\orderbook_benchmark.exe 1000000
```

## Example
Resting asks: 20 @ 501.00, 40 @ 502.00. An incoming BUY for 50 @ 502.00 generates two fills: 20 @ 501.00 and 30 @ 502.00. The second resting ask remains with quantity 10.

## Benchmarking notes
Run benchmarks on your own machine and report the actual hardware/compiler/build type. Do **not** put invented latency or throughput numbers on a resume. For serious latency measurements, add per-order timing, median/p95/p99 reporting, CPU pinning, warm-up runs, and allocation profiling.

## Concurrency / networking extension
The matching core is intentionally single-threaded. This is a common and explainable design because a single writer gives deterministic ordering and avoids locks in the critical matching path. `ThreadSafeQueue.hpp` is provided as a primitive for a future architecture:

```
TCP clients -> gateway/parser threads -> queue -> single matching thread -> trade/event queue -> publishers
```

A production-style next step is to add an Asio TCP gateway and keep the book owned by exactly one matching thread.

## Interview discussion points
Be ready to explain: why integer ticks beat `double`; why bids and asks need opposite ordering; FIFO price-time priority; `map` vs `unordered_map`; why a list + iterator helps cancellation; cancel/replace semantics; single-writer architecture; cache locality limitations of node-based STL containers; and how you would benchmark tail latency.

## Resume bullets (replace benchmark placeholders with real measurements)
- Engineered a C++20 exchange-style limit order book supporting limit/market, IOC/FOK, partial fills, cancellation and price-time-priority matching.
- Designed ordered price levels with FIFO queues and an order-ID index for efficient best-price access and average O(1) order lookup.
- Benchmarked the engine on 1M synthetic orders, achieving **[YOUR RESULT] orders/sec** on **[YOUR CPU/compiler]**, and analyzed allocation/cache bottlenecks.

## Further improvements
- Asio TCP order gateway and binary protocol
- Per-order latency histogram (median/p95/p99)
- Object pool / arena allocator
- Flat price-level structures for improved cache locality
- Market-data snapshots/incremental events
- Persistence/replay log
- Property/fuzz testing
