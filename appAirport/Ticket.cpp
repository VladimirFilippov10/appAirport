#include "Ticket.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

Ticket::Ticket() : saleDate(0), price(0) {}

Ticket::Ticket(const std::string& cashier, const std::string& flightNum, std::time_t dt,
    const std::string& passenger, const std::string& psSeries,
    const std::string& psNum, double pr)
    : cashierNumber(cashier), flightNumber(flightNum), saleDate(dt),
    passengerName(passenger), passportSeries(psSeries), passportNumber(psNum), price(pr) {}

std::string Ticket::getCashierNumber() const { return cashierNumber; }
std::string Ticket::getFlightNumber() const { return flightNumber; }
std::time_t Ticket::getSaleDate() const { return saleDate; }
std::string Ticket::getPassengerName() const { return passengerName; }
std::string Ticket::getPassportSeries() const { return passportSeries; }
std::string Ticket::getPassportNumber() const { return passportNumber; }
double Ticket::getPrice() const { return price; }

void Ticket::setCashierNumber(const std::string& num) { cashierNumber = num; }
void Ticket::setFlightNumber(const std::string& num) { flightNumber = num; }
void Ticket::setSaleDate(std::time_t dt) { saleDate = dt; }
void Ticket::setPassengerName(const std::string& name) { passengerName = name; }
void Ticket::setPrice(double p) { price = p; }

std::string Ticket::getSaleDateStr() const {
    std::tm tmBuf = {};
    localtime_s(&tmBuf, &saleDate);
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Ticket::toFileString() const {
    return cashierNumber + ";" + flightNumber + ";" + getSaleDateStr() + ";" +
        passengerName + ";" + passportSeries + ";" + passportNumber + ";" +
        std::to_string(price);
}

Ticket Ticket::fromFileString(const std::string& line) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string part;
    while (std::getline(ss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.size() >= 7) {
        std::tm tm = {};
        std::istringstream timeStream(parts[2]);
        timeStream >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::time_t saleDt = std::mktime(&tm);

        Ticket result;
        result.setCashierNumber(parts[0]);
        result.setFlightNumber(parts[1]);
        result.setSaleDate(saleDt);
        result.setPassengerName(parts[3]);
        result.setPrice(std::stod(parts[6]));
        return result;
    }
    return Ticket();
}