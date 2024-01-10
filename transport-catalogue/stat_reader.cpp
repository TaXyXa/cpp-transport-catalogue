#include "stat_reader.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <string_view>
#include <set>
#include <tuple>

using namespace std::literals;

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {
    auto not_space = request.find_first_not_of(' ');
    auto space = request.find_first_of(' ', not_space);
    auto not_space2 = request.find_first_not_of(' ', space);
    auto last = request.find_first_of('\\', not_space2);

    std::string command = std::string(request.substr(not_space, space - not_space));
    std::string description = std::string(request.substr(not_space2, last - not_space2 + space));

    if (command == "Bus") {
        std::tuple<int, int, double> answer = tansport_catalogue.GetRouteData(description);
        
        if (std::get<0>(answer) == 0) {
            output << command << " "s << description << ": not found"s << std::endl;
            return;
        }
        output << command << " "s << description << ": "s << std::get<0>(answer) << " stops on route, "s << std::get<1>(answer) << " unique stops, "s << std::get<2>(answer) << " route length"s << std::endl;
    }
    
    if (command == "Stop") {
        std::set<std::string_view> answer = tansport_catalogue.GetStopData(description);
        if (answer.empty()) {
            output << command << " "s << description << ": no buses"s << std::endl;
            return;
        }
        if (answer.size()==1 && *(answer.begin())=="no found") {
            output << command << " "s << description << ": not found"s << std::endl;
            return;
        }
        output << command << " "s << description << ": buses"s;
        for (const auto& bus:answer) {
            output << " " << bus;
        }
        output << std::endl;
    }


}