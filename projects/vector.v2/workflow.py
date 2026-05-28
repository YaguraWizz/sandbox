def dependencies():
    return {"os": ["linux", "macos", "windows"]}


def build():
    return [
        "cmake -S . -B build",
        "cmake --build build"
    ]


def tests():
    return [
        "cmake -S . -B build -DBUILD_TEST=ON",
        "cmake --build build",
        "ctest --test-dir build --output-on-failure"
    ]