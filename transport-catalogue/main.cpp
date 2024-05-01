#include <iostream>
#include <fstream>
#include <string>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"
using namespace std;

int main() {
    ///*
    //std::ifstream input;
    //std::ofstream output;
    //input.open("input.txt");
    //output.open("out.txt");
    TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    TransportRouter router(catalogue);
    RequestHandler request_handler(catalogue, map_renderer, router);
    json::Reader json_reader(request_handler);
    json_reader.SetInfo(cin);
    json_reader.PrintInfo(cout);
    //svg::Document doc;
    //request_handler.RenderMap(doc);
    //doc.Render(output);
}
