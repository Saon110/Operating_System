#!/bin/bash

SUBMISSIONS_DIR="./Shell-Scripting-Assignment-Files/Workspace/submissions"
TARGET_DIR="./targets"

mkdir -p "$TARGET_DIR/C" "$TARGET_DIR/C++" "$TARGET_DIR/Java" "$TARGET_DIR/Python"

for zip_file in "$SUBMISSIONS_DIR"/*.zip; do
    base_name=$(basename "$zip_file")
    student_id=$(echo "$base_name" | grep -oE '[0-9]{7}(\.zip$)' | grep -oE '[0-9]{7}')

    temp_dir=$(mktemp -d)
    unzip -q "$zip_file" -d "$temp_dir"

    code_file=$(find "$temp_dir" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.java" -o -name "*.py" \) | head -n 1)

    if [[ -n "$code_file" ]]; then
        case "$code_file" in
            *.c)
                lang_dir="C"
                target_file="main.c"
                ;;
            *.cpp)
                lang_dir="C++"
                target_file="main.cpp"
                ;;
            *.java)
                lang_dir="Java"
                target_file="Main.java"
                ;;
            *.py)
                lang_dir="Python"
                target_file="main.py"
                ;;
        esac

        student_dir="$TARGET_DIR/$lang_dir/$student_id"
        mkdir -p "$student_dir"

        cp "$code_file" "$student_dir/$target_file"
    fi

    rm -rf "$temp_dir"
done

echo "Task A completed. Organized files are in the 'targets' directory."