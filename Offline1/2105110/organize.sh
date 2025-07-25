#!/bin/bash

if [ "$#" -lt 4 ]; then
    echo "Usage: $0 <submission_path> <target_path> <test_path> <answer_path> [-v] [-noexecute] [-nolc] [-nocc] [-nofc]"
    exit 1
fi

SUBMISSION_DIR="$1"
TARGET_DIR="$2"
TEST_DIR="$3"
ANSWER_DIR="$4"

CURRENT_DIR=$(pwd)

shift 4

VERBOSE=false
NOEXECUTE=false
NOLC=false
NOCC=false
NOFC=false

for ARG in "$@"; do
    case $ARG in
        -v) VERBOSE=true ;;
        -noexecute) NOEXECUTE=true ;;
        -nolc) NOLC=true ;;
        -nocc) NOCC=true ;;
        -nofc) NOFC=true ;;
    esac
done

mkdir -p "$TARGET_DIR"/{C,C++,Python,Java}

COMMON_TEMP_DIR="temp"
mkdir -p "$COMMON_TEMP_DIR"

rm -rf result.csv

HEADER="student_id,student_name,language"
[ "$NOEXECUTE" = false ] && HEADER+=",matched,not_matched"
[ "$NOLC" = false ] && HEADER+=",line_count"
[ "$NOCC" = false ] && HEADER+=",comment_count"
[ "$NOFC" = false ] && HEADER+=",function_count"
echo "$HEADER" >> result.csv

