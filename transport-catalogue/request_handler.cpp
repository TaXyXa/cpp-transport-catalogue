#include <unordered_set>

#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "request_handler.h"

RequestHandler::RequestHandler(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer)
	:catalogue_(catalogue), map_renderer_(map_renderer)
{}

RouteData RequestHandler::GetRouteData(const std::string_view& bus_name) const {
	return catalogue_.GetRouteData(bus_name);
}

using BusPtr = Route*;
const std::unordered_set<BusPtr, RouteHasher>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return catalogue_.GetStopInfo(stop_name).buses;
}

StopInfo RequestHandler::GetStopInfo(const std::string_view& bus_name) const {
	return catalogue_.GetStopInfo(bus_name);
}

void RequestHandler::RenderMap(svg::Document& doc) const {
	map_renderer_.RenderMap(doc, catalogue_.GetAllRouts());
}

void RequestHandler::RenderRoute(svg::Document& doc, std::string route_name) const {
	std::vector<const Route*> routs; 
	routs.push_back(catalogue_.GetRoute(route_name));
	map_renderer_.RenderMap(doc, routs);
}

Stop* RequestHandler::AddStop(const std::string& name, const Coordinates& coordinates) const {
	return catalogue_.AddStop(name, coordinates);
}

void RequestHandler::AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector, 
	const std::vector<std::string_view>& end_stops_vector) const {
	catalogue_.AddRoute(bus_name, stops_vector, end_stops_vector);
}

Stop* RequestHandler::GetStop(const std::string& name) const {
	return catalogue_.GetStop(name);
}

void RequestHandler::AddDistance(Stop* curent_stop, Stop* second_stop, uint32_t distance) const {
	catalogue_.AddDistance(curent_stop, second_stop, distance);
}

void RequestHandler::SetSettings(renderer::Setting& setting) const {
	map_renderer_.SetSettings(setting);
}
