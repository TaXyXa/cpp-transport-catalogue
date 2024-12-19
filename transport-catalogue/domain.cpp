#include "domain.h"

bool CompareRoutes::operator()(const Route* route1, const Route* route2) const {
    return route1->name_ < route2->name_;
}

size_t RouteHasher::operator()(const Route* route_name) const {
    return std::hash<std::string>{} (route_name->name_);
}

size_t DistanceHasher::operator()(const std::pair<Stop*, Stop*> stop_name) const {
    size_t hash = std::hash<std::string>{} ((*stop_name.first).name)
        * std::hash<std::string>{} ((*stop_name.first).name)
        + std::hash<std::string>{} ((*stop_name.second).name);
    return hash;
}

size_t StopHasher::operator()(const Stop* stop_name) const {
    return std::hash<std::string>{} (stop_name->name);
}

size_t CoordinateHasher::operator()(const Coordinates coordinate) const {
    return coordinate.lat*42 + coordinate.lng * 42 * 42;
}