for FILE in "$SUBMISSION_DIR"/*.zip; do
    STUDENT_ID=$(echo "$FILE" | cut -d"_" -f4 | cut -d'.' -f1 | tr -d '[:space:]')
    STUDENT_NAME=$(basename "$FILE" | cut -d'_' -f1 | sed 's/[[:space:]]*$//')

    if [ "$VERBOSE" = true ]; then
        echo -e "\n📦 Organizing submission: $(basename "$FILE")"
        echo "  Student ID: $STUDENT_ID"
        echo "  Student Name: $STUDENT_NAME"
    fi

    unzip -qq "$FILE" -d "$COMMON_TEMP_DIR"

    SRC_FILE=$(find "$COMMON_TEMP_DIR" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.java" -o -name "*.py" \) | head -n 1)

    if [ -z "$SRC_FILE" ]; then
        echo "No valid code file found in $FILE"
        continue
    fi

    EXT=$(echo "$SRC_FILE" | rev | cut -d'.' -f1 | rev)

    case "$EXT" in
        c) LANG="C" ;;
        cpp) LANG="C++" ;;
        java) LANG="Java" ;;
        py) LANG="Python" ;;
        *)
            echo "Unsupported file extension: $EXT"
            continue
            ;;
    esac

    STUDENT_DIR="$TARGET_DIR/$LANG/$STUDENT_ID"
    mkdir -p "$STUDENT_DIR"

    if [ "$EXT" = "java" ]; then
        cp "$SRC_FILE" "$STUDENT_DIR/Main.java"
    else
        cp "$SRC_FILE" "$STUDENT_DIR/main.$EXT"
    fi

    if [ "$VERBOSE" = true ]; then
        echo "  ➤ Source file detected: $(basename "$SRC_FILE")"
        echo "  ➤ Language detected: $LANG"
        echo "  ➤ Copied to: $STUDENT_DIR"
    fi

    if [ "$NOEXECUTE" = false ]; then
        if [ "$VERBOSE" = true ]; then
            echo "🔧 Compiling..."
        fi

        if [ "$LANG" = "C" ]; then
            gcc "$STUDENT_DIR/main.$EXT" -o "$STUDENT_DIR/main.out" 2>/dev/null
            [ $? -eq 0 ] && $VERBOSE && echo "  ✓ C compilation successful" || echo "  ✗ Compilation failed"
        elif [ "$LANG" = "C++" ]; then
            g++ "$STUDENT_DIR/main.$EXT" -o "$STUDENT_DIR/main.out" 2>/dev/null
            [ $? -eq 0 ] && $VERBOSE && echo "  ✓ C++ compilation successful" || echo "  ✗ Compilation failed"
        elif [ "$LANG" = "Java" ]; then
            javac "$STUDENT_DIR/Main.java" 2>/dev/null
            [ $? -eq 0 ] && $VERBOSE && echo "  ✓ Java compilation successful" || echo "  ✗ Compilation failed"
        fi

        MATCHED=0
        NOT_MATCHED=0

        if [ "$VERBOSE" = true ]; then
            echo "🧪 Running test cases:"
        fi

        for TEST_FILE in "$TEST_DIR"/*; do
            TEST_NUM=$(basename "$TEST_FILE" | grep -o -E '[0-9]+')
            OUTPUT_FILE="$STUDENT_DIR/out${TEST_NUM}.txt"
            EXPECTED_OUTPUT="$ANSWER_DIR/ans${TEST_NUM}.txt"

            if [ "$LANG" = "C" ] || [ "$LANG" = "C++" ]; then
                "$STUDENT_DIR/main.out" < "$TEST_FILE" > "$OUTPUT_FILE" 2>/dev/null
            elif [ "$LANG" = "Java" ]; then
                (
                    cd "$STUDENT_DIR"
                    java "Main" < "$CURRENT_DIR/$TEST_FILE" > "out${TEST_NUM}.txt" 2>/dev/null
                    cd "$CURRENT_DIR"
                )
            elif [ "$LANG" = "Python" ]; then
                python3 "$STUDENT_DIR/main.py" < "$TEST_FILE" > "$OUTPUT_FILE" 2>/dev/null
            fi

            diff -q "$OUTPUT_FILE" "$EXPECTED_OUTPUT" > /dev/null
            if [ $? -eq 0 ]; then
                ((MATCHED++))
                [ "$VERBOSE" = true ] && echo "  ✓ Test case $TEST_NUM passed"
            else
                ((NOT_MATCHED++))
                [ "$VERBOSE" = true ] && echo "  ✗ Test case $TEST_NUM failed"
            fi
        done
    fi

    CODE_FILE="$STUDENT_DIR/main.$EXT"
    [ "$LANG" = "Java" ] && CODE_FILE="$STUDENT_DIR/Main.java"

    [ "$NOLC" = false ] && LINE_COUNT=$(wc -l < "$CODE_FILE")
    if [ "$LANG" = "Python" ]; then
        [ "$NOCC" = false ] && COMMENT_COUNT=$(grep -c '#' "$CODE_FILE")
        [ "$NOFC" = false ] && FUNCTION_COUNT=$(grep -cE '^\s*def ' "$CODE_FILE")
    elif [ "$LANG" = "Java" ]; then
        [ "$NOCC" = false ] && COMMENT_COUNT=$(grep -c '//' "$CODE_FILE")
        [ "$NOFC" = false ] && FUNCTION_COUNT=$(grep -cE '^\s*(public|private|protected)?\s*(static\s+)?\w+\s+\w+\s*\(.*\)\s*\{' "$CODE_FILE")
    else
        [ "$NOCC" = false ] && COMMENT_COUNT=$(grep -c '//' "$CODE_FILE")
        [ "$NOFC" = false ] && FUNCTION_COUNT=$(grep -cE '^\s*\w+\s+\w+\s*\(.*\)\s*\{' "$CODE_FILE")
    fi

    OUT="$STUDENT_ID,\"$STUDENT_NAME\",$LANG"
    [ "$NOEXECUTE" = false ] && OUT+=",$MATCHED,$NOT_MATCHED"
    [ "$NOLC" = false ] && OUT+=",$LINE_COUNT"
    [ "$NOCC" = false ] && OUT+=",$COMMENT_COUNT"
    [ "$NOFC" = false ] && OUT+=",$FUNCTION_COUNT"

    echo "$OUT" >> result.csv

    rm -rf "$COMMON_TEMP_DIR"/*
done

mv result.csv "$TARGET_DIR"/result.csv
rm -rf "$COMMON_TEMP_DIR"
