#include <iostream>
#include <fstream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;

    
    //tests
    /*
    {
        TransportCatalogue catalogue_test;
        vector<string> catalog_string = {
        "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"
        "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"
        "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"
        "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"
        "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam"
        "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya"
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye"
        "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"
        "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"
        "Stop Prazhskaya: 55.611678, 37.603831"
        };
        input::Reader reader_test;
        for (int i = 0; i < catalog_string.size(); ++i) {
            reader_test.ParseLine(catalog_string[i]);
        }
        reader_test.ApplyCommands(catalogue_test);
        
        vector<string> request_string = {
        "Bus 256"
        "Bus 750"
        "Bus 751"
        "Stop Samara"
        "Stop Prazhskaya"
        "Stop Biryulyovo Zapadnoye"};
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