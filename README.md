# sample_cpp_project

A minimal C++ sample project demonstrating a simple build/run workflow and a Dockerized runtime.

**Status:** Minimal example — useful as a template for small C++ utilities and learning how to containerize C++ apps.

## Contents

- `main.cpp` — example C++ program (entry point).
- `Dockerfile` — container image definition to build and run the binary.
- `index.html` — optional static file included in the repo.
- `README.md` — this file.

## Goals

- Provide a tiny, reproducible C++ project that compiles with common toolchains.
- Show how to build locally and inside a Docker container.

## Prerequisites

- C++ compiler supporting C++17 (e.g. `g++`, `clang++`, or MSVC).
- `make` (optional, not required for the minimal example).
- Docker (optional) to build and run the image defined in `Dockerfile`.

## Build (Local)

Build with GNU/Clang on Windows (via WSL) or Linux/macOS:

```
g++ -std=c++17 main.cpp -O2 -o sample_cpp_project
```

On Windows with MSVC (Developer Command Prompt):

```
cl /std:c++17 /O2 /EHsc main.cpp /Fe:sample_cpp_project.exe
```

## Run (Local)

After building, run the produced binary:

```
./sample_cpp_project        # Unix-like shells
sample_cpp_project.exe      # Windows Command Prompt / PowerShell
```

## Docker (Build & Run)

Build the Docker image (from repository root):

```
docker build -t sample_cpp_project .
```

Run the image (container will run the sample binary):

```
docker run --rm sample_cpp_project
```

Notes:
- The `Dockerfile` uses a lightweight base and compiles or copies the binary into the container depending on how it is authored. Inspect the `Dockerfile` if you need to change build options or base image.

## Project Structure

- `main.cpp` — simple example C++ source file. Replace or extend this file for your project logic.
- `Dockerfile` — contains instructions to containerize the binary.
- `index.html` — placeholder/static file not required for the C++ build.

## Testing

This project does not include an automated test suite. To add tests, consider adding a test framework (e.g. GoogleTest) and a small `CMakeLists.txt` or `Makefile` to drive compilation of tests.

## Contributing

Contributions are welcome. For simple improvements:

1. Fork the repository.
2. Create a branch for your change.
3. Open a pull request describing your change.

## License

Specify a license for the project here (e.g. MIT). If you don't want to add a license file now, add a short license statement here.

---

If you'd like, I can also:
- add a `CMakeLists.txt` or `Makefile` for cross-platform builds,
- add CI configuration to build the binary automatically, or
- add a minimal test harness using GoogleTest.

Tell me which of the above you'd like next.