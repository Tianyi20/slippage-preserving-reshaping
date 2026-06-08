/**
 * Minimal command-line demo rewritten to use ReshapingOptimizer.
 *
 * This keeps the same idea as your current command:
 *   ./reshaping_cpp_wrapper_demo -i <input_mesh.obj> -o <output_folder> -e <edit_label>
 *
 * The real future Python binding should call ReshapingOptimizer directly with:
 *   V, F, face_k1, face_k2, constraint_vertex_ids, constraint_positions
 */
#include "ReshapingOptimizer.h"
#include "load_input_data.h"

#include <mesh_reshaping/data_filenames.h>
#include <mesh_reshaping/edit_operation.h>
#include <mesh_reshaping/reshaping_tool_io.h>

#include <CLI/CLI.hpp>

#include <filesystem>
#include <string>

struct CLIArgs {
    std::string input_fn;
    std::string edit_label;
    std::string output_dir;
    std::string temp_dir;
    int max_iters = 100;
    bool handle_error_distrib_on = true;
};

namespace fs = std::filesystem;

void setup_logger() {
    LOGGER.set_level(spdlog::level::level_enum::info);
}

int parse_command_args(int argc, char const* argv[], CLIArgs& args) {
    CLI::App cli_app{argv[0]};
    cli_app.allow_extras(true);
    cli_app.add_option("-i, --input", args.input_fn, "Input mesh filename (.obj)")->required();
    cli_app.add_option("-o, --output", args.output_dir, "Output folder")->required();
    cli_app.add_option("-e, --edit", args.edit_label, "Edit label to be loaded")->required();
    cli_app.add_option("--max-iters", args.max_iters, "Maximum solver iterations");
    cli_app.add_option("--temp-dir", args.temp_dir, "Debug/temp output directory");

    try {
        cli_app.parse(argc, argv);
        return 0;
    } catch(const CLI::ParseError& e) {
        return cli_app.exit(e);
    }
}

void setup_directories(CLIArgs& args) {
    if(!fs::is_directory(args.output_dir)) {
        fs::create_directories(args.output_dir);
    }
    if(args.temp_dir.empty()) {
        args.temp_dir = (fs::path(args.output_dir) / "debug").string();
    }
    fs::create_directories(args.temp_dir);
}

int main(const int argc, const char** argv) {
    setup_logger();

    CLIArgs args;
    if(parse_command_args(argc, argv, args) != 0) {
        return 1;
    }
    setup_directories(args);

    const std::string run_name = fs::path(args.input_fn).stem().string() + "_" + args.edit_label;

    // Keep using the existing loader for this CLI demo because it already loads
    // the mesh, .fk curvature, optional .straight file, and .deform edit op.
    InputData input = load_input_data(args.input_fn, args.edit_label);
    if(!input.mesh || !input.edit_op) {
        return 1;
    }

    reshaping_binding::ReshapingOptimizer optimizer(
        *input.mesh,
        input.PV1,
        input.PV2,
        input.straight_info.get()
    );

    reshaping_binding::ReshapingOptimizer::Options options;
    options.max_iters = args.max_iters;
    options.handle_error_distrib_enabled = args.handle_error_distrib_on;
    options.debug_folder = args.temp_dir;
    options.input_name = run_name;

    auto result = optimizer.optimize_with_edit_operation(*input.edit_op, options);

    reshaping::TriMesh output_mesh(input.mesh->get_vertices(), input.mesh->get_facets());
    Eigen::MatrixXd optimized_vertices = result.vertices;
    output_mesh.import_vertices(optimized_vertices);

    const bool saved = reshaping::save_mesh(output_mesh, args.output_dir, run_name, "output");
    if(!saved) {
        LOGGER.error("Error while saving output mesh to '{}'", args.output_dir);
        return 1;
    }

    LOGGER.info("Optimized mesh written to '{}'", args.output_dir);
    return 0;
}
