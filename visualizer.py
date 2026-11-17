"""Generate process tree PNG and execution timeline PNG from C event log."""
import csv
from collections import defaultdict
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from graphviz import Digraph

BASE = Path(__file__).resolve().parent
with (BASE / 'process_events.csv').open(newline='') as f:
    events = list(csv.DictReader(f))
if not events:
    raise SystemExit('No events found. Run ./process first.')
for e in events:
    e['timestamp'] = float(e['timestamp'])
    for key in ('pid', 'ppid', 'child_pid'):
        e[key] = int(e[key])
events.sort(key=lambda e: e['timestamp'])

nodes = {e['pid'] for e in events}
parents = {e['pid'] for e in events if e['event'] == 'FORK'}
exited = {e['pid'] for e in events if e['event'] == 'EXIT'}
dot = Digraph('Process Tree', graph_attr={'rankdir': 'TB', 'bgcolor': '#101827'})
for pid in sorted(nodes):
    role = 'Parent' if pid in parents else 'Child'
    dot.node(str(pid), f'{role}\nPID {pid}', style='rounded,filled', shape='box',
             fillcolor='#cbd5e1' if pid in exited else '#93c5fd')
for e in events:
    if e['event'] == 'FORK':
        dot.edge(str(e['pid']), str(e['child_pid']), color='#94a3b8')
dot.render(str(BASE / 'process_tree'), format='png', cleanup=True)

by_pid = defaultdict(list)
for e in events:
    by_pid[e['pid']].append(e)
fig, ax = plt.subplots(figsize=(11, 5))
t0 = events[0]['timestamp']
for row, (pid, records) in enumerate(sorted(by_pid.items())):
    start = next((e['timestamp'] for e in records if e['event'] == 'START'), None)
    if start is None:
        continue
    cursor, state = start, 'Active'
    for e in records:
        if e['event'] not in ('SLEEP', 'WAKE', 'EXIT'):
            continue
        t = e['timestamp']
        if t > cursor:
            ax.barh(row, t-cursor, left=cursor-t0, height=.55,
                    color='#22c55e' if state == 'Active' else '#f59e0b')
        cursor = t
        state = {'SLEEP': 'Sleeping', 'WAKE': 'Active', 'EXIT': 'Exited'}[e['event']]
ax.set_yticks(range(len(by_pid)), [f'PID {pid}' for pid in sorted(by_pid)])
ax.set_xlabel('Seconds since first event')
ax.set_title('Logged process activity (not kernel CPU scheduling states)')
ax.legend(handles=[Patch(color='#22c55e', label='Active'), Patch(color='#f59e0b', label='Sleeping')])
ax.grid(axis='x', alpha=.2)
fig.tight_layout()
fig.savefig(BASE / 'process_timeline.png', dpi=180)
print('Created process_tree.png and process_timeline.png')
