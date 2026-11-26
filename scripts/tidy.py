import argparse
import json
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


SCRIPTS = Path(__file__).resolve().parent
ROOT = SCRIPTS.parent


if __name__ == "__main__":
    default_jobs = max(1, (os.cpu_count() or 1) * 2 // 3)
    parser = argparse.ArgumentParser(description="Report clang-tidy diagnostics for the project's compiled sources.")
    parser.add_argument(
        "-p", dest="build_dir", type=Path, default=Path("build/analysis"), help="Build directory (default: build/analysis)."
    )
    parser.add_argument(
        "-j", "--jobs", type=int, default=default_jobs, help="Parallel clang-tidy processes (default: %(default)s)."
    )
    if Path.cwd().resolve() not in (ROOT, SCRIPTS):
        parser.error("Run this script from the repository root or its scripts/ directory.")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("-j must be at least 1.")

    build_dir = (ROOT / args.build_dir).resolve()
    clang_tidy = shutil.which("clang-tidy")
    if clang_tidy is None:
        parser.error("clang-tidy is missing from PATH. Use the clang-tidy from your build's LLVM toolchain.")
    cmake = shutil.which("cmake")
    if cmake is None:
        parser.error("cmake is missing from PATH.")

    configure = [cmake, "-S", str(ROOT), "-B", str(build_dir)]
    if not (build_dir / "CMakeCache.txt").is_file():
        preset = {"win32": "windows-clang-analysis", "linux": "linux-clang-analysis"}.get(sys.platform)
        if preset is None:
            parser.error("A fresh analysis directory requires the Windows or Linux analysis preset.")
        configure.extend([
            "--preset", preset,
            "-DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON",
            "-DCMAKE_CXX_SCAN_FOR_MODULES=OFF",
        ])
        for language, environment_variable in (("C", "CC"), ("CXX", "CXX")):
            if compiler := os.environ.get(environment_variable):
                configure.append(f"-DCMAKE_{language}_COMPILER={compiler}")
    configuration = subprocess.run(configure, cwd=ROOT)
    if configuration.returncode:
        raise SystemExit(configuration.returncode)

    database = build_dir / "compile_commands.json"
    if not database.is_file():
        parser.error(f"CMake did not generate {database}. Use an analysis directory with the Ninja generator.")
    commands = json.loads(database.read_bytes())
    compiled = {(build_dir / command["directory"] / command["file"]).resolve() for command in commands}
    files = subprocess.check_output(
        ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", ".", ":(exclude)RodEngine/vendor"],
        cwd=ROOT,
    )
    sources = set()
    for name in os.fsdecode(files).split("\0"):
        path = ROOT / name
        if path.resolve() in compiled and path.is_file():
            sources.add(name)
    if not sources:
        parser.error(f"No project sources found in {database}.")

    ordered_sources = sorted(sources)
    workers = min(args.jobs, len(ordered_sources))
    command = [clang_tidy, "--quiet", "-p", str(build_dir)]
    exit_code = 0
    with ThreadPoolExecutor(max_workers=workers) as executor:
        analyses = [
            executor.submit(
                subprocess.run,
                [*command, *ordered_sources[index::workers]],
                cwd=ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                encoding="utf-8",
                errors="replace",
            )
            for index in range(workers)
        ]
        for analysis in as_completed(analyses):
            result = analysis.result()
            print(result.stdout, end="", flush=True)
            if result.returncode:
                exit_code = 1
    raise SystemExit(exit_code)
