#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_set>

#include "map_renderer.h"
#include "geo.h"
#include "svg.h"

namespace renderer {

	void MapRenderer::SetSettings(Setting& setting) {
		render_setting_ = setting;
	}

	std::unordered_set<Coordinates, CoordinateHasher> GetAllPoints(const std::vector<const Route*>& routs) {
		std::unordered_set<Coordinates, CoordinateHasher> all_points;
		for (const auto& route_ptr : routs) {
			for (const auto& stop_ptr : route_ptr->stops_list) {
				all_points.insert(stop_ptr->coordinates);
			}
		}
		return all_points;
	}

	std::vector<svg::Color> MakeRoutsPalete(Setting& render_setting, const std::vector<const Route*>& routs) {
		std::vector<svg::Color> routs_colors;
		auto color_iter = render_setting.color_palette.begin();
		const size_t size = routs.size();
		for (size_t i = 0; i < size; i++) {
			if (color_iter == render_setting.color_palette.end()) {
				color_iter = render_setting.color_palette.begin();
			}
			routs_colors.push_back(*color_iter);
			++color_iter;
		}
		return routs_colors;
	}

	void MapRenderer::RenderMap(svg::Document& document, std::vector<const Route*> routs) {
		//scaling map
		std::unordered_set<Coordinates, CoordinateHasher> all_points = GetAllPoints(routs);
		const SphereProjector projector(all_points.begin(), all_points.end(), 
			render_setting_.width, render_setting_.height, render_setting_.padding);
		std::vector<svg::Color> routs_colors = MakeRoutsPalete(render_setting_, routs);
		std::vector<Stop*> stops_list;
		std::vector<Route_End_Data> route_ens_stops_list;
		//makes lines of routs
		auto line_color_ptr = routs_colors.begin();

		for (const auto& route_ptr : routs) {
			svg::Polyline route_line;
			for (const auto& stop_ptr : route_ptr->stops_list) {
				svg::Point pp = projector(stop_ptr->coordinates);
				route_line.AddPoint(pp);
			}
			route_line.SetFillColor("none").SetStrokeColor(*line_color_ptr).SetStrokeWidth(render_setting_.line_width).
				SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(route_line);
			//add stops to container
			for (const auto& stop_ptr : route_ptr->stops_list) {
				auto curent_iter = find(stops_list.begin(), stops_list.end(), stop_ptr);
				if (curent_iter == stops_list.end()) {
					stops_list.push_back(stop_ptr);
				}
			}
			//add ends stops
			for (const auto& stop : route_ptr->end_stops_list) {
				route_ens_stops_list.push_back({ stop, route_ptr->name_, *line_color_ptr });
			}
			++line_color_ptr;
		}

		sort(stops_list.begin(), stops_list.end(), [](const Stop* lhr, const Stop* rhr) {
			return lhr->name < rhr->name;
			});

		for (const auto& [end_stop, name, color] : route_ens_stops_list) {
			svg::Text route_name;
			route_name.SetPosition(projector(end_stop->coordinates)).SetOffset(render_setting_.bus_label_offset)
				.SetFontSize(render_setting_.bus_label_font_size).SetFontFamily("Verdana")
				.SetFontWeight("bold").SetData(name);
			svg::Text route_name_underlayer = route_name;
			route_name.SetFillColor(color);
			route_name_underlayer.SetFillColor(render_setting_.underlayer_color).SetStrokeColor(render_setting_.underlayer_color)
				.SetStrokeWidth(render_setting_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(route_name_underlayer);
			document.Add(route_name);
		}

		for (const auto& stop : stops_list) {
			svg::Circle stop_circle;
			stop_circle.SetCenter(projector(stop->coordinates)).SetRadius(render_setting_.stop_radius)
				.SetFillColor("white");
			document.Add(stop_circle);
		}

		for (const auto& stop : stops_list) {
			svg::Text stop_name;
			stop_name.SetPosition(projector(stop->coordinates)).SetOffset(render_setting_.stop_label_offset)
				.SetFontSize(render_setting_.stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name);
			svg::Text stop_name_underlayer = stop_name;
			stop_name.SetFillColor("black");
			stop_name_underlayer.SetFillColor(render_setting_.underlayer_color).SetStrokeColor(render_setting_.underlayer_color)
				.SetStrokeWidth(render_setting_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(stop_name_underlayer);
			document.Add(stop_name);
		}
	}
}
