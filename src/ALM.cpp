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
#include "Resolve.h"

template<typename... Args>
void simple_wrapper_println(std::format_string<Args...> fmt, Args&&... args) {
    std::print("[LOG] "); // Print the prefix
    std::println(fmt, std::forward<Args>(args)...); // Print the rest and a newline
}

void process_command_line_args(CommandLineOpts *opts, std::span<char*, std::dynamic_extent> args) {
    for (char * &ptr : args) {
        std::string_view arg(ptr);

        if (arg.find("-") == 0) {
            if (arg == "-ast-dump") {
                opts->dump_ast = true;
            }
        } else {
            std::string filename(arg.begin(), arg.end());
            opts->filenames.push_back(filename);
        }
    }
}

void atomizer_init();

void parser_load_file(Parser *parser, std::string filename) {
    std::ifstream file(filename, std::ios::binary);

    if (file) {
        SourceFile *source = new SourceFile();
        fs::path path(filename);
        fs::path absolute_path = fs::absolute(path);

        std::string f = path.filename().string();
        std::string pp = path.filename().string();
        std::string ap = absolute_path.string();

        source->filename = make_string((u8 *)f.c_str(), f.length() + 1);
        source->path = make_string((u8 *)pp.c_str(), pp.length() + 1);
        source->absolute_path = make_string((u8 *)ap.c_str(), ap.length() + 1);
        std::string text = std::string(std::istreambuf_iterator<char>(file), {});
        String content = make_string(text.c_str(), text.length() + 1);
        source->content = content;
        add_source(parser, source);
    } else {
        std::println("Error reading file '{}'", filename);
    }
}

int main(int argc, char **argv) {
    // simple_wrapper_println("\x1b[31mHello, Red!\x1b[0m");

    CommandLineOpts opts;

    process_command_line_args(&opts, std::span(argv + 1, argc - 1));

    atomizer_init();

    Parser *parser = new Parser();
    parser_load_file(parser, "preload.alm");

    for (std::string filename : opts.filenames) {
        parser_load_file(parser, filename);
    }

    Array<AstFile*> ast_files;
    for (SourceFile *file : parser->files) {
        AstFile *ast_file = parse_file(parser, file);
        ast_files.add(ast_file);
    }

    for (AstFile *file : ast_files) {
        if (opts.dump_ast) {
            ast_print(file);
        }
    }

    Resolver *resolver = new Resolver();
    resolver->files = ast_files;
    resolve_program(resolver, parser);

    return 0;
}
