#!/bin/bash

INPUT_DIR=images
OUTPUT_DIR=images/outputs
EXEC=./build/image_filter
DEVICE=${1:-cpu}  # use first argument, default to "cpu"

FILTERS=("sobel" "gauss" "median" "luma")

mkdir -p "$OUTPUT_DIR"

i=1
for input in "$INPUT_DIR"/*.jpeg; do
    [ -e "$input" ] || continue

    for filter in "${FILTERS[@]}"; do
        output="$OUTPUT_DIR/output${i}_${filter}_${DEVICE}.png"
        echo "Running $filter on $(basename "$input") → $output"
        "$EXEC" --input "$input" --output "$output" --filter "$filter" --device "$DEVICE"
    done

    i=$((i + 1))
done
