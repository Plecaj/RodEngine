import argparse
import json
import os
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


SCRIPTS = Path(__file__).resolve().parent
ROOT = SCRIPTS.parent


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Report clang-tidy diagnostics for the project's compiled sources.")
    parser.add_argument(
        "-p", dest="build_dir", type=Path, default=Path("build/analysis"), help="Build directory (default: build/analysis)."
    )
    parser.add_argument(
        "-j", "--jobs", type=int, default=os.cpu_count() or 1, help="Parallel clang-tidy processes (default: %(default)s)."
    )
    if Path.cwd().resolve() not in (ROOT, SCRIPTS):
        parser.error("Run this script from the repository root or its scripts/ directory.")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("-j must be at least 1.")

    build_dir = (ROOT / args.build_dir).resolve()
    database = build_dir / "compile_commands.json"
    if not database.is_file():
        parser.error(f"{database} is missing. Configure the windows-clang-analysis or linux-clang-analysis preset first.")
    clang_tidy = shutil.which("clang-tidy")
    if clang_tidy is None:
        parser.error("clang-tidy is missing from PATH. Use the clang-tidy from your build's LLVM toolchain.")

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
