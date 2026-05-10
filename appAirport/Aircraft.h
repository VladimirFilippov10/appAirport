#pragma once
#include <string>

class Aircraft {
private:
    std::string name;
    std::string category;
    int seats;
public:
    Aircraft();
    Aircraft(const std::string& n, const std::string& c, int s);
    std::string getName() const;
    std::string getCategory() const;
    int getSeats() const;
    void setName(const std::string& n);
    void setCategory(const std::string& c);
    void setSeats(int s);
};