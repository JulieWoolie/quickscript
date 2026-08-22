clang -O3 fib.c -o fib.exe

Copy-Item "../cmake-build-llvm-mingw-debug/quickscript.exe" -Destination "./qs.exe"

$loops = 100

$cTimes = @()
$denoTimes = @()
$qsTimes = @()

Write-Host "Running benchmarks..."

Write-Host "Running C benchmarks..."
0..$loops | % {
    $elapsed = Measure-Command { ./fib.exe }
    $cTimes += $elapsed.TotalMilliseconds
}

Write-Host "Running deno benchmarks..."
0..$loops | % {
    $elapsed = Measure-Command { deno run fib.ts }
    $denoTimes += $elapsed.TotalMilliseconds
}

Write-Host "Running quickscript benchmarks..."
0..$loops | % {
    $elapsed = Measure-Command { ./qs run fibonacci.qscr }
    $qsTimes += $elapsed.TotalMilliseconds
}

$cAvg = ($cTimes | Measure-Object -Average).Average
$denoAvg = ($denoTimes | Measure-Object -Average).Average
$qsAvg = ($qsTimes | Measure-Object -Average).Average

Write-Host ""
Write-Host "Ran benchmark $loops times"
Write-Host "Average C time (ms): $cAvg"
Write-Host "Average Deno time (ms): $denoAvg"
Write-Host "Average quiskcript time (ms): $qsAvg"