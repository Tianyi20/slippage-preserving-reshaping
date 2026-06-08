#!/usr/bin/env python3
import argparse
from pathlib import Path

import slippage_reshaping as sr


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", required=True, help="Input OBJ")
    parser.add_argument("-e", "--edit", required=True, help="Edit label from companion .deform file")
    parser.add_argument("-o", "--output", default="", help="Optional output folder")
    parser.add_argument("--max-iters", type=int, default=100)
    parser.add_argument("--disable-error-distribution", action="store_true")
    args = parser.parse_args()

    mesh_path = Path(args.input)
    run_name = f"{mesh_path.stem}_{args.edit}_py"

    debug_folder = ""
    if args.output:
        debug_folder = str(Path(args.output) / "debug")

    options = sr.Options(
        max_iters=args.max_iters,
        handle_error_distrib_enabled=not args.disable_error_distribution,
        debug_folder=debug_folder,
        input_name=run_name,
    )

    result = sr.optimize_from_edit_file(
        str(mesh_path),
        args.edit,
        options,
        normalize_mesh=True,
        load_straightness=True,
    )

    print("Optimization finished")
    print("vertices:", result.vertices.shape)
    print("faces:", result.faces.shape)
    print("iterations:", result.iterations)
    print("best_solution_iteration:", result.best_solution_iteration)
    print("total_time:", result.total_time)

    if args.output:
        ok = sr.save_obj(result.vertices, result.faces, args.output, run_name, "output")
        print("saved:", ok, "folder:", args.output)


if __name__ == "__main__":
    main()
