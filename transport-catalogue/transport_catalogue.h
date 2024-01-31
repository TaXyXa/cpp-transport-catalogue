#pragma once

#include <cstdint>
#include <functional>
#include <tuple>
#include <forward_list>
#include <string>
#include <string_view>
#include <set>
#include <vector>
#include <unordered_map>

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
};

struct RouteData {
    size_t stops_number;
    size_t uniq_stops_number;
    double route_distance;
    double curvature;
};

struct CompareRoutes {
    bool operator()(const Route* route1, const Route* route2) const {
        return route1->name_ < route2->name_;
    }
};

struct StopInfo {
    RequestStatus request_status;
    std::set<Route*, CompareRoutes> buses;
};

class StopHasher {
public:
    size_t operator()(const Stop* stop_name) const;

};

class DistanceHasher {
public:
    size_t operator()(const std::pair<Stop*, Stop*> stop_name) const;

};

//структура для передачи распарсеной строки 
struct StopDataParce {
    Coordinates coordinates;
    std::vector<std::pair<std::string, uint32_t>> distances;
};

class TransportCatalogue {
public:
    Stop* AddStop(const std::string& name, const Coordinates& coordinates);
    
    void AddDistance(std::string& curent_stop, std::string& second_stop, uint32_t distance);

    uint32_t GetDistance(Stop* curent_stop, Stop* second_stop) const;

    void AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector);

    RouteData GetRouteData(const std::string_view& bus_name) const;
    
    StopInfo GetStopInfo(const std::string_view& stop_name) const;

private:
    //мне нужен контейнер с быстрым добавлением и не инвалидирующимся при добавлении 
    //итератором, так как для быстрого доступа к остановкам я использую бинарное дерево
    //указателей Stop*. Но пока достаточно и однонаправленного списка, наверное вы на него
    //и намекали
    std::forward_list<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_reference_;
    std::unordered_map<std::string, Route> routes_;
    std::unordered_map<std::string_view, std::set<Route*, CompareRoutes>> stop_and_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, DistanceHasher> distances_;

    Stop* AddEmptyStop (const std::string& name, const Coordinates& coordinate);
    double GetGeoDistance (const std::vector<Stop*>* route) const;
    double GetRouteDistance (const std::vector<Stop*>* route) const;
    size_t GetUniqStops (const std::vector<Stop*>* route) const;
};
