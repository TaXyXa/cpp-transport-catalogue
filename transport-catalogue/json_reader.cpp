#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <utility>

#include "domain.h"
#include "geo.h"
#include "json.h"
#include "request_handler.h"

using namespace std::literals;

namespace json {
	namespace parse {

		StopDataParce CoordAndDists(const Dict& stop_data) {
			StopDataParce answer;
			answer.coordinates = { stop_data.at("latitude").AsDouble(), stop_data.at("longitude").AsDouble() };
			auto iter = stop_data.find("road_distances");
			if (iter != stop_data.end()) {
				for (const auto& element : (*iter).second.AsMap()) {
					answer.distances.push_back({ element.first, element.second.AsDouble() });
				}
			}
			return answer;
		}

		std::vector<std::string_view> Route(const Array& stops, bool is_roundtrip) {
			std::vector<std::string_view> answer;
			for (const auto& stop : stops) {
				answer.push_back(stop.AsString());
			}
			if (!is_roundtrip) {
				for (int i = answer.size()-2; i >= 0; i--) {
					answer.push_back(answer[i]);
				}
			}
			return answer;
		}

		std::vector<std::string_view> EndRoute(const Array& stops, bool is_roundtrip) {
			std::vector<std::string_view> answer;
			answer.push_back(stops.begin()->AsString());
			if (!is_roundtrip && (stops.begin()->AsString() != (stops.end() - 1)->AsString())) {
				answer.push_back((stops.end() - 1)->AsString());
			}
			return answer;
		}
	}

	namespace print {
		
		void BusCommand(Dict& dict, RouteData& route_info) {
			if (route_info.request_status == RequestStatus::good) {
				dict.insert({ "curvature", route_info.curvature });
				dict.insert({ "route_length", route_info.route_distance });
				dict.insert({ "stop_count", static_cast<int>(route_info.stops_number) });
				dict.insert({ "unique_stop_count", static_cast<int>(route_info.uniq_stops_number) });
			}
			if (route_info.request_status == RequestStatus::bad) {
				Node message = Node("not found"s);
				dict.insert({ "error_message", message });
			}
		}

		void StopCommand(Dict& dict, const StopInfo& stop_info) {
			if (stop_info.request_status == RequestStatus::good) {
				//буферный контейнер для сортировки, по какой то причине тренажер не компилировался с сортировкой через лямбду
                std::vector<std::string> bus_names;
				for (const auto& bus : *stop_info.buses) {
					bus_names.push_back(bus->name_);
				}
				std::sort(bus_names.begin(), bus_names.end(), [](std::string lhr, std::string rhr) {
					return std::lexicographical_compare(lhr.begin(), lhr.end(), rhr.begin(), rhr.end());
					});
                Array buses;
                buses.reserve(bus_names.size());
                for (const auto& bus : bus_names) {
                    buses.push_back(bus);
                }
				dict.insert({ "buses", buses });
			}
			if (stop_info.request_status == RequestStatus::bad) {
				Node message = Node("not found"s);
				dict.insert({ "error_message", message });
			}
		}

		void MapCommand(Dict& dict, svg::Document&& doc) {
			std::stringstream to_string_flow;
			doc.Render(to_string_flow);
			std::string map_route = to_string_flow.str();
			dict.insert({ "map", map_route });
		}
	}

	Reader::Reader(RequestHandler& request_handler)
		:request_handler_(request_handler)
	{

	}

	svg::Color PrintColor(const json::Node& node) {
		svg::Color curent_color;
		if (node.IsArray()) {
			json::Array color = node.AsArray();
			if (color.size() == 3) {
				svg::Rgb color_case{ static_cast<uint8_t>(color[0].AsDouble()), static_cast<uint8_t>(color[1].AsDouble()), 
                                    static_cast<uint8_t>(color[2].AsDouble()) };
				curent_color = color_case;
			}
			else if (color.size() == 4) {
				svg::Rgba color_case{ static_cast<uint8_t>(color[0].AsDouble()), static_cast<uint8_t>(color[1].AsDouble()), 
                                     static_cast<uint8_t>(color[2].AsDouble()), color[3].AsDouble() };
				curent_color = color_case;
			}
		}
		else if (node.IsString()) {
			curent_color = node.AsString();
		}
		return curent_color;
	}

