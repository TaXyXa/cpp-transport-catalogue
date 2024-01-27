#include <iostream>
#include <fstream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;

    /*
    //tests
    {
        TransportCatalogue catalogue_test;
        vector<string> catalog_string = {
        "Stop Tolstopaltsevo: 55.611087, 37.208290",
        "Stop Marushkino: 55.595884, 37.209755",
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye",
        "Bus 75055: Tolstopaltsevo - Marushkino - Rasskazovka",
        "Bus 753: Rasskazovka - Biryusinka - Biryulyovo Tovarnaya - Biryulyovo Passazhirskaya",
        "Stop Rasskazovka: 55.632761, 37.333324",
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700",
        "Stop Biryusinka: 55.581065, 37.648390",
        "Stop Universam: 55.587655, 37.645687",
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656",
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"};
        InputReader reader_test;
        for (int i = 0; i < catalog_string.size(); ++i) {
            reader_test.ParseLine(catalog_string[i]);
        }
        reader_test.ApplyCommands(catalogue_test);
        
        vector<string> request_string = {
        "Bus 256",
        "Bus 75055",
        "Bus 753",
        "Bus 751555"};
        for (int i = 0; i < request_string.size(); ++i) {
            ParseAndPrintStat(catalogue_test, request_string[i], cout);
        }
        
    }
    */
    std::ifstream input;
    std::ofstream output;
    input.open("tsC_case1_input.txt");
    output.open("tsC_output.txt");
    
    int base_request_count;
    input >> base_request_count >> ws;

    {
        input::Reader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(input, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    input >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}