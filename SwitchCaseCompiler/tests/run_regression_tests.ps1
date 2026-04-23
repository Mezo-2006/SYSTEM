param(
    [string]$CompilerPath = "",
    [string]$CasesRoot = "tests/cases"
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($CompilerPath)) {
    $releaseCli = "build/Release/SwitchCaseCompilerCLI.exe"
    $debugCli = "build/Debug/SwitchCaseCompilerCLI.exe"

    if (Test-Path $releaseCli) {
        $CompilerPath = $releaseCli
    } elseif (Test-Path $debugCli) {
        $CompilerPath = $debugCli
    } else {
        Write-Error "Compiler CLI not found. Build the project first (cmake --build build --config Release)."
    }
}

$cases = @(
    @{
        Name = "valid_basic_switch"
        File = "valid_basic_switch.txt"
        ExitCode = 0
        MustContainAll = @("[phase] lexer", "[phase] parser", "[phase] semantic", "[status] success")
    },
    @{
        Name = "valid_declarations_and_math"
        File = "valid_declarations_and_math.txt"
        ExitCode = 0
        MustContainAll = @("[phase] codegen", "[status] success")
    },
    @{
        Name = "valid_switch_expression_condition"
        File = "valid_switch_expression_condition.txt"
        ExitCode = 0
        MustContainAll = @("[phase] codegen", "[status] success")
    },
    @{
        Name = "valid_string_switch_with_std_preamble"
        File = "valid_string_switch_with_std_preamble.txt"
        ExitCode = 0
        MustContainAll = @("[phase] semantic", "[status] success")
    },
    @{
        Name = "semantic_string_operator_not_supported"
        File = "semantic_string_operator_not_supported.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "String operands with arithmetic operators")
    },
    @{
        Name = "lexical_illegal_char"
        File = "lexical_illegal_char.txt"
        ExitCode = 1
        MustContainAll = @("[error][lexer]", "Illegal character")
    },
    @{
        Name = "syntax_missing_semicolon"
        File = "syntax_missing_semicolon.txt"
        ExitCode = 2
        MustContainAll = @("[error][parser]", "Expected ';' after statement")
    },
    @{
        Name = "semantic_duplicate_case"
        File = "semantic_duplicate_case.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "Duplicate case value: 1")
    },
    @{
        Name = "semantic_duplicate_declaration"
        File = "semantic_duplicate_declaration.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "Duplicate declaration of variable 'x'")
    },
    @{
        Name = "semantic_undeclared_assignment"
        File = "semantic_undeclared_assignment.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "must be declared before assignment")
    },
    @{
        Name = "semantic_undeclared_switch_condition"
        File = "semantic_undeclared_switch_condition.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "Undeclared variable 'x'")
    },
    @{
        Name = "semantic_uninitialized_use"
        File = "semantic_uninitialized_use.txt"
        ExitCode = 3
        MustContainAll = @("[error][semantic]", "may be used before initialization")
    },
    @{
        Name = "cpp_main_not_supported"
        File = "cpp_main_not_supported.txt"
        ExitCode = 2
        MustContainAll = @("[error][parser]", "Expected 'return' after switch statement")
    },
    @{
        Name = "syntax_loop_not_supported"
        File = "syntax_loop_not_supported.txt"
        ExitCode = 2
        MustContainAll = @("[error][parser]", "Loops are not supported")
    },
    @{
        Name = "user_sample_no_false_duplicates"
        File = "repro_user_input.txt"
        ExitCode = 0
        MustContainAll = @("[status] success")
    }
)

$passCount = 0
$failCount = 0

foreach ($case in $cases) {
    $casePath = Join-Path $CasesRoot $case.File

    if (-not (Test-Path $casePath)) {
        Write-Host "[FAIL] $($case.Name): missing case file $casePath" -ForegroundColor Red
        $failCount++
        continue
    }

    $output = & $CompilerPath --input $casePath 2>&1 | Out-String
    $actualExit = $LASTEXITCODE

    $exitOk = ($actualExit -eq $case.ExitCode)
    $missingTexts = @()
    foreach ($expectedText in $case.MustContainAll) {
        if (-not $output.ToLower().Contains($expectedText.ToLower())) {
            $missingTexts += $expectedText
        }
    }
    $textOk = ($missingTexts.Count -eq 0)

    if ($exitOk -and $textOk) {
        Write-Host "[PASS] $($case.Name)" -ForegroundColor Green
        $passCount++
    } else {
        Write-Host "[FAIL] $($case.Name)" -ForegroundColor Red
        Write-Host "  expected exit: $($case.ExitCode), actual: $actualExit"
        if ($missingTexts.Count -gt 0) {
            Write-Host "  missing expected text:"
            foreach ($missing in $missingTexts) {
                Write-Host "    - $missing"
            }
        }
        Write-Host "  output:"
        Write-Host $output
        $failCount++
    }
}

Write-Host ""
Write-Host "Regression summary: $passCount passed, $failCount failed"

if ($failCount -gt 0) {
    exit 1
}

exit 0
