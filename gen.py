#!/usr/bin/python3

import cfile.src.cfile as C

hello = C.cfile('hello.c')
hello.code.append(C.sysinclude('stdio.h'))
hello.code.append(C.blank())
hello.code.append(C.function('main', 'int',).add_param(C.variable('argc', 'int')).add_param(C.variable('argv', 'char', pointer=2)))
body = C.block(innerIndent=3)
body.append(C.statement(C.fcall('printf').add_arg(r'"Hello World!\n"')))
body.append(C.statement('return 0'))
hello.code.append(body)
print(str(hello))

with open('acdata.h') as topo_file:
    for line in topo_file:
        if ("//" not in line and "#" not in line and "{" not in line and "}" not in line):
            if ("struct" in line):
                print(line)  # The comma to suppress the extra new line char
