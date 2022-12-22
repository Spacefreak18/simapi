#!/usr/bin/python3

import cfile.src.cfile as C

simmap = C.cfile('simmap.c')
simmap.code.append(C.sysinclude('stdio.h'))
simmap.code.append(C.sysinclude('stddef.h'))
simmap.code.append(C.include('acdata.h'))
simmap.code.append(C.include('basicmap.h'))
simmap.code.append(C.blank())
simmap.code.append(C.function('CreateACMap', 'int',).add_param(C.variable('map', 'struct Map', pointer=1)).add_param(C.variable('spfp', 'struct SPageFilePhysics', pointer=1)))
body = C.block(innerIndent=4)

structname=""
mapnum=0
with open('acdata.h') as topo_file:
    for line in topo_file:
        if ("//" not in line and "#" not in line and "{" not in line and "}" not in line and "typedef" not in line):
            if ("struct" in line):
                structname = line.split()[1]
            elif (line.isspace()):
                continue
            else:
                rawline=line.split()
                variable=rawline[1][0:-1]
                type=rawline[0]
                if "]" in variable:
                    array_length=variable[variable.find("[")+1:variable.find("]")]
                    variable=variable[0:variable.find("[")]
                    for i in range(int(array_length)):
                        body.append(C.statement( "map["+str(mapnum)+"]->name = \"" + structname+"_"+ variable + str(i) + "\"" ))
                        body.append(C.statement( "map["+str(mapnum)+ "]->value = (((char*) spfp) + offsetof(struct " + structname + ", " + variable + ")) + sizeof(" + type + ") * " + str(i) ) )
                        mapnum = mapnum + 1
                else:
                    body.append(C.statement( "map["+str(mapnum)+ "]->name = \"" + structname + "_" + variable + "\"" ) )
                    body.append(C.statement( "map["+str(mapnum)+ "]->value = (((char*) spfp) + offsetof(struct " + structname + ", " + variable + "))" ) )
                    mapnum = mapnum + 1

body.append(C.statement('return 0'))
simmap.code.append(body)

with open("mapacdata.c", "w") as f:
    f.write(str(simmap))
