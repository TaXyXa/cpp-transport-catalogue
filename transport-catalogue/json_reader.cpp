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
#include "json_builder.h"
#include "request_handler.h"

using namespace std::literals;

namespace json {

	namespace print {
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
	}

	namespace parse {

		StopDataParse GetCoordAndDists(const Dict& stop_data) {
			StopDataParse answer;
			answer.coordinates = { stop_data.at("latitude").AsDouble(), stop_data.at("longitude").AsDouble() };
			auto iter = stop_data.find("road_distances");
			if (iter != stop_data.end()) {
				for (const auto& element : (*iter).second.AsDict()) {
					answer.distances.push_back({ element.first, element.second.AsDouble() });
				}
			}
			return answer;
		}

		std::vector<std::string_view> GetStopsRoute(const Array& stops, bool is_roundtrip) {
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

		std::vector<std::string_view> GetEndStopsRoute(const Array& stops, bool is_roundtrip) {
			std::vector<std::string_view> answer;
			answer.push_back(stops.begin()->AsString());
			if (!is_roundtrip && (stops.begin()->AsString() != (stops.end() - 1)->AsString())) {
				answer.push_back((stops.end() - 1)->AsString());
			}
			return answer;
		}

		void ReadBaseRequest (const Dict& root, RequestHandler& request_handler) {
			auto iter = root.find("base_requests");
			if (iter != root.end()) {
				std::vector<std::pair<std::pair<Stop*, std::string>, uint32_t>> distances;
				for (const Node& element : (*iter).second.AsArray()) {
					Dict curent_map = element.AsDict();
					if (curent_map.at("type").AsString() == "Stop") {
						StopDataParse stop_data = parse::GetCoordAndDists(curent_map);
						Stop* curent_stop = request_handler.AddStop(curent_map.at("name").AsString(), stop_data.coordinates);
						for (auto& stop_dist : stop_data.distances) {
							distances.push_back({ {curent_stop, stop_dist.first}, stop_dist.second });
						}
					}
				}
				for (const Node& element : (*iter).second.AsArray()) {
					Dict curent_map = element.AsDict();
					if (curent_map.at("type").AsString() == "Bus") {
						request_handler.AddRoute(
							curent_map.at("name").AsString(), 
							parse::GetStopsRoute(curent_map.at("stops").AsArray(), curent_map.at("is_roundtrip").AsBool()), 
							parse::GetEndStopsRoute(curent_map.at("stops").AsArray(), curent_map.at("is_roundtrip").AsBool())
						);
					}
				}
				for (auto& dist : distances) {
					Stop* second_stop = request_handler.GetStop(dist.first.second);
					if (second_stop != nullptr) {
						request_handler.AddDistance(dist.first.first, second_stop, dist.second);
					}
				}
			}
		}

		void ReadRenderSetting (const Dict& root, RequestHandler& request_handler) {	
			auto iter = root.find("render_settings");
			if (iter != root.end()) {
				Dict curent_set = iter->second.AsDict();
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

				setting.underlayer_color = print::PrintColor(curent_set.at("underlayer_color"));
				setting.underlayer_width = curent_set.at("underlayer_width").AsDouble();
				for (const auto& curent_array : curent_set.at("color_palette").AsArray()) {
					setting.color_palette.push_back(print::PrintColor(curent_array));
				}
				request_handler.SetSettings(setting);
			}
		}

		void ReadStatRequest (const Dict& root, std::vector<Requests>& requests) {
			auto iter = root.find("stat_requests");
			if (iter != root.end()) {
				for (const Node& element : (*iter).second.AsArray()) {
					Dict curent_map = element.AsDict();
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
					requests.push_back({ id, type, name });
				}
			}
		}
	}

	namespace print {
		/*
		Забавно что этот код работает без наследования от Builder-a. По сути, если я правильно понимаю, такая передача не гарантирует ошибку компиляции
		И я не пользовался написаной мной самим системой классов, но я не смог разобраться как сделать это наследование чтобы все работало 
		(просто не смог объявить наследуемые классы без их определения "class AfterValue : public Builder;" не компилируется). 
		Если объясните как это правильно делатьбуду благодарен
		*/
		void PrintBusCommand(Builder& dict, RouteData& route_info) {
			if (route_info.request_status == RequestStatus::good) {
				dict.Key("curvature").Value(route_info.curvature).Key("route_length").Value(route_info.route_distance);
				dict.Key("stop_count").Value(route_info.stops_number).Key("unique_stop_count").Value(route_info.uniq_stops_number);
			}
			if (route_info.request_status == RequestStatus::bad) {
				dict.Key("error_message").Value("not found"s);
			}
		}

		void PrintStopCommand(Builder& dict, const StopInfo& stop_info) {
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
				dict.Key("buses").Value(buses);
			}
			if (stop_info.request_status == RequestStatus::bad) {
				dict.Key("error_message").Value("not found");
			}
		}

		void PrintMapCommand(Builder& dict, svg::Document&& doc) {
			std::stringstream to_string_flow;
			doc.Render(to_string_flow);
			std::string map_route = to_string_flow.str();
			dict.Key("map").Value(map_route);
		}

	}

	Reader::Reader(RequestHandler& request_handler)
		:request_handler_(request_handler)
	{

	}

	void Reader::SetInfo(std::istream& input) {
		Document info_base = Load(input);
		if (!info_base.GetRoot().IsDict()) {
			throw std::logic_error("base_request is not a map");
		}
		const Dict& root = info_base.GetRoot().AsDict();
		parse::ReadBaseRequest (root, request_handler_);
		parse::ReadRenderSetting (root, request_handler_);
		parse::ReadStatRequest (root, requests_);
	}

	void Reader::PrintInfo(std::ostream& output) {
		Builder root = Builder{};
		root.StartArray();
		for (Requests request : requests_) {

			root.StartDict().Key("request_id").Value(request.id);

			if (request.type == "Stop") {
				StopInfo stop_info = request_handler_.GetStopInfo(request.name);
				print::PrintStopCommand(root, stop_info);
			}
			if (request.type == "Bus") {
				RouteData route_info = request_handler_.GetRouteData(request.name);
				print::PrintBusCommand(root, route_info);
			}
			if (request.type == "Map") {
				svg::Document doc;
				request_handler_.RenderMap(doc);
				print::PrintMapCommand(root, std::move(doc));
			}
			root.EndDict();
		}
		Document answer{ root.EndArray().Build() };
		Print(answer, output);
	}

}
