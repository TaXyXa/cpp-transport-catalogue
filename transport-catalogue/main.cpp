#include <iostream>
#include <fstream>
#include <string>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"
using namespace std;

int main() {
    ///*
    //std::ifstream input;
    //std::ofstream output;
    //input.open("input.json");
    //output.open("out.txt");
    TransportCatalogue catalogue;
    renderer::MapRenderer map_renderer;
    RequestHandler request_handler(catalogue, map_renderer);
    json::Reader json_reader(request_handler);
    json_reader.SetInfo(cin);
    json_reader.PrintInfo(cout);
    //svg::Document doc;
    //request_handler.RenderMap(doc);
    //doc.Render(output);
}