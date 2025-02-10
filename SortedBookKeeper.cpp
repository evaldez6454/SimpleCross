#include "SortedBookKeeper.h"

void fillOrder(char* szType, uint16_t OrderId, std::string StockId, uint64_t quantity, double Price)
{
	sprintf(szType, "F %d %s %llu %7.5f", OrderId, StockId.c_str(), quantity, Price);
	return;
};

SortedBookKeeper::SortedBookKeeper(TradeAction ActionType) : m_BookType(ActionType)
{}

SortedBookKeeper::~SortedBookKeeper()
{}

bool SortedBookKeeper::QueryOrder(BookedItem& Request)
{
	if (m_AllOrders.find(Request.get_OrderId()) != m_AllOrders.end())
		return false;

	return true;
}


bool SortedBookKeeper::QueryEntry(BookedItem& Request)
{
	auto key = m_Book.find(Request.get_Id());
	// Test to see if this symbol is in the book yet
	if (key == m_Book.end())
	{
		return false;
	}
	else
	{
		auto firstMap = m_Book[Request.get_Id()];
		// find if the price is already booked
		auto secondKey = firstMap.find(Request.get_Price());

		if (secondKey == firstMap.end())
			return false;
		else
		{
			auto InnerSet = firstMap[Request.get_Price()];
			if (!InnerSet.empty())
				return true;
			else
				return false;
		}
	}
}

