from pathlib import Path
p=Path('extensions/navigation/qml/NavigationView.qml')
s=p.read_text()
count=0
for i,l in enumerate(s.splitlines(),1):
    for ch in l:
        if ch=='{': count+=1
        elif ch=='}': count-=1
    if i<=120 or i>780:
        print(f"{i:04d}: {count}: {l}")
print('final count',count)
