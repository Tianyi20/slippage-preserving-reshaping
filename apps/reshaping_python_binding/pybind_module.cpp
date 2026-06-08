#include "ReshapingOptimizer.h"

#include <mesh_reshaping/data_filenames.h>
#include <mesh_reshaping/edit_operation.h>
#include <mesh_reshaping/reshaping_tool_io.h>

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace py = pybind11;

namespace {

using Optimizer = reshaping_binding::ReshapingOptimizer;
using Options = reshaping_binding::ReshapingOptimizer::Options;
using Result = reshaping_binding::ReshapingOptimizer::Result;

Options make_options(int max_iters,
                     bool handle_error_distrib_enabled,
                     const std::string& debug_folder,
                     const std::string& input_name) {
    Options options;
    options.max_iters = max_iters;
    options.handle_error_distrib_enabled = handle_error_distrib_enabled;
    options.debug_folder = debug_folder;
    options.input_name = input_name;
    return options;
}

Result optimize_arrays(const Eigen::MatrixXd& vertices,
                       const Eigen::MatrixXi& faces,
                       const Eigen::VectorXd& face_k1,
                       const Eigen::VectorXd& face_k2,
                       const std::vector<int>& constraint_vertex_ids,
                       const Eigen::MatrixXd& constraint_positions,
                       const Options& options) {
    Optimizer optimizer(vertices, faces, face_k1, face_k2, nullptr);
    py::gil_scoped_release release;
    return optimizer.optimize(constraint_vertex_ids, constraint_positions, options);
}

Result optimize_from_edit_file(const std::string& mesh_filename,
                               const std::string& edit_label,
                               const Options& options,
                               bool normalize_mesh,
                               bool load_straightness) {
    Optimizer optimizer = Optimizer::from_files(mesh_filename, normalize_mesh, load_straightness);

    const std::string edit_operation_filename = reshaping::get_edit_operation_fn(mesh_filename);
    auto load_result = reshaping::load_edit_operation_from_json(edit_operation_filename, edit_label);

    if(!load_result.first) {
        throw std::runtime_error(
            "Could not load edit operation '" + edit_label +
            "' from '" + edit_operation_filename + "'"
        );
    }

    py::gil_scoped_release release;
    return optimizer.optimize_with_edit_operation(load_result.second, options);
}

bool save_obj(const Eigen::MatrixXd& vertices,
              const Eigen::MatrixXi& faces,
              const std::string& output_dir,
              const std::string& run_name,
              const std::string& suffix) {
    if(vertices.cols() != 3) {
        throw std::invalid_argument("vertices must have shape #V x 3");
    }
    if(faces.cols() != 3) {
        throw std::invalid_argument("faces must have shape #F x 3");
    }

    std::filesystem::create_directories(output_dir);

    reshaping::TriMesh mesh(vertices, faces);
    return reshaping::save_mesh(mesh, output_dir, run_name, suffix);
}

} // namespace

PYBIND11_MODULE(slippage_reshaping_cpp, m) {
    m.doc() = "Python binding for Slippage-Preserving Reshaping";

    py::class_<Options>(m, "Options")
        .def(py::init(&make_options),
             py::arg("max_iters") = 100,
             py::arg("handle_error_distrib_enabled") = true,
             py::arg("debug_folder") = "",
             py::arg("input_name") = "python_binding")
        .def_readwrite("max_iters", &Options::max_iters)
        .def_readwrite("handle_error_distrib_enabled", &Options::handle_error_distrib_enabled)
        .def_readwrite("debug_folder", &Options::debug_folder)
        .def_readwrite("input_name", &Options::input_name);

    py::class_<Result>(m, "Result")
        .def_readonly("vertices", &Result::vertices)
        .def_readonly("faces", &Result::faces)
        .def_readonly("iterations", &Result::iterations)
        .def_readonly("best_solution_iteration", &Result::best_solution_iteration)
        .def_readonly("total_time", &Result::total_time)
        .def_readonly("average_iteration_time", &Result::average_iteration_time)
        .def("as_tuple", [](const Result& r) {
            return py::make_tuple(r.vertices, r.faces);
        });

    py::class_<Optimizer>(m, "ReshapingOptimizer")
        .def(py::init<const Eigen::MatrixXd&,
                      const Eigen::MatrixXi&,
                      const Eigen::VectorXd&,
                      const Eigen::VectorXd&>(),
             py::arg("vertices"),
             py::arg("faces"),
             py::arg("face_k1"),
             py::arg("face_k2"))
        .def_static("from_files",
             &Optimizer::from_files,
             py::arg("mesh_filename"),
             py::arg("normalize_mesh") = true,
             py::arg("load_straightness") = true)
        .def("vertices", &Optimizer::vertices)
        .def("faces", &Optimizer::faces)
        .def("face_k1", &Optimizer::face_k1, py::return_value_policy::reference_internal)
        .def("face_k2", &Optimizer::face_k2, py::return_value_policy::reference_internal)
        .def("optimize",
             [](const Optimizer& optimizer,
                const std::vector<int>& constraint_vertex_ids,
                const Eigen::MatrixXd& constraint_positions,
                const Options& options) {
                 py::gil_scoped_release release;
                 return optimizer.optimize(constraint_vertex_ids, constraint_positions, options);
             },
             py::arg("constraint_vertex_ids"),
             py::arg("constraint_positions"),
             py::arg_v("options", Options{}, "Options()"));

    m.def("optimize",
          &optimize_arrays,
          py::arg("vertices"),
          py::arg("faces"),
          py::arg("face_k1"),
          py::arg("face_k2"),
          py::arg("constraint_vertex_ids"),
          py::arg("constraint_positions"),
          py::arg_v("options", Options{}, "Options()"));

    m.def("optimize_from_edit_file",
          &optimize_from_edit_file,
          py::arg("mesh_filename"),
          py::arg("edit_label"),
          py::arg_v("options", Options{}, "Options()"),
          py::arg("normalize_mesh") = true,
          py::arg("load_straightness") = true);

    m.def("save_obj",
          &save_obj,
          py::arg("vertices"),
          py::arg("faces"),
          py::arg("output_dir"),
          py::arg("run_name"),
          py::arg("suffix") = "output");
}
