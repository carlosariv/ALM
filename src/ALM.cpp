#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <filesystem>
#include <print>

namespace fs = std::filesystem;

#include "Array.h"
#include "ALM.h"
#include "Parser.h"

template<typename... Args>
void simple_wrapper_println(std::format_string<Args...> fmt, Args&&... args) {
    std::print("[LOG] "); // Print the prefix
    std::println(fmt, std::forward<Args>(args)...); // Print the rest and a newline
}

void CommandLineOpts::process_command_line_args(std::span<char*, std::dynamic_extent> args) {
    for (char * &ptr : args) {
        std::string_view arg(ptr);

        if (arg.find("-") == 0) {
        } else {
            std::string filename(arg.begin(), arg.end());
            filenames.push_back(filename);
        }
    }
}

void init_global_parser();
void atomizer_init();

int main(int argc, char **argv) {
    simple_wrapper_println("\x1b[31mHello, Red!\x1b[0m");

    CommandLineOpts opts;
    opts.process_command_line_args(std::span(argv + 1, argc - 1));

    atomizer_init();
    init_global_parser();

    std::string foo = "one two";

    String foo_str = make_string(foo.c_str(), foo.length());

    std::println("{}", foo_str);

    Parser parser;
    for (std::string filename : opts.filenames) {
        std::ifstream file(filename, std::ios::binary);

        if (file) {
            SourceFile *source = new SourceFile();
            fs::path path(filename);
            std::string f = path.filename().string();
            std::string pp = path.filename().string();

            source->filename = make_string((u8 *)f.c_str(), f.length() + 1);
            source->path = make_string((u8 *)pp.c_str(), pp.length() + 1);
            std::string text = std::string(std::istreambuf_iterator<char>(file), {});
            String content = make_string(text.c_str(), text.length() + 1);
            source->content = content;
            add_source(&parser, source);
        } else {
            std::println("Error reading file '{}'", filename);
        }
    }

    parse(&parser);

    return 0;
}
