#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <unordered_map>

#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

TransportRouter::TransportRouter(TransportCatalogue& catalogue)
	:catalogue_(catalogue)
	//Передача через конструктор настроек мне не нравится тем что настройки мы получаем уже при чтении данных, 
	//а значит мы должны конструировать весь маршрутизатор либо в json_reader-e, либо внутри фасада что не очень 
	//мне нравится с точки зрения логики разделения программы на блоки, потому что при чтении не джейсона, 
	//а другого формата придется и там не забыть конструктор. 
	//Конструировать Граф тоже не хочу сразу, хотел сделать ленивую инициализацию, так как не всегда требуется построение маршрута
	//Если в чем то не прав - с радостью приму замечания, но не спорить не могу по натуре :)
{}

std::optional<BestRouteInfo> TransportRouter::MakeRoute(const Stop* from, const Stop* to) {
	if (graph_ == nullptr) {
		MakeGraph();
		router_ptr_ = std::make_unique<graph::Router<double>>(*graph_);
	}
	size_t from_index, to_index;
	auto iter = comming_stop_.find(from);
	if (iter != comming_stop_.end()) {
		from_index = iter->second;
	}
	else {
		return {};
	}
	iter = comming_stop_.find(to);
	if (iter != comming_stop_.end()) {
		to_index = iter->second;
	}
	else {
		return {};
	}
	auto route = router_ptr_->BuildRoute(from_index, to_index);
	if (route) {
		return MakeItems(route->edges);
	}
	else {
		return {};
	}
}

void TransportRouter::MakeGraph() {
	graph_ = std::make_unique<Graph>(2 * catalogue_.GetStopsNumber());
	MakeVertex();
	MakeRouteEdges();

}

void TransportRouter::MakeVertex() {
	size_t index = 0;
	const std::unordered_map<std::string_view, Stop*>& all_stops = catalogue_.GetAllStop();
	size_t size = all_stops.size();
	for (const auto& stop : all_stops) {
		comming_stop_.insert({ stop.second, index });
		leave_stop_.insert({ stop.second, size+index });
		AddEdge(index, size + index, setting_.wait_time_, Type::Wait, stop.first, 0);
		index++;
	}
}

void TransportRouter::MakeRouteEdges() {
	std::vector<const Route*> routes = catalogue_.GetAllRouts();
	
	for (const auto& route : routes) {
		auto midle_stop_iter = route->stops_list.begin();
		if (route->is_roundtrip_) {
			MakeSimpleRoute(route->stops_list.begin(), route->stops_list.end(), route);
		}
		else {
			midle_stop_iter += route->end_stop_number_;
			MakeSimpleRoute(route->stops_list.begin(), midle_stop_iter, route);
			MakeSimpleRoute(midle_stop_iter-1, route->stops_list.end(), route);							
		}
		
	}
}

void TransportRouter::AddEdge( size_t first_stop, size_t second_stop, double weight, 
								Type edge_type, std::string_view name, int span_count) {
	size_t edge_index = graph_->AddEdge({ first_stop, second_stop, weight });
	edge_stops_.insert({ edge_index, {edge_type, name, weight, span_count } });

}

template <typename VSIter>
void TransportRouter::MakeSimpleRoute(VSIter first_stop, VSIter last_stop,
	const Route* route) {
	VSIter start_stop = first_stop;
	
	while (start_stop != last_stop) {
		VSIter curent_stop = start_stop + 1;
		double weight = 0;
		int span_count = 1;
		while (curent_stop != last_stop) {
			weight += 1. * catalogue_.GetDistance(*(curent_stop-1), *curent_stop) / setting_.velocity_;
			auto previous_stop_iter = leave_stop_.find(*start_stop);
			auto curent_stop_iter = comming_stop_.find(*curent_stop);
            std::string_view route_name = route->name_;
			AddEdge(previous_stop_iter->second, curent_stop_iter->second, weight, 
					Type::Bus, route_name, span_count);

			++curent_stop;
			++span_count;
		}
		++start_stop;
	}
}

BestRouteInfo TransportRouter::MakeItems(std::vector<size_t> edges) {
	std::vector<RouteItem> route_items;
	size_t size = edges.size();
	double weight=0;

    for (size_t i = 0; i < size; i++) {
		RouteItem item = edge_stops_.find(edges[i])->second;
		weight += item.time;
		route_items.push_back(item);
	}

	return { weight, route_items };
}

void TransportRouter::SetSetting(int wait_time, int velocity) {
	setting_.wait_time_ = static_cast<double>(wait_time);
	setting_.velocity_ = velocity/0.06;
}