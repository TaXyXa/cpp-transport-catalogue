#pragma once

#include <deque>
#include <functional>
#include <tuple>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

#include "geo.h"

struct StopAndCoordinates {
    std::string name;
    Coordinates coordinates;
};

class StopHasher {
public:
    size_t operator()(const StopAndCoordinates* stop_name) const;

};

class TransportCatalogue {
public:
    void AddStop(std::string& name, Coordinates coordinates);

    void AddRoute(std::string& bus_name, const std::vector<std::string_view>& stops_vector);

    std::tuple<int, int, double> GetRouteData(const std::string& bus_name) const;
    
    std::set<std::string_view> GetStopData(const std::string& stop_name) const;

private:
    std::deque<StopAndCoordinates> stops_;
    std::unordered_map<std::string_view, StopAndCoordinates*> stops_reference_;
    std::unordered_map<std::string, std::vector<StopAndCoordinates*>> routes_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_and_buses_;
};
