#!/bin/sh


COVERAGE_MODE=0
if [ "${COVERAGE:-}" = "yes" ]; then
    COVERAGE_MODE=1
fi
if [ -z "${BIN_PATH:-}" ]; then
    echo "[ERROR] BIN_PATH is not set" >&2
    exit 0
fi
TEST_ROOT="test_func"
TOTAL=0
TOTAL_C=0
PASS=0
PASS_C=0

if [ "$COVERAGE_MODE" -eq 1 ]; then
    echo "[INFO] Running unit tests" >&2
    for unit in tests/test_unitaires/test_*; do
        [ -x "$unit" ] || continue
        TOTAL_C=$((TOTAL_C + 1))

        if timeout 2 "$unit"; then
            PASS_C=$((PASS_C + 1))
        else
            echo "[FAIL] $unit" >&2
        fi
    done
fi


for dir in "$TEST_ROOT"/*; do
    [ -d "$dir" ] || continue

    sh_file=$(ls "$dir"/*.sh 2>/dev/null)
    out_file=$(ls "$dir"/*.out 2>/dev/null)
    sta_file=$(ls "$dir"/*.sta 2>/dev/null)
    err_file=$(ls "$dir"/*.err 2>/dev/null)
    in_file=$(ls "$dir"/*.in 2>/dev/null)
    if [ ! -f "$sh_file" ] || [ ! -f "$out_file" ] || [ ! -f "$sta_file" ]; then
        echo "[SKIP] MISSING TEST FILES " >&2
        continue
    fi
    if [ ! -f "$sta_file" ] && [ ! -f "$err_file" ] && [ ! -f "$in_file" ]; then
        echo "[SKIP] MISSING CODE FILES" >&2
        continue
    fi

    TOTAL=$((TOTAL + 1))
    out_tmp=$(mktemp)
    err_tmp=$(mktemp)

    if [ -f "$in_file" ]; then
        timeout 2 "$BIN_PATH" "$sh_file" <"$in_file" >"$out_tmp" 2>"$err_tmp"
    else
        timeout 2 "$BIN_PATH" "$sh_file" >"$out_tmp" 2>"$err_tmp"
    fi
    rc=$?
    exp_rc=$(cat "$sta_file")

    diff -u "$out_file" "$out_tmp" >/dev/null
    diff_rc=$?
    err_diff_rc=0
    if [ -f "$err_file" ]; then
        diff -u "$err_file" "$err_tmp" >/dev/null
        err_diff_rc=$?
    else
        [ -s "$err_tmp" ] && err_diff_rc=1
    fi

    if [ "$diff_rc" -eq 0 ] && [ "$err_diff_rc" -eq 0 ] && [ "$rc" = "$exp_rc" ]; then
        PASS=$((PASS + 1))
    else
        echo "[FAIL] $dir"

        if [ "$rc" != "$exp_rc" ]; then
            echo "DIFFERENT EXIT CODES : got $rc, expected $exp_rc"
        fi

        if [ "$diff_rc" -ne 0 ]; then
            echo "DIFFERENT OUTPUTS:"
            diff -u "$out_file" "$out_tmp"
        fi


        if [ "$err_diff_rc" -ne 0 ]; then
            echo "DIFFERENT ERROR OUTPUTS:"
            if [ -f "$err_file" ]; then
                diff -u "$err_file" "$err_tmp"
            else
                echo " expected: <empty stderr>"
                cat "$err_tmp"
            fi
        fi
    fi

    rm -f "$out_tmp"
    rm -f "$err_tmp"
done
if [ "$COVERAGE_MODE" -eq 1 ]; then
    RESULT=$(( (PASS * 70 + PASS_C * 30) / (TOTAL + TOTAL_C) ))
else
    RESULT=$((PASS * 100 / TOTAL))
fi
echo "====================" >&2
echo "Passed: $PASS / $TOTAL" >&2
echo "Score : $RESULT%" >&2
echo "====================" >&2

if [ -n "${OUTPUT_FILE:-}" ]; then
    echo "$RESULT" > "$OUTPUT_FILE"
fi

exit 0
