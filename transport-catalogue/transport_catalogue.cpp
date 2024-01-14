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

void TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates) {
	auto list_end = stops_.end();
    stops_.insert(list_end, { std::move(name), coordinates });
	StopAndCoordinates* list_pointer = &stops_.back();
	stops_reference_.insert({ static_cast<std::string_view>((*list_pointer).name), list_pointer });
    stop_and_buses_.insert({ static_cast<std::string_view>((*list_pointer).name), {} });
}

void TransportCatalogue::AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector) {
	Route route = {bus_name, {}};
	for (const auto stop_name: stops_vector) {
		StopAndCoordinates* it = stops_reference_.at(stop_name);
		route.stops_list.push_back(it);
	}
    routes_.insert({ bus_name, std::move(route) });
   
    std::unordered_map<std::string, Route>::iterator iter;
    try {
        iter = routes_.find(bus_name);
    } catch (std::out_of_range& t) {
        return;
    }
    
    for (const auto& bus:(iter->second).stops_list) {
        try {
            std::set<Route*, CompareRoutes>& my_set = stop_and_buses_.at((*bus).name);
            my_set.insert(&(iter->second));
        } catch (std::out_of_range& t) {
            continue;
        }
    }
}

RouteData TransportCatalogue::GetRouteData(const std::string_view& bus_name) const {
	const std::vector<StopAndCoordinates*>* route;
    try {
        route = &routes_.at(static_cast<std::string>(bus_name)).stops_list;
    } catch (std::out_of_range& t) {
        return {0, 0, 0.0};
    }
    
	int stops_number = (*route).size();
	std::unordered_set <StopAndCoordinates*, StopHasher> iters;
	double route_distance = 0;
	Coordinates coordinate = (*(*route).begin())->coordinates;
	for (const auto stop : *route) {
		route_distance += ComputeDistance(coordinate, stop->coordinates);
		iters.insert(stop);
		coordinate = stop->coordinates;
	}
	int uniq_stops_number = iters.size();
	return { stops_number , uniq_stops_number , route_distance };
}

StopData TransportCatalogue::GetStopData(const std::string_view& stop_name) const {
    StopData answer;
    try {
        auto pointer = &stop_and_buses_.at(stop_name);
        answer = {0, *pointer};
    } catch (std::out_of_range& t) {
        answer = {1, {}};
    }
    return answer;
}

size_t StopHasher::operator()(const StopAndCoordinates* stop_name) const {
    
    return std::hash<std::string>{} ((*stop_name).name);
    
}