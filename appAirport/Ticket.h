#pragma once
#include <string>
#include <ctime>

class Ticket {
private:
    std::string cashierNumber;
    std::string flightNumber;
    std::time_t saleDate;
    std::string passengerName;
    std::string passportSeries;
    std::string passportNumber;
    double price;
public:
    Ticket();
    Ticket(const std::string& cashier, const std::string& flightNum, std::time_t dt,
        const std::string& passenger, const std::string& psSeries,
        const std::string& psNum, double pr);

    std::string getCashierNumber() const;
    std::string getFlightNumber() const;
    std::time_t getSaleDate() const;
    std::string getPassengerName() const;
    std::string getPassportSeries() const;
    std::string getPassportNumber() const;
    double getPrice() const;

    void setCashierNumber(const std::string& num);
    void setFlightNumber(const std::string& num);
    void setSaleDate(std::time_t dt);
    void setPassengerName(const std::string& name);
    void setPrice(double p);

    std::string getSaleDateStr() const;

    std::string toFileString() const;
    static Ticket fromFileString(const std::string& line);
};