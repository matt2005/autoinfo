# Contributing to Crankshaft Reborn

Thank you for your interest in contributing to Crankshaft Reborn!

## Code of Conduct

By participating in this project, you agree to abide by our Code of Conduct. Please be respectful and constructive in all interactions.

## How to Contribute

### Reporting Bugs

Create an issue on GitHub with:
- Clear title and description
- Steps to reproduce
- Expected vs actual behaviour
- System information
- Relevant logs

### Suggesting Features

Create an issue with:
- Clear description of the feature
- Use case and benefits
- Possible implementation approach

### Pull Requests

1. Fork the repository
2. Create a branch: `git checkout -b feature/your-feature-name`
3. Make your changes following coding standards
4. Write tests for new functionality
5. Update documentation
6. Commit: `git commit -m "Add: Brief description"`
7. Push: `git push origin feature/your-feature-name`
8. Create Pull Request on GitHub

## Coding Standards

### C++ Guidelines

Follow the Google C++ Style Guide:

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Naming Conventions**:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: `snake_case_` (with trailing underscore)

- **Header Guards**: Use `#pragma once`
- **Comments**: Use `//` for single-line, `/* */` for multi-line

### File Headers

Include the license header in all new files:

```cpp
/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */
```

## Commit Guidelines

Follow these commit message conventions:
- `Add:` for new features
- `Fix:` for bug fixes
- `Update:` for updates to existing features
- `Refactor:` for code refactoring
- `Docs:` for documentation changes
- `Test:` for test changes

## Testing

Write unit tests for new functionality and ensure all tests pass before submitting.

## Documentation

Update relevant documentation when making changes:
- README.md for user-facing changes
- API documentation for API changes
- Inline comments for complex logic

## Development Environment

### Prerequisites

- Qt6 development packages
- CMake 3.16+
- C++17 compiler
- Git
- (Windows) WSL + VS Code Remote extension (tasks run under WSL)
- Optional analysis tools: `clang-format clang-tidy cppcheck cmake-lint doxygen bandit`

### Build for Development

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j$(nproc)
ctest --output-on-failure
```

### VS Code Tasks (WSL)

The repository ships a shared `.vscode/tasks.json` (tracked despite other VS Code files being ignored). Core tasks:

| Task | Purpose |
|------|---------|
| Configure CMake (Debug/Release) | Generate build tree with tests enabled |
| Build (Debug/Release) | Compile targets |
| Run Tests | Execute test suite via `ctest` |
| Check Formatting (clang-format) | Enforce Google style with CI parity |
| Lint C++ Code (clang-tidy) | Static analysis (config in `.clang-tidy`) |
| Check C++ Code (cppcheck) | Additional static analysis surface |
| Check CMake Files | Lint key CMakeLists |
| Build Package (DEB) | Run `cpack -G DEB` packaging |
| Run Application (VNC Debug) | Launch Qt app on virtual display |
| Check License Headers | Ensure GPL header presence |
| Pre-commit Check | Sequenced validation suite |

Install tooling quickly using the `Install Dev Tools (WSL)` task.

### Quality & Style Tooling

- Formatting: `clang-format` (Google style, column 100). Run formatting before committing.
- Static Analysis: `clang-tidy` (configured via root `.clang-tidy`), and `cppcheck` with broad enable flags.
- Configuration: `.editorconfig` enforces line endings and indentation to keep diffs clean.
- License Compliance: All new source/QML/script files must include the GPL header.
- Fix Summaries: For significant changes/fixes add a markdown file under `docs/fix_summaries/` (lowercase underscores) describing context, change and impact.

### Recommended Workflow

1. Run `Install Dev Tools (WSL)` once per environment.
2. Implement changes with small, focused commits.
3. Run `Pre-commit Check` task; resolve any failures.
4. Add/update tests and documentation.
5. Create or update a fix summary if the change is notable.
6. Push and open a Pull Request referencing related issues.

### Pull Request Checklist

- [ ] Builds succeed (Debug & Release)
- [ ] Tests added/updated and passing
- [ ] Formatting passes (`Check Formatting` task)
- [ ] Static analysis warnings addressed or justified
- [ ] License headers present in new files
- [ ] Documentation/fix summary updated if needed

## Community

- GitHub Discussions for questions
- Discord for real-time chat

Thank you for contributing to Crankshaft Reborn!
