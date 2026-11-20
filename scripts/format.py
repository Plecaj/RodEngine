import argparse
import os
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path


SCRIPTS = Path(__file__).resolve().parent
ROOT = SCRIPTS.parent
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".inl", ".ipp", ".tpp", ".cppm", ".ixx"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Format the repository's C/C++ files.")
    parser.add_argument("--check", action="store_true", help="Check formatting without modifying files.")
    if Path.cwd().resolve() not in (ROOT, SCRIPTS):
        parser.error("Run this script from the repository root or its scripts/ directory.")
    args = parser.parse_args()
    action = "Needs formatting" if args.check else "Formatting"

    files = subprocess.check_output(
        ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", ".", ":(exclude)RodEngine/vendor"],
        cwd=ROOT,
    )
    exit_code = 0
    for filename in os.fsdecode(files).split("\0"):
        path = ROOT / filename
        if path.suffix.lower() not in EXTENSIONS or not path.is_file():
            continue

        file_args = ["--style=file", "--", filename]
        replacements = subprocess.check_output(["clang-format", "--output-replacements-xml", *file_args], cwd=ROOT)
        if not replacements or ET.fromstring(replacements).find("replacement") is None:
            continue

        print(f"{action} {filename}", flush=True)
        if args.check:
            exit_code = 1
        else:
            subprocess.run(["clang-format", "-i", *file_args], cwd=ROOT, check=True)
    raise SystemExit(exit_code)
