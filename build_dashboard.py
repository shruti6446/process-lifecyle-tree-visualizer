#!/usr/bin/env python3
import csv,json
from pathlib import Path
base=Path(__file__).resolve().parent
runs=[]
for i in range(1,8):
 path=base/'runs'/f'run_{i}.csv'
 with path.open(newline='',encoding='utf-8') as f:
  runs.append([{'timestamp':float(r['timestamp']),'pid':int(r['pid']),'ppid':int(r['ppid']),'event':r['event'],'child_pid':int(r['child_pid'])} for r in csv.DictReader(f)])
html=(base/'index.html').read_text(encoding='utf-8')
encode=lambda x:json.dumps(x,separators=(',',':')).replace('<','\u003c')
html=html.replace('/*PROCESS_EVENTS*/[]',encode(runs[0]),1)
html=html.replace('/*CAPTURED_RUNS*/[]',encode(runs),1)
(base/'dashboard.html').write_text(html,encoding='utf-8')
print('Embedded seven real Linux traces:',*[len(r) for r in runs])
