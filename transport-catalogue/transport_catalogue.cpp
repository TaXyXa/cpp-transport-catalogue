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
    //не могу полностью избавиться от добавления пустых остановок, так как для хранения 
    //расстояний я хочу использовать легкие указатели Stop*, с помощью которых сразу и буду 
    //искать нужное расстоние. Иначе я вижу только вариант с хранением string для каждой 
    //дистанции
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

void TransportCatalogue::AddStop(const std::string& name, const StopDataParce& stop_data) {
    Stop* curent_stop = AddEmptyStop(name, stop_data.coordinates);
    stop_and_buses_.insert({ static_cast<std::string_view>((*curent_stop).name), {} });

    for (const auto& dist : stop_data.distances) {
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
    const std::vector<Stop*>* route = nullptr;
    auto it_route = routes_.find(static_cast<std::string>(bus_name));
    if (it_route != routes_.end()) {
        route = &(*it_route).second.stops_list;
    } else {
        return { 0, 0, 0.0, 0.0 };
    }

    const size_t stops_number = (*route).size();
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
            auto it_dist = distances_.find({ previous_stop, stop });
            if (it_dist != distances_.end()) {
                route_distance += (*it_dist).second;
            } 
            else {
                it_dist = distances_.find({ stop, previous_stop });
                //да действительно здесь получилось сэкономить, так как расстояния теперь не дублируются 
                //за счет контейнера с парой указателей. Спасибо!
                if (it_dist != distances_.end()) {
                    route_distance += (*it_dist).second;
                }
            }        
        }
        geo_distance += ComputeDistance(coordinate, stop->coordinates);
        iters.insert(stop);
        coordinate = stop->coordinates;
        previous_stop = stop;
    }
    const double curvature = route_distance / geo_distance;
    const size_t uniq_stops_number = iters.size();
    return { stops_number , uniq_stops_number , route_distance , curvature };
}



StopInfo TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
    StopInfo answer;
    try {
        auto pointer = &stop_and_buses_.at(stop_name);
        answer = { Requvest_Status::good, *pointer };
    }
    catch (std::out_of_range& t) {
        answer = { Requvest_Status::bad, {} };
    }
    return answer;
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