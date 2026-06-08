#include "ReshapingOptimizer.h"

#include <mesh_reshaping/data_filenames.h>
#include <mesh_reshaping/face_principal_curvatures_io.h>
#include <mesh_reshaping/precompute_reshaping_data.h>
#include <mesh_reshaping/reshaping_data.h>
#include <mesh_reshaping/reshaping_tool.h>

#include <ca_essentials/meshes/load_trimesh.h>

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace reshaping_binding {
namespace {

std::unique_ptr<reshaping::TriMesh>
load_mesh_or_throw(const std::string& mesh_filename, bool normalize_mesh) {
    auto mesh = ca_essentials::meshes::load_trimesh(mesh_filename, normalize_mesh);
    if(!mesh) {
        throw std::runtime_error("Could not load mesh: " + mesh_filename);
    }
    return mesh;
}

void load_curvatures_or_throw(const std::string& mesh_filename,
                              Eigen::VectorXd& face_k1,
                              Eigen::VectorXd& face_k2) {
    const std::string curvature_filename = reshaping::get_curvature_fn(mesh_filename);
    if(!std::filesystem::exists(curvature_filename)) {
        throw std::runtime_error("Could not find curvature file: " + curvature_filename);
    }

    const bool ok = reshaping::load_face_principal_curvature_values(
        curvature_filename,
        face_k1,
        face_k2
    );
    if(!ok) {
        throw std::runtime_error("Could not load curvature file: " + curvature_filename);
    }
}

std::unique_ptr<reshaping::StraightChains>
load_straightness_or_null(const std::string& mesh_filename) {
    const std::string straightness_filename = reshaping::get_straightness_fn(mesh_filename);
    if(!std::filesystem::exists(straightness_filename)) {
        return nullptr;
    }

    auto straight_chains = std::make_unique<reshaping::StraightChains>();
    if(!straight_chains->load_from_file(straightness_filename)) {
        throw std::runtime_error("Could not load straightness file: " + straightness_filename);
    }
    return straight_chains;
}

} // namespace

ReshapingOptimizer::ReshapingOptimizer(const Eigen::MatrixXd& vertices,
                                       const Eigen::MatrixXi& faces,
                                       const Eigen::VectorXd& face_k1,
                                       const Eigen::VectorXd& face_k2,
                                       const reshaping::StraightChains* straight_chains)
    : mesh_(std::make_unique<reshaping::TriMesh>(vertices, faces)),
      face_k1_(face_k1),
      face_k2_(face_k2),
      straight_chains_(straight_chains) {
    validate_mesh_arrays(vertices, faces);
    validate_curvatures();
}

ReshapingOptimizer::ReshapingOptimizer(const reshaping::TriMesh& mesh,
                                       const Eigen::VectorXd& face_k1,
                                       const Eigen::VectorXd& face_k2,
                                       const reshaping::StraightChains* straight_chains)
    : mesh_(std::make_unique<reshaping::TriMesh>(mesh.get_vertices(), mesh.get_facets())),
      face_k1_(face_k1),
      face_k2_(face_k2),
      straight_chains_(straight_chains) {
    validate_curvatures();
}

ReshapingOptimizer
ReshapingOptimizer::from_files(const std::string& mesh_filename,
                               bool normalize_mesh,
                               bool load_straightness) {
    auto mesh = load_mesh_or_throw(mesh_filename, normalize_mesh);

    Eigen::VectorXd face_k1;
    Eigen::VectorXd face_k2;
    load_curvatures_or_throw(mesh_filename, face_k1, face_k2);

    ReshapingOptimizer optimizer(*mesh, face_k1, face_k2, nullptr);

    if(load_straightness) {
        optimizer.owned_straight_chains_ = load_straightness_or_null(mesh_filename);
        optimizer.straight_chains_ = optimizer.owned_straight_chains_.get();
    }

    return optimizer;
}

Eigen::MatrixXd ReshapingOptimizer::vertices() const {
    return mesh_->get_vertices();
}

Eigen::MatrixXi ReshapingOptimizer::faces() const {
    return mesh_->get_facets();
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize(const ConstraintMap& constraints) const {
    return optimize(constraints, Options{});
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize(const ConstraintMap& constraints,
                             const Options& options) const {
    validate_constraints(constraints);

    reshaping::ReshapingParams params = make_params(options);

    // Use the same proven sequence as the original reshaping_demo:
    // 1. precompute reshaping data
    // 2. insert hard constraints into reshaping_data->bc
    // 3. solve
    auto reshaping_data = reshaping::precompute_reshaping_data(
        params,
        *mesh_,
        face_k1_,
        face_k2_,
        straight_chains_
    );

    for(const auto& [vertex_id, target_position] : constraints) {
        reshaping_data->bc.insert({vertex_id, target_position});
    }

    Eigen::MatrixXd optimized_vertices = reshaping::reshaping_solve(params, *reshaping_data);

    Result result;
    result.vertices = std::move(optimized_vertices);
    result.faces = mesh_->get_facets();
    result.iterations = reshaping_data->iter;
    result.best_solution_iteration = reshaping_data->best_sol_iter;
    result.total_time = reshaping_data->total_time;
    result.average_iteration_time = reshaping_data->avg_iter_time;
    return result;
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize(const std::vector<int>& constraint_vertex_ids,
                             const Eigen::MatrixXd& constraint_positions) const {
    return optimize(constraint_vertex_ids, constraint_positions, Options{});
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize(const std::vector<int>& constraint_vertex_ids,
                             const Eigen::MatrixXd& constraint_positions,
                             const Options& options) const {
    if(static_cast<int>(constraint_vertex_ids.size()) != constraint_positions.rows()) {
        throw std::invalid_argument(
            "constraint_vertex_ids.size() must equal constraint_positions.rows()"
        );
    }
    if(constraint_positions.cols() != 3) {
        throw std::invalid_argument("constraint_positions must have shape N x 3");
    }

    ConstraintMap constraints;
    constraints.reserve(constraint_vertex_ids.size());
    for(int i = 0; i < static_cast<int>(constraint_vertex_ids.size()); ++i) {
        constraints.emplace(
            constraint_vertex_ids[i],
            constraint_positions.row(i).transpose()
        );
    }

    return optimize(constraints, options);
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize_with_edit_operation(const reshaping::EditOperation& edit_op) const {
    return optimize_with_edit_operation(edit_op, Options{});
}

ReshapingOptimizer::Result
ReshapingOptimizer::optimize_with_edit_operation(const reshaping::EditOperation& edit_op,
                                                 const Options& options) const {
    ConstraintMap constraints;
    constraints.reserve(edit_op.displacements.size());

    const double diagonal_length = mesh_->get_bbox().diagonal().norm();
    const Eigen::MatrixXd vertices = mesh_->get_vertices();

    for(const auto& [vertex_id, displacement] : edit_op.displacements) {
        const Eigen::Vector3d original_position = vertices.row(vertex_id).transpose();
        const Eigen::Vector3d target_position = reshaping::displacement_to_abs_position(
            original_position,
            displacement,
            diagonal_length
        );
        constraints.emplace(vertex_id, target_position);
    }

    return optimize(constraints, options);
}

reshaping::ReshapingParams
ReshapingOptimizer::make_params(const Options& options) {
    reshaping::ReshapingParams params;
    params.max_iters = options.max_iters;
    params.handle_error_distrib_enabled = options.handle_error_distrib_enabled;
    params.input_name = options.input_name;

    if(!options.debug_folder.empty()) {
        params.debug_folder = options.debug_folder;
    }

    return params;
}

void ReshapingOptimizer::validate_mesh_arrays(const Eigen::MatrixXd& vertices,
                                              const Eigen::MatrixXi& faces) {
    if(vertices.cols() != 3) {
        throw std::invalid_argument("vertices must have shape #V x 3");
    }
    if(faces.cols() != 3) {
        throw std::invalid_argument("faces must have shape #F x 3");
    }
    if(vertices.rows() == 0 || faces.rows() == 0) {
        throw std::invalid_argument("vertices and faces must be non-empty");
    }

    for(int f = 0; f < faces.rows(); ++f) {
        for(int c = 0; c < 3; ++c) {
            const int vertex_id = faces(f, c);
            if(vertex_id < 0 || vertex_id >= vertices.rows()) {
                throw std::out_of_range("faces contain an invalid vertex index");
            }
        }
    }
}

void ReshapingOptimizer::validate_curvatures() const {
    const int num_faces = static_cast<int>(mesh_->get_facets().rows());
    if(face_k1_.size() != num_faces || face_k2_.size() != num_faces) {
        throw std::invalid_argument(
            "face_k1 and face_k2 must each have one value per mesh face"
        );
    }
}

void ReshapingOptimizer::validate_constraints(const ConstraintMap& constraints) const {
    if(constraints.empty()) {
        throw std::invalid_argument("at least one constrained vertex is required");
    }

    const int num_vertices = static_cast<int>(mesh_->get_vertices().rows());
    for(const auto& [vertex_id, target_position] : constraints) {
        if(vertex_id < 0 || vertex_id >= num_vertices) {
            throw std::out_of_range("constraint vertex id is outside the mesh vertex range");
        }
        if(!target_position.allFinite()) {
            throw std::invalid_argument("constraint target positions must be finite");
        }
    }
}

} // namespace reshaping_binding
