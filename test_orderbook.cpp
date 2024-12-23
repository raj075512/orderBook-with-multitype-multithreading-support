#include<iostream>
#include<vector>
#include<string>
#include<list>
#include<cmath>
#include<chrono>
#include<algorithm>
#include<numeric>
#include<tuple>

#include<optional>
#include<variant>
#include<memory>
#include<limits>
#include<stack>
#include<queue>
#include<set>
#include<map>

using namespace std;




enum class Side
{
    Buy,
    Sell
};





enum class OrderType
{
    GoodTillCancel,
    FillAndKill,
    FillOrKill,
    GoodForDay,
    Market,
};

using Price =std::int32_t;
using Quantity=std::uint32_t;
using OrderId=std::uint64_t;


struct LevelInfo
{
    // OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

struct TradeInfo
{
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};


using LevelInfos=std::vector<LevelInfo>;



class OrderbookLevelInfos
{
public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : bids_{ bids }
        , asks_{ asks }
    { }

    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }

private:
    LevelInfos bids_;
    LevelInfos asks_;
};




class Order
{
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
        : orderType_{ orderType }
        , orderId_{ orderId }
        , side_{ side }
        , price_{ price }
        , initialQuantity_{ quantity }
        , remainingQuantity_{ quantity }
    { }

    // Order(OrderId orderId, Side side, Quantity quantity)
    //     : Order(OrderType::Market, orderId, side, Constants::InvalidPrice, quantity)
    // { }

    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    OrderType GetOrderType() const { return orderType_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return GetInitialQuantity() - GetRemainingQuantity(); }
   
    bool IsFilled() const { return GetRemainingQuantity() == 0; }
    void Fill(Quantity quantity)
    {
        // if (quantity > GetRemainingQuantity())
        //     throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));

        remainingQuantity_ -= quantity;
    }
    // void ToGoodTillCancel(Price price) 
    // { 
    //     if (GetOrderType() != OrderType::Market)
    //         throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", GetOrderId()));

    //     price_ = price;
    //     orderType_ = OrderType::GoodTillCancel;
    // }

private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;


class OrderModify
{
public:
    OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
        : orderId_{ orderId }
        , price_{ price }
        , side_{ side }
        , quantity_{ quantity }
    { }

    OrderId GetOrderId() const { return orderId_; }
    Price GetPrice() const { return price_; }
    Side GetSide() const { return side_; }
    Quantity GetQuantity() const { return quantity_; }

    OrderPointer ToOrderPointer(OrderType type) const
    {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderId orderId_;
    Price price_;
    Side side_;
    Quantity quantity_;
};


class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade_{ bidTrade }
        , askTrade_{ askTrade }
    { }

    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;



class Orderbook
{
private:

    struct OrderEntry
    {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
    };

    struct LevelData
    {
        Quantity quantity_{ };
        Quantity count_{ };

        enum class Action
        {
            Add,
            Remove,
            Match,
        };
    };

    // std::unordered_map<Price, LevelData> data_;
    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;
    // mutable std::mutex ordersMutex_;
    // std::thread ordersPruneThread_;
    // std::condition_variable shutdownConditionVariable_;
    // std::atomic<bool> shutdown_{ false };
    
    bool CanMatch(Side side, Price price) const
    {
        if (side == Side::Buy)
        {
            if (asks_.empty())
                return false;

            const auto& [bestAsk, _] = *asks_.begin();
            return price >= bestAsk;
        }
        else
        {
            if (bids_.empty())
                return false;

            const auto& [bestBid, _] = *bids_.begin();
            return price <= bestBid;
        }
    }


    // void PruneGoodForDayOrders();

    // void CancelOrders(OrderIds orderIds);
    // void CancelOrderInternal(OrderId orderId);

    // void OnOrderCancelled(OrderPointer order);
    // void OnOrderAdded(OrderPointer order);
    // void OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
    // void UpdateLevelData(Price price, Quantity quantity, LevelData::Action action);

    // bool CanFullyFill(Side side, Price price, Quantity quantity) const;
    


