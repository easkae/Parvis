#include "parser.h"
#include "generator.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_directory> <language>" << std::endl;
        std::cerr << "Supported languages: C++, Python" << std::endl;
        return 1;
    }

    std::string path = argv[1];
    std::string language = argv[2];

    try {
        Parser code_parser(language);
        Generator dot_generator;

        std::unordered_map<std::string, std::vector<std::string>> call_graph;
        code_parser.traverse_directory(path, call_graph);
        code_parser.print_call_graph(call_graph);

        std::string dot_filename = "call_graph.dot";
        std::string output_filename = "call_graph.png";
        dot_generator.generate_dot_file(call_graph, dot_filename);
        dot_generator.render_graph(dot_filename, output_filename);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