std::list<std::string> SortedBookKeeper::modifyEntry(BookedItem& Request)
{
	// Check that we are reducing from the opposite book 
	// i.e. if the book records buy orders the request must be a sell order

	std::list<std::string> retList;
	char szType[128];
	if (Request.get_Action() == m_BookType)
	{
		printf("Error! You requested to remove an item from the wrong book!\n");
		return retList;
	}

	auto key = m_Book.find(Request.get_Id());
	// Test to see if this symbol is in the book yet
	if (key == m_Book.end())
	{
		return retList;
	}
	else
	{
		auto firstMap = m_Book[Request.get_Id()];
		// find if the price is already booked
		auto secondKey = firstMap.find(Request.get_Price());

		// Case where an exact match was found
		if (secondKey != firstMap.end())
		{
			auto InnerSet = firstMap[Request.get_Price()];
			auto Item = m_BookedItems[*(InnerSet.rbegin())];
			uint64_t quantity = Item.get_Quantity();
			if (quantity == Request.get_Quantity())
			{
				auto iter = m_BookedItems.find(*(InnerSet.rbegin()));
				
				// Fill First String
				fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), quantity, Request.get_Price());
				std::string Result = std::string(szType);
				retList.push_back(Result);
				// Fill Second String
				fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), quantity, Request.get_Price());
				Result = std::string(szType);
				retList.push_back(Result);

				// Set the Request quantity to 0 and eliminate this book entry
				Request.set_Quantity(0);
				auto iter2 = InnerSet.rbegin();
				std::map<double, std::set<uint16_t>>::reverse_iterator iterator = firstMap.rbegin();
				eraseEntry(*iter2, firstMap, iterator);

				return retList;
				
			}
			else if (quantity > Request.get_Quantity()) // Case where book quantity is less than entry
			{
				// Set the result strings
				fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
				std::string Result = std::string(szType);
				retList.push_back(Result);
				fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
				Result = std::string(szType);
				retList.push_back(Result);

				// Set the order to 0, reduce the quantity of the bookeditems
				Request.set_Quantity(0);
				m_BookedItems[*(InnerSet.rbegin())].set_Quantity(quantity - Request.get_Quantity());

				return retList;

			}
			else // The quantity is lower
			{
				// try to find another entry in the book to fullfill the remaining items
				// Handle this logic upstairs, for now just reduce the request quanity and eliminate the entry
				fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
				std::string Result = std::string(szType);
				retList.push_back(Result);
				fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
				Result = std::string(szType);
				retList.push_back(Result);

				Request.set_Quantity(Request.get_Quantity() - quantity);

				auto iter = m_BookedItems.find(*(InnerSet.rbegin()));
				auto iter2 = InnerSet.rbegin();
				std::map<double, std::set<uint16_t>>::reverse_iterator iterator = firstMap.rbegin();
				eraseEntry(*iter2, firstMap, iterator);

				return retList;
			}
		}
		else // Case where there isn't an exact match with price
		{
			// Iterate through the rest of the map until we find a condition t hat satisfies the transaction
			for (std::map<double, std::set<uint16_t>>::reverse_iterator iterator = firstMap.rbegin(); iterator != firstMap.rend(); ++iterator)
			{
				if (m_BookType == TradeAction::Buy)
				{
					// if its a buy book and the request is lower priced than the iterator, go ahead and book the transaction
					if (iterator->first > Request.get_Price())
					{
						auto InnerSet = firstMap[iterator->first];
						auto Item = m_BookedItems[*(InnerSet.rbegin())];
						uint64_t quantity = Item.get_Quantity();
						if (quantity > Request.get_Quantity())
						{
							m_BookedItems[*(InnerSet.rbegin())].set_Quantity(quantity - Request.get_Quantity());
							Request.set_Quantity(0);
							// Create the fill order string
							fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							std::string Result = std::string(szType);
							retList.push_back(Result);
							fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							Result = std::string(szType);
							retList.push_back(Result);
							return retList;
						}
						else if (quantity < Request.get_Quantity()) // Case where book quantity is less than entry
						{
							// fillout results strings
							fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							std::string Result = std::string(szType);
							retList.push_back(Result);
							fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							Result = std::string(szType);
							retList.push_back(Result);

							// reduce quantities to 0
							auto iter = m_BookedItems.find(*(InnerSet.rbegin()));
							Request.set_Quantity(Request.get_Quantity() - iter->second.get_Quantity());
							auto iter2 = InnerSet.rbegin();


							eraseEntry(*iter2, firstMap, iterator);

							return retList;

						}
						else // Case wheret they're equal (we should probably not ever need this
						{
							// fillout results strings
							fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							std::string Result = std::string(szType);
							retList.push_back(Result);
							fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							Result = std::string(szType);
							retList.push_back(Result);

							auto iter = m_BookedItems.find(*(InnerSet.rbegin()));
							Request.set_Quantity(Request.get_Quantity() - iter->second.get_Quantity());
							auto iter2 = InnerSet.rbegin();
							
							eraseEntry(*iter2, firstMap, iterator);


							return retList;

						}
					}
					else
					{
						// If we can't find a price to match the action, then return empty list
						return retList;
					}
				}
				else if (m_BookType == TradeAction::Sell)
				{
					// if our iterator's price is lower than the requested price, then do the sale
					if (iterator->first < Request.get_Price())
					{
						auto InnerSet = firstMap[iterator->first];
						auto Item = m_BookedItems[*(InnerSet.rbegin())];
						uint64_t quantity = Item.get_Quantity();
						if (quantity > Request.get_Quantity())
						{
							m_BookedItems[*(InnerSet.rbegin())].set_Quantity(quantity - Request.get_Quantity());
							Request.set_Quantity(0);
							// Create the fill order string
							fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							std::string Result = std::string(szType);
							retList.push_back(Result);
							fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							Result = std::string(szType);
							retList.push_back(Result);
							return retList;
						}
						else if (quantity < Request.get_Quantity()) // Case where book quantity is less than entry
						{
							// fillout results strings
							fillOrder(&szType[0], *(InnerSet.rbegin()), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							std::string Result = std::string(szType);
							retList.push_back(Result);
							fillOrder(&szType[0], Request.get_OrderId(), Request.get_Id(), Request.get_Quantity(), Request.get_Price());
							Result = std::string(szType);
							retList.push_back(Result);

							// reduce quantities to 0
							auto iter = m_BookedItems.find(*(InnerSet.rbegin()));
							Request.set_Quantity(Request.get_Quantity() - iter->second.get_Quantity());
							auto iter2 = InnerSet.rbegin();

							eraseEntry(*iter2, firstMap, iterator);

							return retList;
						}
					}
					else
					{
						// If we can't find a price to match the action, then return empty list
						return retList;
					}
				}
			}
		}
	}
	
	return retList;
}

void SortedBookKeeper::PrintTree(std::list<std::string>& RetList)
{
	BookedItem Item;

	for (std::unordered_map<std::string, std::map<double, std::set<uint16_t>>>::iterator StockIDIterator = m_Book.begin(); StockIDIterator != m_Book.end(); StockIDIterator++)
	{
		for (auto PriceIterator = StockIDIterator->second.begin(); PriceIterator != StockIDIterator->second.end(); PriceIterator++)
		{
			for (auto OIter = PriceIterator->second.begin(); OIter != PriceIterator->second.begin(); OIter++)
			{
				Item = m_BookedItems[*OIter];
				std::string retstring = Item.printItem();
				RetList.push_back(retstring);
			}
		}
	}
}

