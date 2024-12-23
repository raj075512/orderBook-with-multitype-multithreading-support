# Orderbook Matching Engine

This project implements a robust **Orderbook Matching Engine** in C++ for managing and matching buy and sell orders in a financial trading system. The engine supports various order types, facilitates order management, and handles trade execution efficiently.

---

## Features

### Core Functionality
- **Order Types**:
  - `GoodTillCancel`: Order remains active until explicitly canceled.
  - `FillAndKill`: Executes immediately as much as possible, cancels the rest.
  - `FillOrKill`: Executes entirely or not at all.
  - `GoodForDay`: Valid for the current trading day.
  - `Market`: Executes immediately at the best available price.

- **Order Sides**:
  - `Buy`
  - `Sell`

### Order Management
- **AddOrder**:
  - Adds a new order to the orderbook.
  - Automatically matches orders if possible.

- **CancelOrder**:
  - Cancels an order by its unique ID.

- **ModifyOrder** (via `MatchOrder`):
  - Modifies an existing order’s price and/or quantity.
  - Internally cancels the original order and re-adds it with updated attributes.

### Matching Engine
- Matches orders based on price and time priority.
- Executes trades when a buy and sell order overlap in price.
- Generates trade details including order IDs, price, and quantity.

### Orderbook Insights
- **GetOrderInfos**:
  - Retrieves detailed bid and ask levels, including total quantities at each price.

- **Size**:
  - Returns the total number of active orders in the orderbook.

---

## Class Structure

### Core Classes

1. **Order**:
   - Represents an individual order with attributes such as price, quantity, and side.

2. **Orderbook**:
   - Manages buy and sell orders.
   - Handles matching, adding, modifying, and canceling orders.

3. **OrderModify**:
   - Facilitates the modification of existing orders.

4. **Trade**:
   - Stores details of executed trades.

5. **OrderbookLevelInfos**:
   - Provides aggregated bid and ask information.

---

## Example Usage

### Adding and Matching Orders
```cpp
Orderbook orderbook;

// Add Buy and Sell Orders
orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 1, Side::Buy, 100, 10));
orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 2, Side::Sell, 110, 5));

std::cout << "Initial Orderbook Size: " << orderbook.Size() << "\n";

// Add Matching Order
auto trades = orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, 3, Side::Sell, 100, 5));

// Print Trade Details
for (const auto& trade : trades) {
    std::cout << "Trade: BuyOrderId=" << trade.GetBidTrade().orderId_
              << " SellOrderId=" << trade.GetAskTrade().orderId_
              << " Price=" << trade.GetBidTrade().price_
              << " Quantity=" << trade.GetBidTrade().quantity_ << "\n";
}
