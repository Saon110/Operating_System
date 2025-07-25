#!/bin/bash

SUBMISSIONS_DIR="./Shell-Scripting-Assignment-Files/Workspace/submissions"
TARGET_DIR="./targets"
CSV_FILE="./targets/result.csv"

echo "student_id,student_name,language,line_count,comment_count,function_count" > "$CSV_FILE"

for lang_dir in "$TARGET_DIR"/*; do
    for student_dir in "$lang_dir"/*; do
        student_id=$(basename "$student_dir")
        lang=$(basename "$lang_dir")

        for zip_file in "$SUBMISSIONS_DIR"/*.zip; do
            base_name=$(basename "$zip_file")
            if [[ "$base_name" == *"$student_id"* ]]; then
                student_name="\"${base_name%%_*}\""
                break
            fi
        done

        code_file=$(find "$student_dir" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.java" -o -name "*.py" \) | head -n 1)
        if [[ -f $code_file ]]; then
            line_count=$(wc -l < "$code_file")
            
            extension="${code_file##*.}"
            if [[ "$extension" == "py" ]]; then
                comment_count=$(grep -cE '.*#' "$code_file")
            else
                comment_count=$(grep -cE '//.*|/\*.*\*/' "$code_file")
            fi

            if [[ "$extension" == "c" || "$extension" == "cpp" ]]; then
                func_pattern='\s*\w+\s+\w+\s*\(.*\)\s*\{'
            elif [[ "$extension" == "java" ]]; then
                func_pattern='\s*(public\s+)?(static\s+)?\w+\s+\w+\s*\(.*\)\s*\{' 
            elif [[ "$extension" == "py" ]]; then
                func_pattern='^def '
            fi

            function_count=$(grep -cE "$func_pattern" "$code_file")

            echo "$student_id,$student_name,$(basename "$lang_dir"),$line_count,$comment_count,$function_count" >> "$CSV_FILE"
        fi
    done
done

(head -n 1 "$CSV_FILE" && tail -n +2 "$CSV_FILE" | sort -t, -k2,2) > "$CSV_FILE.tmp" && mv "$CSV_FILE.tmp" "$CSV_FILE"

echo "Task B completed. Results are in the 'result.csv' file."
