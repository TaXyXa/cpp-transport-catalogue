#pragma once

#include <cstdint>
#include <functional>
#include <forward_list>
#include <optional>
#include <string>
#include <string_view>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

enum class RequestStatus {
    good,
    bad
};

struct Stop {
    std::string name;
    Coordinates coordinates;
};

struct Route {
    std::string name_;
    std::vector<Stop*> stops_list;
    std::vector<Stop*> end_stops_list;
};

struct RouteData {
    RequestStatus request_status;
    size_t stops_number;
    size_t uniq_stops_number;
    double route_distance;
    double curvature;
};

struct CompareRoutes {
    bool operator()(const Route* route1, const Route* route2) const;
};

class RouteHasher {
public:
    size_t operator()(const Route* route_name) const;
};

struct StopInfo {
    RequestStatus request_status;
    const std::unordered_set<Route*, RouteHasher>* buses;
};

class StopHasher {
public:
    size_t operator()(const Stop* stop_name) const;
};

class DistanceHasher {
public:
    size_t operator()(const std::pair<Stop*, Stop*> stop_name) const;
};

class CoordinateHasher {
public:
    size_t operator()(const Coordinates coordinate) const;
};
//структура для передачи распарсеной строки 
struct StopDataParse {
    Coordinates coordinates;
    std::vector<std::pair<std::string, uint32_t>> distances;
};

