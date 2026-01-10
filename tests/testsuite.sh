#!/bin/sh

if [ -z "${BIN_PATH:-}" ]; then
    echo "[ERROR] BIN_PATH is not set" >&2
    exit 0
fi

TEST_ROOT="test_func"

TOTAL=0
PASS=0

for dir in "$TEST_ROOT"/*; do
    [ -d "$dir" ] || continue

    sh_file=$(ls "$dir"/*.sh 2>/dev/null)
    out_file=$(ls "$dir"/*.out 2>/dev/null)
    sta_file=$(ls "$dir"/*.sta 2>/dev/null)

    if [ ! -f "$sh_file" ] || [ ! -f "$out_file" ] || [ ! -f "$sta_file" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    out_tmp=$(mktemp)

    timeout 2 "$BIN_PATH" "$sh_file" >"$out_tmp" 2>/dev/null
    rc=$?
    exp_rc=$(cat "$sta_file")

    diff -u "$out_file" "$out_tmp" >/dev/null
    diff_rc=$?

    if [ "$diff_rc" -eq 0 ] && [ "$rc" = "$exp_rc" ]; then
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
    fi

    rm -f "$out_tmp"
done

if [ "$TOTAL" -eq 0 ]; then
    RESULT=0
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
