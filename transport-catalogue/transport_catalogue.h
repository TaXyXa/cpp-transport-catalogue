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
#include "domain.h"

class TransportCatalogue {
public:
    Stop* AddStop(const std::string& name, const Coordinates& coordinates);

    void AddDistance(Stop* curent_stop, Stop* second_stop, uint32_t distance);

    Stop* GetStop(const std::string& name);

    uint32_t GetDistance(Stop* curent_stop, Stop* second_stop) const;

    void AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector, 
        const std::vector<std::string_view>& end_stops_vector);

    const Route* GetRoute(const std::string& name) const ;

    std::vector<const Route*> GetAllRouts() const;

    RouteData GetRouteData(const std::string_view& bus_name) const;

    StopInfo GetStopInfo(const std::string_view& stop_name) const;

private:
    std::forward_list<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_reference_;
    std::unordered_map<std::string, Route> routes_;
    std::unordered_map<std::string_view, std::unordered_set<Route*, RouteHasher>> stop_and_buses_;
    std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, DistanceHasher> distances_;

    double GetGeoDistance(const std::vector<Stop*>* route) const;
    double GetRouteDistance(const std::vector<Stop*>* route) const;
    size_t GetUniqStops(const std::vector<Stop*>* route) const;
};