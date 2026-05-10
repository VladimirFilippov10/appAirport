#pragma once
#include <string>
#include <ctime>
#include "Aircraft.h"

class Flight {
private:
    std::string number;
    Aircraft* aircraft;
    std::string destination;
    std::string departurePoint;
    std::time_t departureTime;
    int durationMinutes;
    int freeSeats;
public:
    Flight();
    Flight(const std::string& num, Aircraft* ac, const std::string& dest,
        const std::string& depPoint, std::time_t depTime, int duration, int seats);

    std::string getNumber() const;
    Aircraft* getAircraft() const;
    std::string getDestination() const;
    std::string getDeparturePoint() const;
    std::time_t getDepartureTime() const;
    int getDurationMinutes() const;
    int getFreeSeats() const;

    void setFreeSeats(int seats);
    void setNumber(const std::string& num);
    void setDestination(const std::string& dest);
    void setDeparturePoint(const std::string& dep);
    void setDepartureTime(std::time_t time);
    void setDurationMinutes(int dur);
    void setAircraft(Aircraft* ac);  // ÒÎËÜÊÎ ÎÁÚßÂËÅÍÈÅ!

    std::string getDepartureTimeStr() const;

    std::string toFileString() const;
    static Flight fromFileString(const std::string& line, Aircraft* ac);
};