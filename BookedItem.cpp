#include "BookedItem.h"

BookedItem::BookedItem(std::string StockID, 
	uint64_t quantity, 
	TradeAction Action, 
	double price,
	uint16_t OrderId) 
	: m_Id(StockID),
	m_quantity(quantity),
	m_Action(Action), 
	m_Price(price),
	m_OrderId(OrderId)
{}

BookedItem::BookedItem() : m_Id("None"),
m_quantity(0),
m_Action(TradeAction::Buy),
m_Price(-1.0),
m_OrderId(-1) // force it to be max ID
{}

BookedItem::~BookedItem()
{

}

std::string BookedItem::printItem()
{
	char szType[128];
	sprintf(szType, "P %d %s %llu %7.5f", m_OrderId, m_Id.c_str(), m_quantity, m_Price);
	std::string Result = std::string(szType);
	return Result;
}