#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <tuple>
#include <list>
#include <string>
#include <string_view>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

Stop* TransportCatalogue::AddEmptyStop(const std::string& name) {
    stops_.push_back({ name, {}, {} });
    Stop* list_pointer = &stops_.back();
    stops_reference_.insert({ static_cast<std::string_view>((*list_pointer).name), list_pointer });
    return list_pointer;
}

Stop* TransportCatalogue::AddStopCoordinate(Stop* stop, const Coordinates& coordinate) {
    (*stop).coordinates = coordinate;
    return stop;
}

Stop* TransportCatalogue::AddStopDistance(Stop* stop, const std::pair<std::string, uint32_t> distostop) {
    auto it = stops_reference_.find(distostop.first);
    if (it != stops_reference_.end()) {
        auto it2 = (*stop).distances.find((*it).first);
        if (it2 != (*stop).distances.end()) {
            (*it2).second = distostop.second;
        }
        else {
            (*stop).distances.insert({ (*it).first, distostop.second });
        }

        auto it3 = (*it).second->distances.find((*stop).name);
        if (it3 == (*it).second->distances.end()) {
            (*it).second->distances.insert({ (*stop).name, distostop.second });
        }
    }
    return stop;
}

void TransportCatalogue::AddStop(const std::string& name, const StopDataParce& stop_data) {
    Stop* list_pointer;
    auto it = stops_reference_.find(name);
    if (it == stops_reference_.end()) {
        list_pointer = AddEmptyStop(name);
    } else {
        list_pointer = (*it).second;
    }
    
    stop_and_buses_.insert({ static_cast<std::string_view>((*list_pointer).name), {} });
    AddStopCoordinate(list_pointer, stop_data.coordinates);
    for (const auto& dist : stop_data.distances) {
        auto it2 = stops_reference_.find(dist.first);
        if (it2 == stops_reference_.end()) {
            AddEmptyStop(dist.first);
        }
        AddStopDistance(list_pointer, dist);
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
    try {
        iter = routes_.find(bus_name);
    }
    catch (std::out_of_range& t) {
        return;
    }

    for (const auto& bus : (iter->second).stops_list) {
        try {
            std::set<Route*, CompareRoutes>& my_set = stop_and_buses_.at((*bus).name);
            my_set.insert(&(iter->second));
        }
        catch (std::out_of_range& t) {
            continue;
        }
    }
}

RouteData TransportCatalogue::GetRouteData(const std::string_view& bus_name) const {
    const std::vector<Stop*>* route;
    try {
        route = &routes_.at(static_cast<std::string>(bus_name)).stops_list;
    }
    catch (std::out_of_range& t) {
        return { 0, 0, 0.0, 0.0 };
    }

    int stops_number = (*route).size();
    std::unordered_set <Stop*, StopHasher> iters;
    double geo_distance = 0;
    double route_distance = 0;
    Coordinates coordinate = (*(*route).begin())->coordinates;
    Stop* previous_stop = *(*route).begin();
    bool first = true;
    for (const auto& stop : *route) {
        if (first) {
            first = false;
        }
        else {
            try {
                route_distance += previous_stop->distances.at(stop->name);
            }
            catch (const std::out_of_range& e) {

            }
            
        }
        geo_distance += ComputeDistance(coordinate, stop->coordinates);
        iters.insert(stop);
        coordinate = stop->coordinates;
        previous_stop = stop;
    }
    double curvature = route_distance / geo_distance;
    int uniq_stops_number = iters.size();
    return { stops_number , uniq_stops_number , route_distance , curvature };
}

StopInfo TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
    StopInfo answer;
    try {
        auto pointer = &stop_and_buses_.at(stop_name);
        answer = { 0, *pointer };
    }
    catch (std::out_of_range& t) {
        answer = { 1, {} };
    }
    return answer;
}

size_t StopHasher::operator()(const Stop* stop_name) const {

    return std::hash<std::string>{} ((*stop_name).name);

}