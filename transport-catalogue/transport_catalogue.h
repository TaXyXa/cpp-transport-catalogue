#pragma once

#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

#include "geo.h"


struct StopAndCoordinates {
    std::string name;
    Coordinates coordinates;
};

struct Route {
    std::string name_;
    std::vector<StopAndCoordinates*> stops_list;
};

struct RouteData {
    int stops_number; 
    int uniq_stops_number;
    double route_distance;
};

struct StopData {
    int requvest_status; 
    std::set<std::string_view> buses;
};

class StopHasher {
public:
    size_t operator()(const StopAndCoordinates* stop_name) const;

};

class TransportCatalogue {
public:
    void AddStop(const std::string& name, const Coordinates& coordinates);

    void AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector);

    RouteData GetRouteData(const std::string_view& bus_name) const;
    
    StopData GetStopData(const std::string_view& stop_name) const;

private:
    std::list<StopAndCoordinates> stops_;
    std::unordered_map<std::string_view, StopAndCoordinates*> stops_reference_;
    std::unordered_map<std::string, Route> routes_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_and_buses_;
};
