#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p runs
echo '[1/3] Compiling C process simulator...'
gcc -std=c11 -Wall -Wextra -O2 process.c -o process
echo '[2/3] Capturing seven genuine Linux fork() runs...'
for n in 1 2 3 4 5 6 7; do
  ./process "$n" "runs/run_${n}.csv"
done
cp runs/run_1.csv process_events.csv
echo '[3/3] Building browser dashboard with all seven captured traces...'
python3 build_dashboard.py
printf '\nReady: dashboard.html\n'
if command -v explorer.exe >/dev/null 2>&1; then explorer.exe "$(wslpath -w "$(pwd)/dashboard.html")" || true; fi