    Trades MatchOrders()
    {
        Trades trades;
        trades.reserve(orders_.size());

        while (true)
        {
            if (bids_.empty() || asks_.empty())
                break;

            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

            if (bidPrice < askPrice)
                break;

            while (!bids.empty() && !asks.empty())
            {
                auto bid = bids.front();
                auto ask = asks.front();

                Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                bid->Fill(quantity);
                ask->Fill(quantity);

                if (bid->IsFilled())
                {
                    bids.pop_front();
                    orders_.erase(bid->GetOrderId());
                }

                if (ask->IsFilled())
                {
                    asks.pop_front();
                    orders_.erase(ask->GetOrderId());
                }
                 if (bids.empty())
                {
                    bids_.erase(bidPrice);
                    // data_.erase(bidPrice);
                }

                if (asks.empty())
                {
                    asks_.erase(askPrice);
                    // data_.erase(askPrice);
                }


                trades.push_back(Trade{
                    TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                    TradeInfo{ ask->GetOrderId(), ask->GetPrice(), quantity } 
                    });

                // OnOrderMatched(bid->GetPrice(), quantity, bid->IsFilled());
                // OnOrderMatched(ask->GetPrice(), quantity, ask->IsFilled());
            } // while ends //

            // if (bids.empty())
            // {
            //     bids_.erase(bidPrice);
            //     // data_.erase(bidPrice);
            // }

            // if (asks.empty())
            // {
            //     asks_.erase(askPrice);
            //     // data_.erase(askPrice);
            // }
        }

        if (!bids_.empty())
        {
            auto& [_, bids] = *bids_.begin();
            auto& order = bids.front();
            if (order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        if (!asks_.empty())
        {
            auto& [_, asks] = *asks_.begin();
            auto& order = asks.front();
            if (order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        return trades;
    }


public:

    // Orderbook();
    // Orderbook(const Orderbook&) = delete;
    // void operator=(const Orderbook&) = delete;
    // Orderbook(Orderbook&&) = delete;
    // void operator=(Orderbook&&) = delete;
    // ~Orderbook();

    Trades AddOrder(OrderPointer order)
{
    // std::scoped_lock ordersLock{ ordersMutex_ };

    if (orders_.count(order->GetOrderId()))
        return { };

    // if (order->GetOrderType() == OrderType::Market)
    // {
    //     if (order->GetSide() == Side::Buy && !asks_.empty())
    //     {
    //         const auto& [worstAsk, _] = *asks_.rbegin();
    //         order->ToGoodTillCancel(worstAsk);
    //     }
    //     else if (order->GetSide() == Side::Sell && !bids_.empty())
    //     {
    //         const auto& [worstBid, _] = *bids_.rbegin();
    //         order->ToGoodTillCancel(worstBid);
    //     }
    //     else
    //         return { };
    // }

    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->GetSide(), order->GetPrice()))
        return { };

    // if (order->GetOrderType() == OrderType::FillOrKill && !CanFullyFill(order->GetSide(), order->GetPrice(), order->GetInitialQuantity()))
    //     return { };

    OrderPointers::iterator iterator;

    if (order->GetSide() == Side::Buy)
    {
        auto& orders = bids_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.begin(),orders.size()-1);
    }
    else
    {
        auto& orders = asks_[order->GetPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.begin(), orders.size()-1);
    }

    orders_.insert({ order->GetOrderId(), OrderEntry{ order, iterator } });
    
    // OnOrderAdded(order);
    
    return MatchOrders();

}






void CancelOrder(OrderId orderId)
{
        // std::scoped_lock ordersLock{ ordersMutex_ };
//  
         if (!orders_.count(orderId))
        return;

    const auto [order, iterator] = orders_.at(orderId);
    orders_.erase(orderId);

    if (order->GetSide() == Side::Sell)
    {
        auto price = order->GetPrice();
        auto& orders = asks_.at(price);
        orders.erase(iterator);
        if (orders.empty())
            asks_.erase(price);
    }
    else
    {
        auto price = order->GetPrice();
        auto& orders = bids_.at(price);
        orders.erase(iterator);
        if (orders.empty())
            bids_.erase(price);
    }

        // CancelOrderInternal(orderId);
}

Trades MatchOrder(OrderModify order)
{
    if (!orders_.contains(order.GetOrderId())) // Correct the usage here
    {
        return { };
    }
    const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
}




    // Trades ModifyOrder(OrderModify order)
    // {
    //     // OrderType orderType;
        
    //      if (!orders_.contains(order.GetOrderId()))
    //             return { };
        
    //      const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
        

    //     // {
    //     //     std::scoped_lock ordersLock{ ordersMutex_ };

    //     //     if (!orders_.contains(order.GetOrderId()))
    //     //         return { };

    //     //     const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
    //     //     orderType = existingOrder->GetOrderType();
    //     // }

    //     CancelOrder(order.GetOrderId());
    //     return AddOrder(order.ToOrderPointer(orderType));
    // }

    
    std::size_t Size() const{
        return orders_.size();
    }

    OrderbookLevelInfos GetOrderInfos() const
    {
        LevelInfos bidInfos, askInfos;
        bidInfos.reserve(orders_.size());
        askInfos.reserve(orders_.size());

        auto CreateLevelInfos = [](Price price, const OrderPointers& orders)
        {
            return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
                [](Quantity runningSum, const OrderPointer& order)
                { return runningSum + order->GetRemainingQuantity(); }) };
        };

        for (const auto& [price, orders] : bids_)
            bidInfos.push_back(CreateLevelInfos(price, orders));

        for (const auto& [price, orders] : asks_)
            askInfos.push_back(CreateLevelInfos(price, orders)); 

        return OrderbookLevelInfos{ bidInfos, askInfos };

    }





};

int main()
{

   Orderbook orderbook;
   const OrderId orderId=1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel,orderId,Side::Buy,100, 10));
    
   std::cout<<orderbook.Size()<<"\n";
   
    orderbook.CancelOrder(orderId);

   cout<<orderbook.Size()<<"\n"; 

    return 0;
}