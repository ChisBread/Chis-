#coding:utf-8
import sys,re
mp = {}
st = set()
g_arrange = {1:set(["Y", "O", "X"])}
def arrangement_padding(size):
    if size in g_arrange:
        return g_arrange[size]
    ret = set()
    for a in arrangement_padding(size-1):
        if a[0] == "X":
            ret.add(a+"X")
        else:
            ret.add(a+"Y")
            ret.add(a+"O")
    g_arrange[size] = ret
    return ret
def create_more(pattern):
    ret = []
    paddinglen = 11-len(pattern)
    for i in range(paddinglen):
        lefts = []
        rights = []
        if i-1 > 0:
            lefts = arrangement_padding(i-1)#余下一位固定为Y
        if paddinglen-i-1 > 0:
            rights = arrangement_padding(paddinglen-i-1)#余下一位固定为Y
        #非边界
        for l in lefts:
            for r in rights:
                ret.append(l+"Y"+pattern+"Y"+r)
        for l in lefts:
            ret.append(l+"Y"+pattern+"X"*(paddinglen-i))
        for r in rights:
                ret.append("X"*i+pattern+"Y"+r)
        ret.append("X"*i+pattern+"X"*(paddinglen-i))
    return ret

def tohex(patstr):
    return hex(int(patstr.replace("_", "00").replace("O", "10").replace("X", "11"),2))
for line in sys.stdin:
    col = line.strip().split("\t")
    # patterns = create_more(re.match(r'XX_*(X[O_]*X)_*XX', col[-1]).group(1))
    patstr = re.match(r'XX_*(X[O_]*X)_*XX', col[-1]).group(1)
    if col[-1] in mp:
        continue
    idx = int(col[-2].replace('a', '10').replace('b', '11').replace('c', '12'))
    st.add(idx)
    mp[tohex(patstr)] = idx
idx2i = {}
for i, idx in enumerate(sorted(list(st))):
    idx2i[idx] = i
print('#include "resource/patterns.h"')
print('')
print('namespace chis {')
print("uint32_t patterns[%s] = {%s};"%(len(mp), ", ".join([ str(k)+str(hex(idx2i[v]))[2:] for k, v in mp.items() ])))
print('}// namespace chis')