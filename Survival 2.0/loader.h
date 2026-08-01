#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

std::vector<int> loadCSV(const std::string& filename, int& width, int& height) {
    std::vector<int> tiles;
    std::ifstream file("levels / l1.csv");
    std::string line;
    width = 0;
    height = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        int rowWidth = 0;
        while (std::getline(ss, token, ',')) {
            tiles.push_back(std::stoi(token));
            ++rowWidth;
        }
        if (width == 0) width = rowWidth;
        ++height;
    }

    return tiles;
}