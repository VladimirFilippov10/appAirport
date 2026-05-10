#include "Aircraft.h"

Aircraft::Aircraft() : name(""), category(""), seats(0) {}

Aircraft::Aircraft(const std::string& n, const std::string& c, int s)
    : name(n), category(c), seats(s) {}

std::string Aircraft::getName() const { return name; }
std::string Aircraft::getCategory() const { return category; }
int Aircraft::getSeats() const { return seats; }
void Aircraft::setName(const std::string& n) { name = n; }
void Aircraft::setCategory(const std::string& c) { category = c; }
void Aircraft::setSeats(int s) { seats = s; }