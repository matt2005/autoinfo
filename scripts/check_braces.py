from pathlib import Path
p=Path('extensions/navigation/qml/NavigationView.qml')
s=p.read_text()
count=0
minc=0
minl=None
for i,l in enumerate(s.splitlines(),1):
    for ch in l:
        if ch=='{': count+=1
        elif ch=='}': count-=1
    if count<minc:
        minc=count; minl=i
print('final',count,'most-negative',minc,'at',minl)
if minl:
    lines=s.splitlines()
    start=max(1,minl-5)
    end=min(len(lines),minl+5)
    print(f'Context {start}..{end}:')
    for i in range(start,end+1):
        print(f"{i:04d}: {lines[i-1]}")
else:
    # show tail
    lines=s.splitlines()
    for i in range(len(lines)-20,len(lines)):
        print(f"{i+1:04d}: {lines[i]}")
