#include "transport_catalogue.h"

#include <cstdint>
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

Stop* TransportCatalogue::AddEmptyStop(const std::string& name, const Coordinates& coordinate) {
    Stop* list_pointer = nullptr;
    auto it = stops_reference_.find(name);
    if (it == stops_reference_.end()) {
        stops_.push_back({ name, coordinate });
        list_pointer = &stops_.back();
    } else {
        list_pointer = (*it).second;
        (*list_pointer).coordinates = coordinate;
    }
    stops_reference_.insert({ static_cast<std::string_view>((*list_pointer).name), list_pointer });
    return list_pointer;
}

Stop* TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates) {
    Stop* curent_stop = AddEmptyStop(name, coordinates);
    stop_and_buses_.insert({ static_cast<std::string_view>((*curent_stop).name), {} });
    return curent_stop;
}

void TransportCatalogue::AddDistance(Stop* curent_stop, const std::vector<std::pair<std::string, uint32_t>>& distances) {
    for (const auto& dist : distances) {
        Stop* second_stop = nullptr;
        auto it2 = stops_reference_.find(dist.first);
        if (it2 == stops_reference_.end()) {
            second_stop = AddEmptyStop(dist.first, {});
        } else {
            second_stop = (*it2).second;
        }
        distances_.insert({ { curent_stop, second_stop }, dist.second });
    }
}

void TransportCatalogue::AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector) {
    Route route = { bus_name, {} };
    for (const auto stop_name : stops_vector) {
        Stop* it = stops_reference_.at(stop_name);
        route.stops_list.push_back(it);
    }
    routes_.insert({ bus_name, std::move(route) });

    std::unordered_map<std::string, Route>::iterator iter;
    iter = routes_.find(bus_name);
    if (iter == routes_.end()) {
        return;
    }

    for (const auto& bus : (iter->second).stops_list) {
        std::set<Route*, CompareRoutes>* my_set = nullptr;
        auto iter2 = stop_and_buses_.find((*bus).name);
        if (iter2 != stop_and_buses_.end()) {
            (*iter2).second.insert(&(iter->second));
        }
    }
}

double TransportCatalogue::GetGeoDistance(const std::vector<Stop*>* route) const {
    double geo_distance = 0;
    Coordinates coordinate = (*(*route).begin())->coordinates;
    for (const auto& stop : *route) {
        geo_distance += ComputeDistance(coordinate, stop->coordinates);
        coordinate = stop->coordinates;
    }
    return geo_distance;
}

double TransportCatalogue::GetRouteDistance(const std::vector<Stop*>* route) const {
    double route_distance = 0;
    Stop* previous_stop = *(*route).begin();
    bool first = true;
    for (const auto& stop : *route) {
        if (first) {
            first = false;
        }
        else {
            auto it_dist = distances_.find({ previous_stop, stop });
            if (it_dist != distances_.end()) {
                route_distance += (*it_dist).second;
            }
            else {
                it_dist = distances_.find({ stop, previous_stop });
                if (it_dist != distances_.end()) {
                    route_distance += (*it_dist).second;
                }
            }
        }
        previous_stop = stop;
    }
    return route_distance;
}

size_t TransportCatalogue::GetUniqStops(const std::vector<Stop*>* route) const {
    std::unordered_set <Stop*, StopHasher> iters;
    for (const auto& stop : *route) {
        iters.insert(stop);
    }
    return iters.size();
}
RouteData TransportCatalogue::GetRouteData(const std::string_view& bus_name) const {
    const std::vector<Stop*>* route = nullptr;
    auto it_route = routes_.find(static_cast<std::string>(bus_name));
    if (it_route != routes_.end()) {
        route = &(*it_route).second.stops_list;
    } else {
        return { 0, 0, 0.0, 0.0 };
    }
    const size_t stops_number = (*route).size();
    double geo_distance = GetGeoDistance (route);
    double route_distance = GetRouteDistance (route);
    const double curvature = route_distance / geo_distance;
    const size_t uniq_stops_number = GetUniqStops (route);
    return { stops_number , uniq_stops_number , route_distance , curvature };
}

StopInfo TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
    auto iter = stop_and_buses_.find(stop_name);
    if (iter != stop_and_buses_.end()) {
        return { RequestStatus::good, (*iter).second };
    }
    else {
        return { RequestStatus::bad, {} };
    }
}

size_t StopHasher::operator()(const Stop* stop_name) const {

    return std::hash<std::string>{} ((*stop_name).name);

}

size_t DistanceHasher::operator()(const std::pair<Stop*, Stop*> stop_name) const {
    size_t hash = std::hash<std::string>{} ((*stop_name.first).name)
        * std::hash<std::string>{} ((*stop_name.first).name)
        + std::hash<std::string>{} ((*stop_name.second).name);
    return hash;

}