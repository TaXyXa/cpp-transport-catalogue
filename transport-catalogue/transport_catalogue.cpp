#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <deque>
#include <functional>
#include <tuple>
#include <string>
#include <string_view>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"

void TransportCatalogue::AddStop(std::string& name, Coordinates coordinates) {
	auto deque_end = stops_.end();
	StopAndCoordinates* deque_pointer = &(*(stops_.insert(deque_end, { std::move(name), coordinates })));
	stops_reference_.insert({ static_cast<std::string_view>((*deque_pointer).name), deque_pointer });
    stop_and_buses_.insert({ static_cast<std::string_view>((*deque_pointer).name), {} });
}

void TransportCatalogue::AddRoute(std::string& bus_name, const std::vector<std::string_view>& stops_vector) {
	std::vector<StopAndCoordinates*> route;
	for (const auto stop_name: stops_vector) {
		StopAndCoordinates* it = stops_reference_.at(stop_name);
		route.push_back(it);
	}
    std::string buff_bus_name = bus_name;
    routes_.insert({ std::move(bus_name), std::move(route) });
    
    std::unordered_map<std::string, std::vector<StopAndCoordinates*>>::iterator iter;
    try {
        iter = routes_.find(buff_bus_name);
    } catch (std::out_of_range& t) {
        return;
    }
    
    for (const auto& bus:iter->second) {
        try {
            std::set<std::string_view>& my_set = stop_and_buses_.at((*bus).name);
            my_set.insert(iter->first);
        } catch (std::out_of_range& t) {
            continue;
        }
    }
}

std::tuple<int, int, double> TransportCatalogue::GetRouteData(const std::string& bus_name) const {
	const std::vector<StopAndCoordinates*>* route;
    try {
        route = &routes_.at(bus_name);
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

std::set<std::string_view> TransportCatalogue::GetStopData(const std::string& stop_name) const {

    std::set<std::string_view> answer;
    try {
        auto pointer = &stop_and_buses_.at(stop_name);
        answer = *pointer;
    } catch (std::out_of_range& t) {
        answer = {"no found"};
    }
    return answer;
}

size_t StopHasher::operator()(const StopAndCoordinates* stop_name) const {
    
    return std::hash<std::string>{} ((*stop_name).name);
    
}