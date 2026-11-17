#!/bin/bash
set -eu
cd "$(dirname "$0")/.."
./json-parser examples/profile.json
if ./json-parser >/dev/null 2>&1; then
    echo 'FAIL: missing argument accepted'; exit 1
fi
if ./json-parser build/nonexistent-file.json >/dev/null 2>&1; then
    echo 'FAIL: missing file accepted'; exit 1
fi
printf '{"broken":}' > build/invalid-cli.json
if ./json-parser build/invalid-cli.json >/dev/null 2>&1; then
    echo 'FAIL: invalid JSON accepted'; exit 1
fi
output=$(./build/read-profile examples/profile.json)
case "$output" in *'Name: Kavya'*'Age: 20'*'First skill: C++'*) ;; *) echo 'FAIL: profile output'; exit 1 ;; esac
printf '5 CLI checks passed\n'
