#include <algorithm>
#include <cstdint>
#include <functional>
#include <forward_list>
#include <string>
#include <string_view>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "transport_catalogue.h"
#include "geo.h"
#include "domain.h"

Stop* TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates) {
    Stop* curent_stop = nullptr;
    auto it = stops_reference_.find(name);
    if (it == stops_reference_.end()) {
        stops_.push_front({ name, coordinates });
        curent_stop = &stops_.front();
    }
    else {
        curent_stop = (*it).second;
        (*curent_stop).coordinates = coordinates;
    }
    stops_reference_.insert({ static_cast<std::string_view>((*curent_stop).name), curent_stop });
    stop_and_buses_.insert({ static_cast<std::string_view>((*curent_stop).name), {} });
    return curent_stop;
}

Stop* TransportCatalogue::GetStop(const std::string& name) {
    auto it = stops_reference_.find(name);
    if (it == stops_reference_.end()) {
        return nullptr;
    }
    else {
        return it->second;
    }
}

void TransportCatalogue::AddDistance(Stop* curent_stop, Stop* second_stop, uint32_t distance) {
    distances_.insert({ { curent_stop, second_stop }, distance });
}

uint32_t TransportCatalogue::GetDistance(Stop* curent_stop, Stop* second_stop) const {
    auto iter = distances_.find({ curent_stop, second_stop });
    if (iter != distances_.end()) {
        return (*iter).second;
    }
    iter = distances_.find({ second_stop, curent_stop });
    if (iter != distances_.end()) {
        return (*iter).second;
    }
    return ComputeDistance(curent_stop->coordinates, second_stop->coordinates);
}

void TransportCatalogue::AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector,
    size_t end_stop_number, bool is_roundtrip) {
    Route route = { bus_name, {}, {}, is_roundtrip };
    for (const auto stop_name : stops_vector) {
        Stop* it = stops_reference_.at(stop_name);
        route.stops_list.push_back(it);
    }
    route.end_stop_number_ = end_stop_number;

    routes_.insert({ bus_name, std::move(route)});

    std::unordered_map<std::string, Route>::iterator iter;
    iter = routes_.find(bus_name);
    if (iter == routes_.end()) {
        return;
    }

    for (const auto& bus : (iter->second).stops_list) {
        auto iter2 = stop_and_buses_.find((*bus).name);
        if (iter2 != stop_and_buses_.end()) {
            (*iter2).second.insert(&(iter->second));
        }
    }
}

const Route* TransportCatalogue::GetRoute(const std::string& name) const {
    auto iter = routes_.find(name);
    if (iter != routes_.end()) {
        return &(iter->second);
    }
    return nullptr;
}

std::vector<const Route*> TransportCatalogue::GetAllRouts() const {
    std::vector<const Route*> routs;
    routs.reserve(routes_.size());
    for (const auto& route : routes_) {
        const Route* route_ptr = &route.second;
        routs.push_back(route_ptr);
    }
    std::sort(routs.begin(), routs.end(), [](const Route* lhr, const Route* rhr) {
        return lhr->name_ < rhr->name_;
        });
    return routs;
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
            route_distance += GetDistance(previous_stop, stop);
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
    }
    else {
        return { RequestStatus::bad, 0, 0, 0.0, 0.0 };
    }
    const int stops_number = (*route).size();
    double geo_distance = GetGeoDistance(route);
    double route_distance = GetRouteDistance(route);
    const double curvature = route_distance / geo_distance;
    const int uniq_stops_number = GetUniqStops(route);
    return { RequestStatus::good, stops_number , uniq_stops_number , route_distance , curvature };
}

StopInfo TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
    StopInfo ans = { RequestStatus::bad , nullptr};
    auto iter = stop_and_buses_.find(stop_name);
    if (iter != stop_and_buses_.end()) {
        ans.request_status = RequestStatus::good;
        ans.buses = &(iter->second);
        return ans;
    }
    else {
        return ans;
    }
}

size_t TransportCatalogue::GetStopsNumber() const {
    return stops_reference_.size();
}

size_t TransportCatalogue::GetRoutsNumber() const {
    return routes_.size();
}

const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetAllStop() const {
    return stops_reference_;
}
