#pragma once

#include <functional>
#include <tuple>
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

struct CompareRoutes {
    bool operator()(const Route* route1, const Route* route2) const {
        return route1->name_ < route2->name_;
    }
};

struct StopData {
    int requvest_status; 
    std::set<Route*, CompareRoutes> buses;
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
    //у меня же есть отдельный unordered_map для быстрого доступа к остановкам. 
    //Если я буду использовать vector то при добавлении новых остановок указатели 
    //на старые в stops_reference_ инвалидируются. Наверное у меня плохой нейминг.
    std::list<StopAndCoordinates> stops_;
    std::unordered_map<std::string_view, StopAndCoordinates*> stops_reference_;
    //список автобусов тут по сехеме: автобус - маршрут
    std::unordered_map<std::string, Route> routes_;
    //а зачем, если я создаю вьюшки на них? вьюшки же как указатели работают, 
    //или я не правильно их понимаю? но если в дальнейшем понадобится доступ 
    //к инфрмации каждого автобуса - поменял
    std::unordered_map<std::string_view, std::set<Route*, CompareRoutes>> stop_and_buses_;
};
