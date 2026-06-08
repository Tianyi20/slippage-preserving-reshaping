#pragma once

#include <Eigen/Core>

#include <mesh_reshaping/globals.h>
#include <mesh_reshaping/reshaping_params.h>
#include <mesh_reshaping/straight_chains.h>
#include <mesh_reshaping/edit_operation.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace reshaping_binding {

/**
 * Thin C++ wrapper around mesh_reshaping_lib.
 *
 * Intended public API for the future Python binding:
 *   input  : mesh vertices/faces, per-face curvature values, constraint vertices
 *   output : optimized vertex positions, faces unchanged
 *
 * The existing demo reads constraints from a .deform edit operation. This class
 * keeps that path as a helper, but the main solve API accepts absolute target
 * positions directly, which is the most natural representation from Python.
 */
class ReshapingOptimizer {
public:
    using ConstraintMap = std::unordered_map<int, Eigen::Vector3d>;

    struct Options {
        int max_iters = 100;
        bool handle_error_distrib_enabled = true;
        std::string debug_folder;
        std::string input_name = "reshaping_binding";
    };

    struct Result {
        Eigen::MatrixXd vertices;   // #V x 3 optimized vertex positions
        Eigen::MatrixXi faces;      // #F x 3 unchanged triangle indices
        int iterations = 0;
        int best_solution_iteration = -1;
        double total_time = 0.0;
        double average_iteration_time = 0.0;
    };

    /** Construct from in-memory mesh arrays. */
    ReshapingOptimizer(const Eigen::MatrixXd& vertices,
                       const Eigen::MatrixXi& faces,
                       const Eigen::VectorXd& face_k1,
                       const Eigen::VectorXd& face_k2,
                       const reshaping::StraightChains* straight_chains = nullptr);

    /**
     * Construct from an already-created TriMesh.
     *
     * The mesh data is copied by extracting vertices/faces instead of relying
     * on TriMesh being copy-constructible.
     */
    ReshapingOptimizer(const reshaping::TriMesh& mesh,
                       const Eigen::VectorXd& face_k1,
                       const Eigen::VectorXd& face_k2,
                       const reshaping::StraightChains* straight_chains = nullptr);

    /**
     * Compatibility constructor for the current demo workflow. It loads the OBJ
     * and the companion .fk curvature file using the same filename conventions
     * as reshaping_demo.
     */
    static ReshapingOptimizer from_files(const std::string& mesh_filename,
                                         bool normalize_mesh = true,
                                         bool load_straightness = true);

    Eigen::MatrixXd vertices() const;
    Eigen::MatrixXi faces() const;
    const Eigen::VectorXd& face_k1() const { return face_k1_; }
    const Eigen::VectorXd& face_k2() const { return face_k2_; }

    /**
     * Main API: constraints are absolute target positions for vertex ids.
     */
    Result optimize(const ConstraintMap& constraints) const;
    Result optimize(const ConstraintMap& constraints,
                    const Options& options) const;

    /**
     * Pybind-friendly overload:
     *   constraint_vertex_ids: length N
     *   constraint_positions : N x 3 absolute target positions
     */
    Result optimize(const std::vector<int>& constraint_vertex_ids,
                    const Eigen::MatrixXd& constraint_positions) const;
    Result optimize(const std::vector<int>& constraint_vertex_ids,
                    const Eigen::MatrixXd& constraint_positions,
                    const Options& options) const;

    /**
     * Compatibility helper for old .deform edit operations. The stored demo
     * displacements are converted to absolute target positions using the mesh
     * bounding-box diagonal, exactly like reshaping_demo does.
     */
    Result optimize_with_edit_operation(const reshaping::EditOperation& edit_op) const;
    Result optimize_with_edit_operation(const reshaping::EditOperation& edit_op,
                                        const Options& options) const;

private:
    static reshaping::ReshapingParams make_params(const Options& options);
    static void validate_mesh_arrays(const Eigen::MatrixXd& vertices,
                                     const Eigen::MatrixXi& faces);
    void validate_curvatures() const;
    void validate_constraints(const ConstraintMap& constraints) const;

    std::unique_ptr<reshaping::TriMesh> mesh_;
    Eigen::VectorXd face_k1_;
    Eigen::VectorXd face_k2_;

    // Owned when from_files(load_straightness=true) is used. Otherwise nullptr.
    std::unique_ptr<reshaping::StraightChains> owned_straight_chains_;

    // Non-owning pointer used by precompute_reshaping_data.
    const reshaping::StraightChains* straight_chains_ = nullptr;
};

} // namespace reshaping_binding
