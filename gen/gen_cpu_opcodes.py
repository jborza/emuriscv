with open('rv32i.txt','r') as f:
    lines = [line.rstrip('\n') for line in f.readlines()]
    #print(lines)
    for line in lines:
        print('void %s(State* state, word* instruction) {' % line)
        print('printf("%s not implemented!\\n"); exit(1);' % line)
        print('}')


    for line in lines:
        #print(line)
        print('else if ((*instruction & MASK_%s) == MATCH_%s) {' % (line.upper(), line.upper()))
        print('     %s(state, instruction);' % line)
        print('}')