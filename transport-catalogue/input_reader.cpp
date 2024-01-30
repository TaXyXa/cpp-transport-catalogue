#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>
#include <utility>

namespace input {
namespace parse {
    
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

StopDataParce CoordAndDists(std::string_view str) {
    static const double nan = std::nan("");
    std::vector<std::pair<std::string, uint32_t>> distances;
    //парсинг координат
    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {{nan, nan}, std::move(distances)};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2, comma2 - not_space2)));
    //парсинг расстояний
    auto iter_comma = comma2;

    uint32_t distance;
    std::string stop_name;

    while (iter_comma != str.npos) {
        auto iter_comma2 = str.find(',', iter_comma + 1);
        auto not_space3 = str.find_first_not_of(' ', iter_comma + 1);
        auto metr = str.find_first_of('m', iter_comma + 1);
        distance = std::stoi(std::string(str.substr(not_space3, metr - not_space3)));
        //теперь исключаем "to" и парсим название остановки
        not_space3 = str.find_first_not_of(' ', metr + 1);
        not_space3 = str.find_first_of(' ', not_space3);
        stop_name = Trim(std::string(str.substr(not_space3, iter_comma2 - not_space3)));
        distances.push_back({stop_name, distance});
        iter_comma = iter_comma2;
    }

    return {{lat, lng}, std::move(distances)};
}
    
std::vector<std::string_view> Route(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
} 

CommandDescription Command(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}   
}
    
void Reader::ParseLine(std::string_view line) {
    auto command_description = parse::Command(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

    
void Reader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    for (auto command : commands_) {
        if (command.command == "Stop") {
            StopDataParce stop_data = parse::CoordAndDists(command.description);
            Stop* curent_stop = catalogue.AddStop(command.id, stop_data.coordinates);
            catalogue.AddDistance(curent_stop, stop_data.distances);
        }
    }
    for (auto command : commands_) {
        if (command.command == "Bus") {
            catalogue.AddRoute(command.id, parse::Route(command.description));
        }
    }
} 
}