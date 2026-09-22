import os

OUTPUT_FILE = "all_code.txt"
EXTENSIONS = (".cpp", ".h", ".hpp", ".cc", ".cxx")

def collect_cpp_files(root="src/"):
    files = []
    for dirpath, _, filenames in os.walk(root):
        for f in filenames:
            if f.endswith(EXTENSIONS):
                files.append(os.path.join(dirpath, f))
    print(files)
    return sorted(files)

def merge_files(files, output):
    with open(output, "w", encoding="utf-8") as out:
        for path in files:
            out.write(f"\n\n// ===== {path} =====\n\n")
            with open(path, "r", encoding="utf-8", errors="ignore") as f:
                out.write(f.read().strip() + "\n")
    print(f"✅ Combined {len(files)} files into {output}")

if __name__ == "__main__":
    files = []
    files += collect_cpp_files("src")
    # files += collect_cpp_files("./src/analyzer/")

    files += collect_cpp_files("tests")
    files.append("CMakeLists.txt")

    merge_files(files, OUTPUT_FILE)

