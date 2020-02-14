#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <iostream>

struct City {
    City(int _id, int _x, int _y) : id(_id), x(_x), y(_y) {}
    int id;
    int x;
    int y;
};

std::vector<City> readCities(std::istream& lineStream) {
    std::vector<City> result;
    // skip header
    std::string header;
    std::getline(lineStream, header);
    while (std::getline(lineStream, header)) {
        std::stringstream cell;
        cell << header;
        std::string valId = "";
        std::string valX = "";
        std::string valY = "";
        std::getline(cell, valId, ',');
        std::getline(cell, valX, ',');
        std::getline(cell, valY);

        result.emplace_back(std::stoi(valId), std::stoi(valX), std::stoi(valY));
    }
    return result;
}
