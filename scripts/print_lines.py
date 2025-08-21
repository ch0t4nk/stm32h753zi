import sys
p='features/feature_registry.json'
start=1008
end=1040
with open(p,'r',encoding='utf-8') as f:
    lines=f.readlines()
for i in range(start-1, min(end,len(lines))):
    print(f"{i+1:4}: {lines[i].rstrip()}")
