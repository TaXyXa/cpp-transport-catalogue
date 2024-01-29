#pragma once

#include <cstdint>
#include <functional>
#include <tuple>
#include <list>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>

#include "geo.h"

enum class Requvest_Status {
    good, bad
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
    Requvest_Status requvest_status;
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
    std::unordered_map<std::string, uint32_t> distances; 
};

class TransportCatalogue {
public:
    void AddStop(const std::string& name, const StopDataParce& stop_data);

    void AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector);

    RouteData GetRouteData(const std::string_view& bus_name) const;
    
    StopInfo GetStopInfo(const std::string_view& stop_name) const;

private:
    //Возможно стоит заменить Stop* на умный указатель, но так как я не понимаю зачем 
    //то пока отправлю просто рабочу версию кода чтобы двигаться дальше
    //Будет интересен ваш комментарий на этот счет
    std::list<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_reference_;
    std::unordered_map<std::string, Route> routes_;
    std::unordered_map<std::string_view, std::set<Route*, CompareRoutes>> stop_and_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, DistanceHasher> distances_;

    Stop* AddEmptyStop (const std::string& name, const Coordinates& coordinate);
};
