import re

print('#include "debug_symbols.h"')
print('void initialize_symbols() {')
print('symbol* current = NULL;')
print('	current = add_symbol(NULL, 0x0, "UNKNOWN");')
print('	symbol_list = current; //only after 1st iteration')

with open('symbols.txt','r',encoding='utf-8') as f:
    for line in f.readlines():
        # 80000000 <_ftext>:
        #	current = add_symbol(current, 0x80000000, "_ftext");
        address, symbol = re.match('(.+) <(.+)>',line).groups()
        print('current = add_symbol(current, 0x%s, "%s");' % (address, symbol))
print('}')