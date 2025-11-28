from pathlib import Path
p=Path('extensions/navigation/qml/NavigationView.qml')
s=p.read_text()
stack=[]
for i,l in enumerate(s.splitlines(),1):
    for j,ch in enumerate(l,1):
        if ch=='{': stack.append((i,j))
        elif ch=='}':
            if stack:
                stack.pop()
            else:
                print('Unmatched closing at',i,j)
if stack:
    print('Unmatched opening braces (line,col):')
    for item in stack[-10:]:
        print(item)
else:
    print('All braces matched')
