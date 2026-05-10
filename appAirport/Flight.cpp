#include "Flight.h"
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

Flight::Flight() : aircraft(nullptr), durationMinutes(0), freeSeats(0), departureTime(0) {}

Flight::Flight(const std::string& num, Aircraft* ac, const std::string& dest,
    const std::string& depPoint, std::time_t depTime, int duration, int seats)
    : number(num), aircraft(ac), destination(dest), departurePoint(depPoint),
    departureTime(depTime), durationMinutes(duration), freeSeats(seats) {}

std::string Flight::getNumber() const { return number; }
Aircraft* Flight::getAircraft() const { return aircraft; }
std::string Flight::getDestination() const { return destination; }
std::string Flight::getDeparturePoint() const { return departurePoint; }
std::time_t Flight::getDepartureTime() const { return departureTime; }
int Flight::getDurationMinutes() const { return durationMinutes; }
int Flight::getFreeSeats() const { return freeSeats; }

void Flight::setFreeSeats(int seats) { freeSeats = seats; }
void Flight::setNumber(const std::string& num) { number = num; }
void Flight::setDestination(const std::string& dest) { destination = dest; }
void Flight::setDeparturePoint(const std::string& dep) { departurePoint = dep; }
void Flight::setDepartureTime(std::time_t time) { departureTime = time; }
void Flight::setDurationMinutes(int dur) { durationMinutes = dur; }

// ========== ĞÅÀËÈÇÀÖÈß setAircraft (òîëüêî çäåñü) ==========
void Flight::setAircraft(Aircraft* ac) { aircraft = ac; }

std::string Flight::getDepartureTimeStr() const {
    std::tm tmBuf = {};
    localtime_s(&tmBuf, &departureTime);
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M");
    return oss.str();
}

std::string Flight::toFileString() const {
    return number + ";" + destination + ";" + departurePoint + ";" +
        getDepartureTimeStr() + ";" + std::to_string(durationMinutes) + ";" +
        std::to_string(freeSeats);
}

Flight Flight::fromFileString(const std::string& line, Aircraft* ac) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string part;
    while (std::getline(ss, part, ';')) {
        parts.push_back(part);
    }

    if (parts.size() >= 6) {
        std::tm tm = {};
        std::istringstream timeStream(parts[3]);
        timeStream >> std::get_time(&tm, "%Y-%m-%d %H:%M");
        std::time_t depTime = std::mktime(&tm);

        Flight result;
        result.setNumber(parts[0]);
        result.setAircraft(ac);
        result.setDestination(parts[1]);
        result.setDeparturePoint(parts[2]);
        result.setDepartureTime(depTime);
        result.setDurationMinutes(std::stoi(parts[4]));
        result.setFreeSeats(std::stoi(parts[5]));
        return result;
    }
    return Flight();
}