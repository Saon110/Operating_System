#!/bin/bash

# Check mandatory arguments
if [ $# -lt 4 ]; then
    echo "Usage: $0 <submission_folder> <target_folder> <test_folder> <answer_folder> [options]"
    echo "Options:"
    echo "  -v: verbose mode"
    echo "  -noexecute: skip execution and matching"
    echo "  -nolc: skip line count"
    echo "  -nocc: skip comment count"
    echo "  -nofc: skip function count"
    exit 1
fi

# ==================================================================
# Task A: File Organization (using your working code)
# ==================================================================

SUBMISSIONS_DIR="$1"
TARGET_DIR="$2"
TESTS_DIR="$3"
ANSWERS_DIR="$4"
CSV_FILE="$TARGET_DIR/result.csv"

# Create target directories
mkdir -p "$TARGET_DIR/C" "$TARGET_DIR/C++" "$TARGET_DIR/Java" "$TARGET_DIR/Python"

# Process each submission
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

# ==================================================================
# Task B: Code Analysis
# ==================================================================

# Process optional arguments
VERBOSE=0
NOEXECUTE=0
NOLC=0
NOCC=0
NOFC=0

for arg in "${@:5}"; do
    case "$arg" in
        -v) VERBOSE=1 ;;
        -noexecute) NOEXECUTE=1 ;;
        -nolc) NOLC=1 ;;
        -nocc) NOCC=1 ;;
        -nofc) NOFC=1 ;;
    esac
done

# Initialize CSV header
echo -n "student_id,student_name,language" > "$CSV_FILE"
[ "$NOLC" -eq 0 ] && echo -n ",line_count" >> "$CSV_FILE"
[ "$NOCC" -eq 0 ] && echo -n ",comment_count" >> "$CSV_FILE"
[ "$NOFC" -eq 0 ] && echo -n ",function_count" >> "$CSV_FILE"
[ "$NOEXECUTE" -eq 0 ] && echo -n ",matched,not_matched" >> "$CSV_FILE"
echo "" >> "$CSV_FILE"

# ==================================================================
# Task C: Execution & Matching
# ==================================================================

process_student() {
    local lang="$1"
    local student_dir="$2"
    local student_id="$3"
    
    # Get student name from zip file
    local student_name=$(basename "$(ls "$SUBMISSIONS_DIR"/*"$student_id".zip)" | cut -d. -f1)
    student_name=$(echo "$student_name" | sed 's/ /_/g')

    # Find code file
    local code_file="$student_dir/$target_file"
    [ ! -f "$code_file" ] && return

    # Calculate metrics
    local line_count=0
    local comment_count=0
    local function_count=0

    # Line count
    if [ "$NOLC" -eq 0 ]; then
        line_count=$(wc -l < "$code_file")
    fi

    # Comment count
    if [ "$NOCC" -eq 0 ]; then
        case "$lang" in
            C|C++|Java)
                comment_count=$(grep -c '//' "$code_file")
                ;;
            Python)
                comment_count=$(grep -c '#' "$code_file")
                ;;
        esac
    fi

    # Function count
    if [ "$NOFC" -eq 0 ]; then
        case "$lang" in
            C|C++)
                function_count=$(grep -cE '^\w+\s+\w+\(.*\)\s*{' "$code_file")
                ;;
            Java)
                function_count=$(grep -cE '^\s*(public|private|protected)?\s+(static)?\s+\w+\s+\w+\(.*\)\s*{' "$code_file")
                ;;
            Python)
                function_count=$(grep -cE '^def\s+\w+\(.*\):' "$code_file")
                ;;
        esac
    fi

    # Execution and matching
    local matched=0
    local not_matched=0

    if [ "$NOEXECUTE" -eq 0 ]; then
        # Compile code
        case "$lang" in
            C)
                gcc "$code_file" -o "$student_dir/main.out" 2>/dev/null
                exe="$student_dir/main.out"
                ;;
            C++)
                g++ "$code_file" -o "$student_dir/main.out" 2>/dev/null
                exe="$student_dir/main.out"
                ;;
            Java)
                javac "$code_file" -d "$student_dir" 2>/dev/null
                exe="java -cp $student_dir Main"
                ;;
            Python)
                exe="python3 $code_file"
                ;;
        esac

        # Process test cases
        for test_file in "$TESTS_DIR"/test*.txt; do
            local test_num=$(basename "$test_file" | grep -oE '[0-9]+')
            local output_file="$student_dir/out$test_num.txt"
            local answer_file="$ANSWERS_DIR/ans$test_num.txt"

            # Execute program
            case "$lang" in
                C|C++|Java)
                    timeout 5s $exe < "$test_file" > "$output_file" 2>/dev/null
                    ;;
                Python)
                    timeout 5s python3 "$code_file" < "$test_file" > "$output_file" 2>/dev/null
                    ;;
            esac

            # Compare results
            if diff -q "$output_file" "$answer_file" >/dev/null; then
                matched=$((matched + 1))
            else
                not_matched=$((not_matched + 1))
            fi
        done
    fi

    # Write to CSV
    echo -n "$student_id,\"$student_name\",$lang" >> "$CSV_FILE"
    [ "$NOLC" -eq 0 ] && echo -n ",$line_count" >> "$CSV_FILE"
    [ "$NOCC" -eq 0 ] && echo -n ",$comment_count" >> "$CSV_FILE"
    [ "$NOFC" -eq 0 ] && echo -n ",$function_count" >> "$CSV_FILE"
    [ "$NOEXECUTE" -eq 0 ] && echo -n ",$matched,$not_matched" >> "$CSV_FILE"
    echo "" >> "$CSV_FILE"
}

# Main processing loop
for lang_dir in "$TARGET_DIR"/C "$TARGET_DIR"/C++ "$TARGET_DIR"/Java "$TARGET_DIR"/Python; do
    lang=$(basename "$lang_dir")
    [ "$VERBOSE" -eq 1 ] && echo "Processing $lang submissions..."
    
    for student_dir in "$lang_dir"/*/; do
        student_id=$(basename "$student_dir")
        [ "$VERBOSE" -eq 1 ] && echo " - Student $student_id"
        process_student "$lang" "$student_dir" "$student_id"
    done
done

# Sort CSV by student ID
(head -n 1 "$CSV_FILE" && tail -n +2 "$CSV_FILE" | sort -t, -k1) > tmp.csv && mv tmp.csv "$CSV_FILE"

echo "All tasks completed successfully!"