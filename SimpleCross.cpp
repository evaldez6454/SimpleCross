#include "SimpleCross.h"
#include <exception>

static std::string AcceptableSymbols = "OXP";


results_t SimpleCross::action(const std::string& line)
{ 
	results_t listResults;
	BookedItem Entry;
	std::string ErrorString;
	CommandType Command = CommandType::E; // Default it to error, but will be overwritten
	bool result = parseLine(Entry, line, Command, ErrorString);
	if (!result)
	{
		ErrorString = "E " + ErrorString;
		listResults.push_back(ErrorString);
		return listResults;
	}

	switch (Command) 
	{
	case CommandType::O:
	{
		HandleTransaction(Entry, listResults);
		break;
	}
	// TODO - Finish the cases where I want to print out entries and cancel orders
	case CommandType::P:
	{
		HandlePrint(listResults);
		break;
	}
	case CommandType::X:
	{
		HandleDelete(Entry.get_OrderId());
		break;
	}
	}

	return listResults; 
} 

bool SimpleCross::parseLine(BookedItem& Item, const std::string& line, CommandType& Command, std::string& ErrReason)
{
	bool success = true;
	std::string forward;
	std::string backward;
	int count = 0;
	if (line.find(' ') != std::string::npos)
	{
		// In this first iteration we expect an Acceptable symbol
		size_t n = line.find(' ');
		forward = line.substr(n+1);
		backward = line.substr(0, n);


		if (AcceptableSymbols.find(backward[0]) == std::string::npos)
		{
			ErrReason = "An Order line was received that did not conform to the normal order types O/P/X";
			return false;
		}

		if (backward[0] == AcceptableSymbols[1])
			Command = CommandType::X;
		else
			Command = CommandType::O;

		while ((forward.find(' ') != std::string::npos) || (forward.size() > 0))
		{
			n = forward.find(' ');
			backward = forward.substr(0, n);
			switch (count)
			{
			case 0: // Transform the second entry into a uint16
			{
				try 
				{
					uint32_t orderId = std::stoul(backward);
					Item.set_OrderId(static_cast<uint16_t>(orderId));
				}
				catch(std::exception& e)
				{
					ErrReason = "Was not able to convert the OrderId String" + std::string(e.what());
				}

				// If this is an order cancelation, return immediately
				if (Command == CommandType::X)
					return true;
				break;
			}
			case 1: // get Stock ID
			{
				if (backward.size() > 8)
				{
					ErrReason = "Stock ID did not conform to the maximum length of 8";
					return false;
				}
				Item.set_Id(backward);
				break;
			}
			case 2: // Action Type, either buy or sell
			{
				if (backward == "S")
					Item.set_Action(TradeAction::Sell);
				else if (backward == "B")
					Item.set_Action(TradeAction::Buy);
				else
				{
					ErrReason = "The transaction Type buy/sell was not parsable";
					return false;
				}

				break;
			}
			case 3: // Trade Quantity
			{
				try
				{
					uint64_t Quantity = std::stoull(backward);
					Item.set_Quantity(Quantity);
				}
				catch(std::exception& e)
				{
					ErrReason = "Could not properly convert the trade quantity" + std::string(e.what());
				}
				break;
			}
			case 4: // Finally the price
			{
				try
				{
					double Quantity = std::stold(backward);
					Item.set_Price(Quantity);
				}
				catch (std::exception& e)
				{
					ErrReason = "Could not properly convert the trade price" + std::string(e.what());
				}
				// at this point we're done, we might as well return true
				return true;
				break;
			}
			}

			forward = forward.substr(n + 1);
			count++;
		}
	}
	else if((line.find(' ') == std::string::npos) && (line.size()==0))
	{
	ErrReason = "Read in a line with no characters";
		return false;
	}
	else // in the event that the order is 'P'
	{
		if (line[0] == AcceptableSymbols[2])
			Command = CommandType::P;
	}

	return success;
}

void SimpleCross::HandlePrint(results_t& resList)
{
	m_BuyingBook->PrintTree(resList);
	m_SalesBook->PrintTree(resList);
}

void SimpleCross::HandleDelete(uint16_t TargetOrder)
{
	m_BuyingBook->CancelOrder(TargetOrder);
	m_SalesBook->CancelOrder(TargetOrder);
}

void SimpleCross::HandleTransaction(BookedItem& Item, results_t& resList)
{
	bool result = true;
	std::string Result;
	results_t local;
	// First Determine if this is a duplicate order
	result = (m_AllOrders.find(Item.get_OrderId()) == m_AllOrders.end());


	if (!result)
	{
		Result = "E " + std::to_string(Item.get_OrderId());
		resList.push_back(Result);
		return;
	}
	else
	{
		// if not duplicate add it to the set
		m_AllOrders.insert(Item.get_OrderId());

	}

	if (Item.get_Action() == TradeAction::Buy)
	{
		if (!(Item.get_Quantity() > 0))
			return;

		local = m_SalesBook->modifyEntry(Item);

		// if the result list is empty, it means that no entries were found
		if (local.empty())
		{
			m_SalesBook->addEntry(Item);
			return;
		}
		else
		{
			while (Item.get_Quantity() > 0)
			{
				local = m_SalesBook->modifyEntry(Item);

				for (results_t::iterator iter = local.begin(); iter != local.end(); iter++)
					resList.push_back(*iter);
			}
			resList = local;

			return;
		}
	}
	else
	{
		if (!(Item.get_Quantity() > 0))
			return;

		local = m_BuyingBook->modifyEntry(Item);

		// if the result list is empty, it means that no entries were found
		if (local.empty())
		{
			m_BuyingBook->addEntry(Item);
			return;
		}
		else
		{
			while (Item.get_Quantity() > 0)
			{
				local = m_BuyingBook->modifyEntry(Item);

				for (results_t::iterator iter = local.begin(); iter != local.end(); iter++)
					resList.push_back(*iter);
			}
			resList = local;
			return;
		}
	}
}
