# CI Code Quality Debug Guide

This guide helps diagnose and resolve CI code-quality workflow failures.

## Quick Diagnosis

### 1. Check CI Logs
```bash
# View latest workflow run in browser
gh run list --workflow=build.yml --limit 1
gh run view $(gh run list --workflow=build.yml --limit 1 --json databaseId -q '.[0].databaseId')
```

### 2. Run Locally (WSL)
```bash
# Install dev tools first (if not already installed)
./scripts/install_dev_tools.sh

# Run code-quality checks exactly as CI does
./scripts/lint_cpp.sh format-check
./scripts/lint_cpp.sh cppcheck
./scripts/lint_cpp.sh clang-tidy
```

## Common Issues

### Issue: clang-tidy Parse Errors

**Symptom:**
```
error: unknown key 'GlobList'
Error parsing .clang-tidy: Invalid argument
```

**Cause:** `.clang-tidy` contains keys not supported by your clang-tidy version. `GlobList` was added in clang-tidy 19 but Ubuntu/Debian may ship older versions.

**Fix:**
1. Check your clang-tidy version:
   ```bash
   clang-tidy --version
   ```

2. Remove unsupported keys from `.clang-tidy`:
   ```bash
   # Edit .clang-tidy and remove GlobList section
   nano .clang-tidy
   ```

3. Alternative: Use `--checks-filter` command-line option instead of config file exclusions.

**Verification:**
```bash
# Should not error on parse
./scripts/lint_cpp.sh clang-tidy 2>&1 | grep -i error | head -5
```

---

### Issue: cppcheck Reports Too Many Warnings

**Symptom:**
```
style: The function 'X' is never used. [unusedFunction]
```

**Cause:** cppcheck reports unused code, which is normal during development.

**Fix Options:**

1. **Suppress specific warnings** in `.clang-tidy` or via inline comments:
   ```cpp
   // cppcheck-suppress unusedFunction
   void myFunction() { }
   ```

2. **Adjust cppcheck flags** in `scripts/lint_cpp.sh`:
   ```bash
   # Add suppressions to cppcheck command
   cppcheck --enable=all --inconclusive --std=c++17 \
     --suppress=missingIncludeSystem \
     --suppress=unknownMacro \
     --suppress=unusedFunction \
     "${ROOT_DIR}/src/" "${ROOT_DIR}/extensions/"
   ```

3. **Focus on errors only** (not style):
   ```bash
   # In scripts/lint_cpp.sh, change --enable=all to --enable=warning,error
   cppcheck --enable=warning,error --std=c++17 ...
   ```

**Verification:**
```bash
./scripts/lint_cpp.sh cppcheck 2>&1 | grep -c "style:"
```

---

### Issue: clang-format Reports Formatting Issues

**Symptom:**
```
error: code should be clang-formatted [-Wclang-format-violations]
```

**Fix:**
```bash
# Auto-format all code
./scripts/lint_cpp.sh format-apply

# Check formatting without fixing
./scripts/lint_cpp.sh format-check
```

**Verification:**
```bash
git diff  # Should show formatting changes
git add -A
git commit -m "style: apply clang-format"
```

---

### Issue: Missing compile_commands.json

**Symptom:**
```
Error while processing src/core/application.cpp.
Error: unable to handle compilation, expected exactly one compiler job
```

**Cause:** CMake didn't generate `build/compile_commands.json`.

**Fix:**
```bash
# Reconfigure with compile commands export
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -G Ninja
```

**Verification:**
```bash
ls -lh build/compile_commands.json
```

---

### Issue: CI Fails But Local Passes

**Cause:** Different tool versions, environment, or stale cache.

**Debug Steps:**

1. **Check CI environment**:
   ```bash
   # In CI logs, find tool versions
   grep -E "clang-tidy|cppcheck|clang-format" .github/workflows/build.yml
   ```

2. **Match local versions**:
   ```bash
   clang-tidy --version
   cppcheck --version
   clang-format --version
   ```

3. **Run in clean container**:
   ```bash
   # Build Docker image matching CI
   docker build -t crankshaft-ci -f Dockerfile .
   docker run -it crankshaft-ci bash
   # Inside container:
   ./scripts/lint_cpp.sh clang-tidy
   ```

---

## Debugging Workflow

### Step 1: Identify Failing Stage
Check GitHub Actions logs for which step failed:
- `Install Dev Tools` → Dependency installation issue
- `Check Code Formatting` → clang-format violations
- `Run Cppcheck` → Static analysis warnings/errors
- `Run clang-tidy` → Linter errors or config parse failure

### Step 2: Reproduce Locally
```bash
# Run the exact failing command from CI
cd /path/to/crankshaft_reborn

# Install tools
./scripts/install_dev_tools.sh

# Run specific check
./scripts/lint_cpp.sh <format-check|cppcheck|clang-tidy>
```

### Step 3: Fix Issues
- **Format issues**: Run `./scripts/lint_cpp.sh format-apply`
- **Config parse**: Edit `.clang-tidy` to remove unsupported keys
- **Real warnings**: Fix code or add suppressions

### Step 4: Verify & Commit
```bash
# Run all checks
./scripts/lint_cpp.sh format-check
./scripts/lint_cpp.sh cppcheck
./scripts/lint_cpp.sh clang-tidy

# Commit fixes
git add -A
git commit -m "fix: resolve code quality issues"
git push
```

---

## Reference: Tool Versions

### Ubuntu 24.04 (trixie) - CI Environment
```
clang-tidy version 19.1.6
cppcheck 2.17.1
clang-format version 19.1.6
```

### Checking Your Version
```bash
wsl bash -lc "clang-tidy --version; cppcheck --version; clang-format --version"
```

---

## Configuration Files

### `.clang-tidy`
Controls clang-tidy checks and severity. **Must be valid YAML**.

**Common Issues:**
- `GlobList` key requires clang-tidy 19+
- Invalid YAML syntax (indentation, duplicate keys)

**Validation:**
```bash
# Parse check
clang-tidy --list-checks --config-file=.clang-tidy 2>&1 | head -20
```

### `scripts/lint_cpp.sh`
Wrapper script for all lint operations.

**Customize checks:**
```bash
# Edit the script
nano scripts/lint_cpp.sh

# Adjust cppcheck suppressions in run_cppcheck()
# Adjust clang-tidy timeout/parallelism in run_clang_tidy()
```

---

## Quick Fixes

### Skip clang-tidy Temporarily
```bash
# Comment out in .github/workflows/build.yml
# - name: Run clang-tidy
#   run: bash ./scripts/lint_cpp.sh clang-tidy
```

### Reduce cppcheck Strictness
```bash
# In scripts/lint_cpp.sh, change:
cppcheck --enable=warning,error ...  # Instead of --enable=all
```

### Auto-fix All Formatting
```bash
./scripts/lint_cpp.sh format-apply
git add -A
git commit -m "style: apply clang-format"
git push
```

---

## Getting Help

1. **Check CI logs** for exact error messages
2. **Reproduce locally** using steps above
3. **Review recent commits** that might have introduced issues
4. **Compare tool versions** between local and CI

**Useful Commands:**
```bash
# View full clang-tidy output
./scripts/lint_cpp.sh clang-tidy 2>&1 | tee clang-tidy.log

# Count warnings by type
./scripts/lint_cpp.sh cppcheck 2>&1 | grep -oP '\[\K[^\]]+' | sort | uniq -c | sort -rn

# Check specific file
clang-tidy -p build src/core/application.cpp
```
