#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>

class Generator
{
public:
    void generate_dot_file(const std::unordered_map<std::string, std::vector<std::string>>& call_graph, const std::string& dot_filename);

    void render_graph(const std::string& dot_filename, const std::string& output_filename);
};
