# Process Life-Cycle and Process Tree Visualiser

## WSL Ubuntu quick start

Install prerequisites (once):

```bash
sudo apt update
sudo apt install -y gcc python3 unzip
```

Open a WSL terminal in this extracted project directory and run:

```bash
bash run.sh
```

This compiles `process.c`, executes all **seven distinct scenarios**, creates `runs/run_1.csv` through `runs/run_7.csv`, and builds `dashboard.html` with all seven runs embedded. Open **dashboard.html**, not index.html, in your Windows browser. You can use `explorer.exe .` to open the folder from WSL.

In the **Overview** section, choose a run from the dropdown. The selection now loads automatically; the **View selected run** button does the same. The tree, timeline, registry, metrics, and events update together. No Streamlit, localhost, CSV import, or browser server is needed.

**If only one run appears:** Make sure you ran `bash run.sh`, opened the freshly generated **dashboard.html**, and refreshed the browser with Ctrl+Shift+R. `index.html` is only the source template and does not contain captured runs.

## C program usage

```bash
gcc -std=c11 -Wall -Wextra -Werror -O2 process.c -o process
./process 3 runs/manual_run_3.csv
```

Scenarios: 1 basic 3 children; 2 sequential forks; 3 nested tree; 4 wide 6-child tree; 5 varied sleep times; 6 two branching children with grandchildren; 7 staggered forks.

The CSV contains timestamp, PID, PPID, event and child PID. Logged active intervals are not a direct measure of CPU scheduling states. Process IDs change each execution.
