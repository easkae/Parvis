#include "generator.h"

void
Generator::generate_dot_file(const std::unordered_map<std::string, std::vector<std::string>>& call_graph, const std::string& dot_filename) {
    std::ofstream dot_file(dot_filename);
    if (!dot_file) {
        std::cerr << "Error opening " << dot_filename << std::endl;
        return;
    }

    dot_file << "digraph CallGraph {" << std::endl;
    dot_file << "    node [shape=box, style=filled, fillcolor=lightblue];" << std::endl;

    auto escape_dot_label = [](const std::string& label) -> std::string {
        std::string escaped = label;
        std::replace(escaped.begin(), escaped.end(), ':', '_');
        std::replace(escaped.begin(), escaped.end(), '<', '_');
        std::replace(escaped.begin(), escaped.end(), '>', '_');
        std::replace(escaped.begin(), escaped.end(), ' ', '_');
        return escaped;
    };

    for (const auto& [func, calls] : call_graph) {
        std::string from = escape_dot_label(func);
        for (const auto& call : calls) {
            std::string to = escape_dot_label(call);
            dot_file << "    \"" << from << "\" -> \"" << to << "\";" << std::endl;
        }
    }

    dot_file << "}" << std::endl;
    dot_file.close();
    std::cout << "Generated DOT file: " << dot_filename << std::endl;
}

void
Generator::render_graph(const std::string& dot_filename, const std::string& output_filename) {
    std::string command = "dot -Tpng " + dot_filename + " -o " + output_filename;
    int result = std::system(command.c_str());
    if (result == 0) {
        std::cout << "Rendered graph to: " << output_filename << std::endl;
    } else {
        std::cerr << "Error rendering graph. Make sure GraphViz is installed and 'dot' is in your PATH." << std::endl;
    }
}