void SortedBookKeeper::eraseEntry(uint16_t orderId, std::map<double, std::set<uint16_t>>& firstMap, std::map<double, std::set<uint16_t>>::reverse_iterator& iterator)
{
	// The first level, i.e. the stock ID, will not be erased
	auto InnerSet = firstMap[iterator->first];
	// if there was nothing to begin with just return
	if (InnerSet.empty())
		return;

	auto iter2 = InnerSet.rbegin();

	// erase the entry of this order ID
	m_AllOrders.erase(orderId);
	m_priceOrders.erase(orderId);
	m_symbolOrders.erase(orderId);
	m_BookedItems.erase(m_BookedItems.find(*(InnerSet.rbegin())));
	InnerSet.erase(*iter2);

	// Finally, if there are no more order IDs in this price point, eliminate the price point shuffle the iterator
	if (InnerSet.empty())
	{
		firstMap.erase(--(iterator.base()));
		iterator = firstMap.rbegin();
	}
}

void SortedBookKeeper::eraseEntry(uint16_t orderId, std::map<double, std::set<uint16_t>>& firstMap, std::map<double, std::set<uint16_t>>::iterator& iterator)
{
	// The first level, i.e. the stock ID, will not be erased
	auto InnerSet = firstMap[iterator->first];
	auto iter2 = InnerSet.rbegin();

	// erase the entry of this order ID
	InnerSet.erase(*iter2);
	m_AllOrders.erase(orderId);
	m_priceOrders.erase(orderId);
	m_symbolOrders.erase(orderId);
	m_BookedItems.erase(m_BookedItems.find(*(InnerSet.rbegin())));

	// Finally, if there are no more order IDs in this price point, eliminate the price point shuffle the iterator
	if (InnerSet.empty())
	{
		firstMap.erase(iterator);
	}
}



void SortedBookKeeper::addEntry(BookedItem& NewEntry)
{
	auto iterStockId = m_Book.find(NewEntry.get_Id());
	
	// Check if this stock ID has been entered before, if not, add it here
	if (iterStockId == m_Book.end())
	{
		std::map<double, std::set<uint16_t>> PairPriceQuantity;
		std::set<uint16_t> innerSet = { NewEntry.get_OrderId() };
		PairPriceQuantity.insert(std::pair<double, std::set<uint16_t>>(NewEntry.get_Price(), innerSet));
		std::map<std::string, std::map<double, std::set<uint16_t>>> PairSymbolWithMap;
		// Inset the new entry into the book
		m_Book.insert(std::pair<std::string, std::map<double, std::set<uint16_t>>>(NewEntry.get_Id(), PairPriceQuantity));
	}
	else
	{
		// If the ID already exists, then go down one level
		// If the price point doesn't exist add it
		std::map<double, std::set<uint16_t>> StockIditer = m_Book[NewEntry.get_Id()];

		if (StockIditer.find(NewEntry.get_Price()) == StockIditer.end())
		{
			std::map<double, std::set<uint16_t>> PairPriceQuantity;
			std::set<uint16_t> innerSet = { NewEntry.get_OrderId() };
			m_Book[NewEntry.get_Id()].insert(std::pair<double, std::set<uint16_t>>(NewEntry.get_Price(), innerSet));
		}
		else
		{
			// finally if the price point is already there, just add to the existing set
			m_Book[NewEntry.get_Id()][NewEntry.get_Price()].insert(NewEntry.get_OrderId());
		}
	}

	// These always get done as each order is unique
	m_AllOrders.insert(NewEntry.get_OrderId());
	m_priceOrders.insert(std::pair<uint16_t, double>(NewEntry.get_OrderId(), NewEntry.get_Price()));
	m_symbolOrders.insert(std::pair<uint16_t, std::string>(NewEntry.get_OrderId(), NewEntry.get_Id()));
	m_BookedItems.insert(std::pair<uint16_t, BookedItem>(NewEntry.get_OrderId(), NewEntry));
}

void SortedBookKeeper::CancelOrder(uint16_t OrderId)
{
	auto itr = m_BookedItems.find(OrderId);

	// if the book does not have this Id, then just return immediately
	if (itr == m_BookedItems.end())
		return;

	BookedItem Item = m_BookedItems[OrderId];

	std::map<double, std::set<uint16_t>>& firstMap = m_Book[Item.get_Id()];
	std::map<double, std::set<uint16_t>>::iterator iterator;

	eraseEntry(OrderId, firstMap, iterator);
}