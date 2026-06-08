import numpy as np
import slippage_reshaping as sr


def make_cube():
    # 8 vertices of a unit cube
    V = np.array(
        [
            [-0.5, -0.5, -0.5],  # 0 bottom
            [ 0.5, -0.5, -0.5],  # 1
            [ 0.5,  0.5, -0.5],  # 2
            [-0.5,  0.5, -0.5],  # 3

            [-0.5, -0.5,  0.5],  # 4 top
            [ 0.5, -0.5,  0.5],  # 5
            [ 0.5,  0.5,  0.5],  # 6
            [-0.5,  0.5,  0.5],  # 7
        ],
        dtype=np.float64,
    )

    # 12 triangles, two per cube face
    F = np.array(
        [
            [0, 1, 2], [0, 2, 3],  # bottom
            [4, 6, 5], [4, 7, 6],  # top

            [0, 4, 5], [0, 5, 1],  # front
            [1, 5, 6], [1, 6, 2],  # right
            [2, 6, 7], [2, 7, 3],  # back
            [3, 7, 4], [3, 4, 0],  # left
        ],
        dtype=np.int32,
    )

    return V, F


def write_obj(path, V, F):
    with open(path, "w") as f:
        for v in V:
            f.write(f"v {v[0]} {v[1]} {v[2]}\n")

        # OBJ face indices are 1-based
        for tri in F:
            f.write(f"f {tri[0] + 1} {tri[1] + 1} {tri[2] + 1}\n")


def main():
    V, F = make_cube()

    # For this artificial test, just set per-face curvatures to zero.
    # Real use should pass your actual face curvature values.
    face_k1 = np.zeros(F.shape[0], dtype=np.float64)
    face_k2 = np.zeros(F.shape[0], dtype=np.float64)

    # Fix bottom vertices and move top vertices upward.
    constraint_ids = [0, 1, 2, 3, 4, 5, 6, 7]

    target_positions = V[constraint_ids].copy()
    target_positions[0:4, :] = V[0:4, :]          # bottom fixed
    target_positions[4:8, :] = V[4:8, :]          # top original
    target_positions[4:8, 2] += 0.35              # move top upward

    V_opt = sr.optimize_mesh(
        V,
        F,
        face_k1,
        face_k2,
        constraint_ids,
        target_positions,
        max_iters=20,
        handle_error_distrib_enabled=False,
        input_name="cube_test",
    )

    print("Input vertices:")
    print(V)

    print("\nOptimized vertices:")
    print(V_opt)

    write_obj("cube_input.obj", V, F)
    write_obj("cube_optimized.obj", V_opt, F)

    print("\nWrote:")
    print("  cube_input.obj")
    print("  cube_optimized.obj")


if __name__ == "__main__":
    main()