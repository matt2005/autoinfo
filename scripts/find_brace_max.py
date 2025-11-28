from pathlib import Path
p=Path('extensions/navigation/qml/NavigationView.qml')
s=p.read_text()
count=0
maxc=0
maxl=None
for i,l in enumerate(s.splitlines(),1):
    for ch in l:
        if ch=='{': count+=1
        elif ch=='}': count-=1
    if count>maxc:
        maxc=count; maxl=i
print('final',count,'max',maxc,'at',maxl)
lines=s.splitlines()
start=max(1,maxl-3)
end=min(len(lines),maxl+3)
print(f'Context around max-count {maxl} ({start}..{end}):')
for i in range(start,end+1):
    print(f"{i:04d}: {lines[i-1]}")
