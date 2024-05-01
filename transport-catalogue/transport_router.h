#pragma once
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"


class TransportRouter {
public:
	using Graph = graph::DirectedWeightedGraph<double>;
	TransportRouter(TransportCatalogue& catalogue);

	BestRouteInfo MakeRoute(const Stop* from, const Stop* to);

	void SetSetting(int wait_time, int velocity);

private:
	TransportCatalogue& catalogue_;
	RouteSetting setting_;
	std::shared_ptr<Graph> graph_;
	std::unique_ptr<graph::Router<double>> router_ptr_;

	std::unordered_map<const Stop*, size_t, StopHasher> comming_stop_;
	std::unordered_map<const Stop*, size_t, StopHasher> leave_stop_;
	std::unordered_map <size_t, RouteItem> edge_stops_;

	void MakeGraph();

	void MakeVertex();

	void MakeRouteEdges();

	void AddEdge(size_t first_stop, size_t second_stop, double weight, 
				Type edge_type, std::string_view name, int span_count);

	template <typename VSIter>
	void MakeSimpleRoute(VSIter first_stop, VSIter last_stop, const Route* route);

	BestRouteInfo MakeItems(std::vector<size_t> edges);

};