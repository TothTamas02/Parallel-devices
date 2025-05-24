# PowerShell version of run_all_filters.sh

$INPUT_DIR = "images"
$OUTPUT_DIR = "images\outputs"
$EXEC = ".\build\image_filter.exe"
$PLATFORM = if ($args.Count -ge 1) { $args[0] } else { 0 }
$FILTERS = @("sobel", "gauss", "median", "luma")

if (!(Test-Path $OUTPUT_DIR)) {
    New-Item -ItemType Directory -Path $OUTPUT_DIR | Out-Null
}

$i = 1
Get-ChildItem "$INPUT_DIR\*.jpeg" | ForEach-Object {
    $input = $_.FullName
    foreach ($filter in $FILTERS) {
        $output = "$OUTPUT_DIR\output${i}_${filter}.png"
        Write-Host "Running $filter on $($_.Name) -> $output"
        & $EXEC --input "$input" --output "$output" --filter "$filter" --platform "$PLATFORM"
    }
    $i++
}