# Regression Testing

This project includes a command-line test runner that validates the compiler phases with both valid and intentionally broken inputs.

## Build

From the project root:

```powershell
cmake --build build --config Release
```

This builds:
- GUI executable: `build/Release/SwitchCaseCompiler.exe`
- CLI executable: `build/Release/SwitchCaseCompilerCLI.exe`

## Run Regression Suite

From the project root:

```powershell
powershell -ExecutionPolicy Bypass -File tests/run_regression_tests.ps1
```

## Run CFG Consistency Test

This checks that the CFG tab content and key parser/derivation implementation points are aligned.

```powershell
powershell -ExecutionPolicy Bypass -File tests/run_cfg_consistency_test.ps1
```

## Run Full Validation + Packaging

This command builds the project, runs all regression tests, deploys Qt runtime files,
verifies required DLL/plugins, and creates a distributable EXE package.

```powershell
powershell -ExecutionPolicy Bypass -File tests/run_full_validation.ps1
```

Outputs:
- Folder package: `dist/SwitchCaseCompiler-Release/`
- Zip package: `dist/SwitchCaseCompiler-Release.zip`

## Included Cases

- `tests/cases/valid_basic_switch.txt` expects full successful pipeline.
- `tests/cases/lexical_illegal_char.txt` expects lexical failure.
- `tests/cases/syntax_missing_semicolon.txt` expects parser failure.
- `tests/cases/semantic_duplicate_case.txt` expects semantic failure.
- `tests/cases/cpp_main_not_supported.txt` expects parser rejection of full C++ `int main` input.
- `tests/cases/valid_declarations_and_math.txt` validates declarations + arithmetic + switch.
- `tests/cases/semantic_undeclared_assignment.txt` validates declaration-before-assignment rule.
- `tests/cases/semantic_undeclared_switch_condition.txt` validates undeclared switch condition detection.
- `tests/cases/syntax_loop_not_supported.txt` validates explicit rejection of loops.

Each case validates:
- Exact compiler exit code.
- Required phrase-level diagnostics (not just pass/fail status).

## Exit Codes (CLI)

- `0`: success
- `1`: lexical error
- `2`: syntax error
- `3`: semantic error
- `64`: CLI usage error
- `66`: input file read error
