#!/usr/bin/python3
import re
import cfile.src.cfile as C

simmap = C.cfile('simmap.c')
simmap.code.append(C.sysinclude('stdio.h'))
simmap.code.append(C.sysinclude('stddef.h'))
simmap.code.append(C.include('basicmap.h'))
simmap.code.append(C.include('../simapi/ac.h'))
simmap.code.append(C.blank())
simmap.code.append(C.function('CreateACMap', 'int',).add_param(C.variable('map', 'struct Map', pointer=1)).add_param(C.variable('acmap', 'ACMap', pointer=1)))
body = C.block(innerIndent=4)

structname=""
structvar=""
mapnum=0

body.append(C.blank())

body.append(C.statement("char* spfp = acmap->physics_map_addr"))
body.append(C.statement("char* spfg = acmap->graphic_map_addr"))
body.append(C.statement("char* spfs = acmap->static_map_addr"))
body.append(C.statement("char* spfc = acmap->crewchief_map_addr"))

body.append(C.blank())

with open('acdata.h') as topo_file:
    for line in topo_file:
        if ("//" not in line and "#" not in line and "{" not in line and "}" not in line and "typedef" not in line):
            if ("struct" in line):
                structname = line.split()[1]

                if ( structname == "SPageFileStatic" ):
                    structvar = "spfs"
                elif ( structname == "SPageFileGraphic" ):
                    structvar = "spfg"
                elif ( structname == "SPageFileCrewChief" ):
                    structvar = "spfc"
                elif ( structname == "SPageFilePhysics" ):
                    structvar = "spfp"
                else:
                    structname = ""

            elif (line.isspace()):
                continue
            elif (structname == ""):
                continue
            else:
                rawline=line.split()
                variable=rawline[1][0:-1]
                type=rawline[0]
                if "]" in variable:
                    array_length=variable[variable.find("[")+1:variable.find("]")]
                    variable=variable[0:variable.find("[")]
                    for i in range(int(array_length)):
                        body.append(C.statement( "map["+str(mapnum)+"].name = \"" + structname+"_"+ variable + str(i) + "\"" ))
                        body.append(C.statement( "map["+str(mapnum)+ "].value = ((char*) " + structvar + " + offsetof(struct " + structname + ", " + variable + ")) + sizeof(" + type + ") * " + str(i) ) )
                        mapnum = mapnum + 1
                else:
                    body.append(C.statement( "map["+str(mapnum)+ "].name = \"" + structname + "_" + variable + "\"" ) )
                    body.append(C.statement( "map["+str(mapnum)+ "].value = ((char*) " + structvar + " + offsetof(struct " + structname + ", " + variable + "))" ) )
                    mapnum = mapnum + 1

body.append(C.statement('return 0'))
simmap.code.append(body)

with open("mapacdata.c", "w") as f:
    f.write(str(simmap))

filedata=""
with open('../simmap/mapacdata.h') as topo_file:


    with open('mapacdata.h', 'w') as topo_file2:

        for line in topo_file:
            if "#define ACMAP_SIZE" in line:
                topo_file2.write("#define ACMAP_SIZE    " + str(mapnum) + "\n")
            else:
                topo_file2.write(line)
