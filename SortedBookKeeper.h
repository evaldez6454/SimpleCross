#pragma once
#include <unordered_map>
#include <string>
#include <map>
#include <set>
#include "BookedItem.h"



class SortedBookKeeper
{
public:
	SortedBookKeeper(TradeAction ActionType);
	~SortedBookKeeper();

	bool QueryEntry(BookedItem& Request); // This method should be called before adding or extracting entries
	bool QueryOrder(BookedItem& Request);
	std::list<std::string> modifyEntry(BookedItem& Request);
	void addEntry(BookedItem& NewEntry);
	void eraseEntry(uint16_t orderId, std::map<double, std::set<uint16_t>>& firstMap, std::map<double, std::set<uint16_t>>::reverse_iterator& iterator);
	void eraseEntry(uint16_t orderId, std::map<double, std::set<uint16_t>>& firstMap, std::map<double, std::set<uint16_t>>::iterator& iterator);
	void PrintTree(std::list<std::string>& RetList);
	void CancelOrder(uint16_t OrderId);

private:
	// Unordered map for SYMBOL ID
	// std::map for price of action
	// std::set for either the timestamp or the Id of the order (these can be used as timestamp proxies)
	std::unordered_map<std::string, std::map<double, std::set<uint16_t>>> m_Book;
	std::unordered_map<uint16_t, BookedItem> m_BookedItems;
	const TradeAction m_BookType;
	std::set<uint16_t> m_AllOrders;
	std::map<uint16_t, double> m_priceOrders;
	std::map<uint16_t, std::string> m_symbolOrders;
};

