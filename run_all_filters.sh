#!/bin/bash

INPUT_DIR=images
OUTPUT_DIR=images/outputs
EXEC=./build/image_filter
PLATFORM=${1:-0}

FILTERS=("sobel" "gauss" "median" "luma")

mkdir -p "$OUTPUT_DIR"

i=1
for input in "$INPUT_DIR"/*.jpeg; do
    [ -e "$input" ] || continue

    for filter in "${FILTERS[@]}"; do
        output="$OUTPUT_DIR/output${i}_${filter}.png"
        echo "Running $filter on $(basename "$input") → $output"
        "$EXEC" --input "$input" --output "$output" --filter "$filter" --platform "$PLATFORM"
    done

    i=$((i + 1))
done
