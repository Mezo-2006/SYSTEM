$ErrorActionPreference = "Stop"

$cfgFile = "src/gui/MainWindow.cpp"
$parserFile = "src/core/Parser.cpp"
$derivationFile = "src/gui/DerivationViewer.cpp"

if (!(Test-Path $cfgFile) -or !(Test-Path $parserFile) -or !(Test-Path $derivationFile)) {
    Write-Error "Required source files are missing."
}

$cfgText = Get-Content -Path $cfgFile -Raw
$parserText = Get-Content -Path $parserFile -Raw
$derivationText = Get-Content -Path $derivationFile -Raw

$checks = @(
    @{ Name = "CFG header exists"; Ok = $cfgText.Contains("Professional CFG (LL(1)-Friendly, No Left Recursion)") },
    @{ Name = "CFG has program rule"; Ok = $cfgText.Contains("R1   program         -> preamble_opt pre_stmt_list switch_stmt") },
    @{ Name = "CFG has switch rule"; Ok = $cfgText.Contains("R15  switch_stmt     -> switch ( expr ) { case_list default_clause }") },
    @{ Name = "CFG has case/default rules"; Ok = ($cfgText.Contains("R18  case_clause") -and $cfgText.Contains("R19  default_clause")) },
    @{ Name = "CFG has LL(1) arithmetic rules"; Ok = ($cfgText.Contains("R25  expr            -> term expr_tail") -and $cfgText.Contains("R29  term            -> factor term_tail")) },
    @{ Name = "CFG includes identifier/digit formation"; Ok = ($cfgText.Contains("R37  identifier") -and $cfgText.Contains("R46  digit")) },
    @{ Name = "CFG states no left recursion"; Ok = $cfgText.Contains("No left-recursive productions are used in this displayed CFG") },
    @{ Name = "CFG clarifies string limitation"; Ok = $cfgText.Contains("String support is intentionally limited to direct assignment semantics") },
    @{ Name = "CFG notes loop restriction"; Ok = $cfgText.Contains("Loops and full C++ features are intentionally rejected") },
    @{ Name = "Parser handles case list"; Ok = $parserText.Contains("while (match(TokenType::CASE))") },
    @{ Name = "Parser handles optional default"; Ok = $parserText.Contains("if (match(TokenType::DEFAULT))") },
    @{ Name = "Parser accepts int and string declaration tokens"; Ok = $parserText.Contains("type == TokenType::INT || type == TokenType::STRING") },
    @{ Name = "Parse tree includes case_list non-terminal"; Ok = $parserText.Contains('makeNonTerminal("case_list")'.Replace('\\"','"')) },
    @{ Name = "Parse tree includes default_clause non-terminal"; Ok = $parserText.Contains('makeNonTerminal("default_clause")'.Replace('\\"','"')) },
    @{ Name = "Derivation view has autoplay logic"; Ok = $derivationText.Contains("onAutoPlay") },
    @{ Name = "Derivation view has step list"; Ok = $derivationText.Contains("stepListWidget") }
)

$pass = 0
$fail = 0

foreach ($check in $checks) {
    if ($check.Ok) {
        Write-Host "[PASS] $($check.Name)" -ForegroundColor Green
        $pass++
    } else {
        Write-Host "[FAIL] $($check.Name)" -ForegroundColor Red
        $fail++
    }
}

Write-Host ""
Write-Host "CFG consistency summary: $pass passed, $fail failed"

if ($fail -gt 0) {
    exit 1
}

exit 0
