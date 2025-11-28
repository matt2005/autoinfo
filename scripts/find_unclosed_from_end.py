from pathlib import Path
p=Path('extensions/navigation/qml/NavigationView.qml')
s=p.read_text()
count=0
negline=None
lines=s.splitlines()
for i,l in enumerate(reversed(lines),1):
    for ch in l:
        if ch=='}': count+=1
        elif ch=='{': count-=1
    if count<0 and negline is None:
        negline=len(lines)-i+1
        break
print('final_balance_from_end',count,'first-negative-from-end-line',negline)
if negline:
    start=max(1,negline-5); end=min(len(lines),negline+5)
    print('context',start,end)
    for i in range(start,end+1):
        print(f"{i:04d}: {lines[i-1]}")
else:
    print('No negative region from end; final count is',count)
