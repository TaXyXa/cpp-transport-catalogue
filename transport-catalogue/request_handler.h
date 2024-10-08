#pragma once

#include <optional>
#include <unordered_set>

#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

class RequestHandler {
public:

    RequestHandler(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer, TransportRouter& router);

    // Возвращает информацию о маршруте (запрос Bus)
    RouteData GetRouteData(const std::string_view& bus_name) const;

    StopInfo GetStopInfo(const std::string_view& bus_name) const;

    using BusPtr = Route*;
    const std::unordered_set<BusPtr, RouteHasher>* GetBusesByStop(const std::string_view& stop_name) const;

    void RenderMap(svg::Document& doc) const;

    void RenderRoute(svg::Document& doc, std::string route_name) const;

    Stop* AddStop(const std::string& name, const Coordinates& coordinates) const;

    void AddRoute(const std::string& bus_name, const std::vector<std::string_view>& stops_vector, 
        size_t end_stop_number, bool is_roundtrip) const;

    Stop* GetStop(const std::string& name) const;

    void AddDistance(Stop* curent_stop, Stop* second_stop, uint32_t distance) const;

    void SetSettings(renderer::Setting& setting) const;

    std::optional<BestRouteInfo> MakeRoute(std::string& from, std::string& to);

    void SetRouteSettings(int wait_time, int velocity)  const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    TransportCatalogue& catalogue_;
    renderer::MapRenderer& map_renderer_;
    TransportRouter& router_;
    //тут хранить ссылку на класс с настройками остановок
};