	void Reader::SetInfo(std::istream& input) {
		Document info_base_ = Load(input);
		if (!info_base_.GetRoot().IsMap()) {
			throw std::logic_error("base_request is not a map");
		}
		const Dict& root = info_base_.GetRoot().AsMap();
		auto iter = root.find("base_requests");
		if (iter != root.end()) {
			std::vector<std::pair<std::pair<Stop*, std::string>, uint32_t>> distances;
			for (const Node& element : (*iter).second.AsArray()) {
				Dict curent_map = element.AsMap();
				if (curent_map.at("type").AsString() == "Stop") {
					StopDataParce stop_data = parse::CoordAndDists(curent_map);
					Stop* curent_stop = request_handler_.AddStop(curent_map.at("name").AsString(), stop_data.coordinates);
					for (auto& stop_dist : stop_data.distances) {
						distances.push_back({ {curent_stop, stop_dist.first}, stop_dist.second });
					}
				}
			}
			for (const Node& element : (*iter).second.AsArray()) {
				Dict curent_map = element.AsMap();
				if (curent_map.at("type").AsString() == "Bus") {
					request_handler_.AddRoute(
						curent_map.at("name").AsString(), 
					    parse::Route(curent_map.at("stops").AsArray(), curent_map.at("is_roundtrip").AsBool()), 
					    parse::EndRoute(curent_map.at("stops").AsArray(), curent_map.at("is_roundtrip").AsBool())
					);
				}
			}
			for (auto& dist : distances) {
				Stop* second_stop = request_handler_.GetStop(dist.first.second);
				if (second_stop != nullptr) {
					request_handler_.AddDistance(dist.first.first, second_stop, dist.second);
				}
			}
		}
		
		iter = root.find("render_settings");
		if (iter != root.end()) {
			Dict curent_set = iter->second.AsMap();
			renderer::Setting setting;
			setting.width = curent_set.at("width").AsDouble();
			setting.height = curent_set.at("height").AsDouble();
			setting.padding = curent_set.at("padding").AsDouble();
			setting.line_width = curent_set.at("line_width").AsDouble();
			setting.stop_radius = curent_set.at("stop_radius").AsDouble();
			setting.bus_label_font_size = curent_set.at("bus_label_font_size").AsDouble();
			double x = curent_set.at("bus_label_offset").AsArray()[0].AsDouble();
			double y = curent_set.at("bus_label_offset").AsArray()[1].AsDouble();
			setting.bus_label_offset = { x, y };
			setting.stop_label_font_size = curent_set.at("stop_label_font_size").AsDouble();
			x = curent_set.at("stop_label_offset").AsArray()[0].AsDouble();
			y = curent_set.at("stop_label_offset").AsArray()[1].AsDouble();
			setting.stop_label_offset = { x, y };

			setting.underlayer_color = PrintColor(curent_set.at("underlayer_color"));
			setting.underlayer_width = curent_set.at("underlayer_width").AsDouble();
			for (const auto& curent_array : curent_set.at("color_palette").AsArray()) {
				setting.color_palette.push_back(PrintColor(curent_array));
			}
			request_handler_.SetSettings(setting);
		}
		
		//stat parsing
		iter = root.find("stat_requests");
		if (iter != root.end()) {
			for (const Node& element : (*iter).second.AsArray()) {
				Dict curent_map = element.AsMap();
				int id = 0;
				std::string type;
				std::string name;
				auto iter_id = curent_map.find("id");
				if (iter_id != curent_map.end()) {
					id = iter_id->second.AsInt();
				}
				auto iter_type = curent_map.find("type");
				if (iter_type != curent_map.end()) {
					type = iter_type->second.AsString();
				}
				auto iter_name = curent_map.find("name");
				if (iter_name != curent_map.end()) {
					name = iter_name->second.AsString();
				}
				requests_.push_back({ id, type, name });
			}
		}
	}

	void Reader::PrintInfo(std::ostream& output) {
		Array root;
		for (Requests request : requests_) {
			Dict ans;
			ans.insert({ "request_id", request.id });

			if (request.type == "Stop") {
				StopInfo stop_info = request_handler_.GetStopInfo(request.name);
				print::StopCommand(ans, stop_info);
			}
			if (request.type == "Bus") {
				RouteData route_info = request_handler_.GetRouteData(request.name);
				print::BusCommand(ans, route_info);
			}
			if (request.type == "Map") {
				svg::Document doc;
				request_handler_.RenderMap(doc);
				print::MapCommand(ans, std::move(doc));
			}
			root.push_back(ans);
		}
		Document answer(root);
		Print(answer, output);
	}

}
