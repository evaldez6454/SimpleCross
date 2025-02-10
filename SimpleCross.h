#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include "SortedBookKeeper.h"

enum CommandType
{
    O = 0,
    X,
    P,
    E // Error
};
typedef std::list<std::string> results_t;

class SimpleCross
{
public:
    SimpleCross()
    {
        m_SalesBook = std::shared_ptr<SortedBookKeeper>(new SortedBookKeeper(TradeAction::Sell));
        m_BuyingBook = std::shared_ptr<SortedBookKeeper>(new SortedBookKeeper(TradeAction::Buy));
    };
    results_t action(const std::string& line);
    bool parseLine(BookedItem& Item, const std::string& line, CommandType& Command, std::string& ErrReason);
    void HandleTransaction(BookedItem& Item, results_t& resList);
    void HandlePrint(results_t& resList);
    void HandleDelete(uint16_t TargetOrder);

    std::shared_ptr<SortedBookKeeper> m_SalesBook;
    std::shared_ptr<SortedBookKeeper> m_BuyingBook;
    std::set<uint16_t> m_AllOrders;

};