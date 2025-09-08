#pragma once
#include <tree_sitter/api.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <sstream>
#include <stack>
#include <algorithm>
#include <cstdlib>

extern "C" const TSLanguage* tree_sitter_cpp();

class Parser {
public:
    std::string read_file(const std::string& path);

    std::string get_node_text(const TSNode& node, const std::string& source);

    std::string get_base_name(const TSNode& node, const std::string& source);

    std::string get_fully_qualified_name(const TSNode& def_node, const std::string& source);

    void collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions);

    void find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls);

    void process_file(const std::string& path, std::unordered_map<std::string, std::vector<std::string>>& call_graph);

    void traverse_directory(const std::filesystem::path& dir, std::unordered_map<std::string, std::vector<std::string>>& call_graph);

    void print_call_graph(const std::unordered_map<std::string, std::vector<std::string>>& call_graph);
};
