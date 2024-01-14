#include "stat_reader.h"

#include <iostream>
#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <set>
#include <tuple>

using namespace std::literals;

void PrintBusCommand (const TransportCatalogue& tansport_catalogue, std::string& command, 
                      std::string& description, std::ostream& output) {
    RouteData answer = tansport_catalogue.GetRouteData(description);
        
    if (answer.stops_number == 0) {
        output << command << " "s << description << ": not found"s << std::endl;
        return;
    }
    output << command << " "s << description << ": "s 
        << answer.stops_number << " stops on route, "s 
        << answer.uniq_stops_number << " unique stops, "s 
        << answer.route_distance << " route length"s << std::endl;    
}

void PrintStopCommand (const TransportCatalogue& tansport_catalogue, std::string& command, 
                      std::string& description, std::ostream& output) {
    StopData answer = tansport_catalogue.GetStopData(description);
    ////
    switch (answer.requvest_status) 
    {
        case 0:
            if (answer.buses.empty()) {
                output << command << " "s << description << ": no buses"s << std::endl;
                return;
            }
        break;
            
        case 1:
            output << command << " "s << description << ": not found"s << std::endl;
            return; 
        break;
    }
    output << command << " "s << description << ": buses"s;
    for (const auto& bus:answer.buses) {
        output << " " << bus->name_;
    }
    output << std::endl;
}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
    std::ostream& output) {
    auto not_space = request.find_first_not_of(' ');
    auto space = request.find_first_of(' ', not_space);
    auto not_space2 = request.find_first_not_of(' ', space);
    auto last = request.find_first_of('\\', not_space2);

    std::string command = std::string(request.substr(not_space, space - not_space));
    std::string description = std::string(request.substr(not_space2, last - not_space2 + space));

    std::map<std::string, int> mapping = {{"Bus", 1}, {"Stop", 2}};
    
    switch (mapping[command]) 
    {
        case 1:
            PrintBusCommand (tansport_catalogue, command, description, output);
            break;
        case 2:
            PrintStopCommand (tansport_catalogue, command, description, output);
            break;
    }

}