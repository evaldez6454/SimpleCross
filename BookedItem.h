#pragma once
#include <string>
#include <stdint.h>
//F 10003 IBM 5 100.00000
enum TradeAction
{
	Buy = 0,
	Sell
};

class BookedItem
{
public:

	BookedItem(std::string StockID, uint64_t quantity, TradeAction Action, double Price, uint16_t OrderId);
	~BookedItem();
	BookedItem();
	// Getters
	std::string get_Id() { return m_Id; };
	uint64_t get_Quantity() { return m_quantity; };
	TradeAction get_Action() { return m_Action; };
	uint16_t get_OrderId() { return m_OrderId; };
	double get_Price() { return m_Price; };
	// Setters
	// Only one setter since the other should be consts
	void set_Quantity(uint64_t quantity) { m_quantity = quantity; };
	void set_OrderId(uint16_t OrderId) { m_OrderId = OrderId; };
	void set_Action(TradeAction Action) { m_Action = Action; };
	void set_Price(double Price) { m_Price = Price; };
	void set_Id(std::string Id) { m_Id = Id; };
	std::string printItem();

private:
	uint64_t m_quantity;
	uint16_t m_OrderId;
	std::string m_Id;
	TradeAction m_Action; 
	double m_Price;
};