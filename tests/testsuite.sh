#!/bin/sh

if [ -z "$BIN_PATH" ]; then
    echo "BIN_PATH not set" >&2
    exit 1
fi

PASS=0
TOTAL=0

for dir in test_func/*; do
    [ -d "$dir" ] || continue

    sh_file="$dir/$(basename $dir).sh"
    out_file="$dir/$(basename $dir).out"
    sta_file="$dir/$(basename $dir).sta"
    err_file="$dir/$(basename $dir).err"

    [ -f "$sh_file" ] || continue
    [ -f "$out_file" ] || continue
    [ -f "$sta_file" ] || continue

    TOTAL=$((TOTAL + 1))

    $BIN_PATH "$sh_file" > /tmp/out.tmp 2> /tmp/err.tmp
    rc=$?

    exp_rc=$(cat "$sta_file")

    if diff "$out_file" /tmp/out.tmp > /dev/null && [ "$rc" = "$exp_rc" ]; then
        if [ -f "$err_file" ]; then
            if diff "$err_file" /tmp/err.tmp > /dev/null; then
                PASS=$((PASS + 1))
            else
                echo "[FAIL] $dir (stderr diff)"
            fi
        else
            PASS=$((PASS + 1))
        fi
    else
        echo "[FAIL] $dir"
        [ "$rc" != "$exp_rc" ] && echo "  exit: got $rc expected $exp_rc"
        diff "$out_file" /tmp/out.tmp
    fi
done

echo "Passed: $PASS / $TOTAL"
