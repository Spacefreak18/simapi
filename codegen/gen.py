#!/usr/bin/python3
import re
import cfile.src.cfile as C

#game="Assetto Corsa"
game="simdata"
includefilename=""
mapperfilename=""
mapperheaderfilename=""
mapsizevariablename=""
if (game == "Assetto Corsa"):
    includefilename="acdata.h"
    mapperfilename="mapacdata.c"
    mapperheaderfilename="mapacdata.h"
    mapsizevariablename="#define ACMAP_SIZE"
if (game == "RFactor2"):
    includefilename="rf2data.h"
    mapperfilename="maprf2data.c"
    mapperheaderfilename="maprf2data.h"
    mapsizevariablename="#define RF2MAP_SIZE"
if (game == "simdata"):
    includefilename="simdata.h"
    mapperfilename="mapsimdata.c"
    mapperheaderfilename="basicmap.h"
    mapsizevariablename="#define SIMDATAMAP_SIZE"

simmap = C.cfile('simmap.c')
simmap.code.append(C.sysinclude('stdio.h'))
simmap.code.append(C.sysinclude('stddef.h'))
simmap.code.append(C.include('basicmap.h'))

if (game == "RFactor2"):
    simmap.code.append(C.include('../simapi/rf2.h'))
    simmap.code.append(C.blank())
    simmap.code.append(C.function('CreateRF2Map', 'int',).add_param(C.variable('map', 'struct Map', pointer=1)).add_param(C.variable('rf2map', 'RF2Map', pointer=1)).add_param(C.variable('mapdata', 'int', pointer=0)))
if (game == "Assetto Corsa"):
    simmap.code.append(C.include('../simapi/ac.h'))
    simmap.code.append(C.blank())
    simmap.code.append(C.function('CreateACMap', 'int',).add_param(C.variable('map', 'struct Map', pointer=1)).add_param(C.variable('acmap', 'ACMap', pointer=1)).add_param(C.variable('mapdata', 'int', pointer=0)))
if (game == "simdata"):
    simmap.code.append(C.include('../simapi/simdata.h'))
    simmap.code.append(C.blank())
    simmap.code.append(C.function('CreateSimDataMap', 'int',).add_param(C.variable('map', 'struct Map', pointer=1)).add_param(C.variable('simdatamap', 'SimDataMap', pointer=1)).add_param(C.variable('mapdata', 'int', pointer=0)))

body = C.block(innerIndent=4)

structname=""
structvar=""
mapnum = 0

typedeflist=[]
typedefdict={}

body.append(C.blank())

if (game == "RFactor2"):
    body.append(C.statement("char* rf2t = NULL"))
    body.append(C.statement("char* rf2s = NULL"))
    body.append("    if (mapdata == 1)") # not sure how to do this cleaner with this api
    iff = C.block(innerIndent=4)
    iff.append(C.statement("rf2t = rf2map->telemetry_map_addr"))
    iff.append(C.statement("rf2s = rf2map->scoring_map_addr"))
    body.append(iff)
if (game == "Assetto Corsa"):
    body.append(C.statement("char* spfp = NULL"))
    body.append(C.statement("char* spfg = NULL"))
    body.append(C.statement("char* spfs = NULL"))
    body.append(C.statement("char* spfc = NULL"))
    body.append("    if (mapdata == 1)") # not sure how to do this cleaner with this api
    iff = C.block(innerIndent=4)
    iff.append(C.statement("spfp = acmap->physics_map_addr"))
    iff.append(C.statement("spfg = acmap->graphic_map_addr"))
    iff.append(C.statement("spfs = acmap->static_map_addr"))
    iff.append(C.statement("spfc = acmap->crewchief_map_addr"))
    body.append(iff)
if (game == "simdata"):
    body.append(C.statement("char* s = NULL"))
    body.append("    if (mapdata == 1)") # not sure how to do this cleaner with this api
    iff = C.block(innerIndent=4)
    iff.append(C.statement("s = simdata"))
    body.append(iff)

body.append(C.blank())

instruct = False
intypedef = False
usestruct = True

def setstructname(g):
    global structvar
    global structname
    match g:
        case "Assetto Corsa":
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
        case "RFactor2":
            if ( structname == "rF2Scoring" ):
                structvar = "rf2s"
            elif ( structname == "rF2Telemetry" ):
                structvar = "rf2t"
            else:
                structname = ""
        case "simdata":
            if ( structname == "simdata" ):
                structvar = "s"
            else:
                structname = ""

def formatdatatype(d):
        if "int" in d:
            return "INTEGER"
        if "float" in d:
            return "FLOAT"
        if "double" in d:
            return "DOUBLE"
        return "CHAR"

def addcodeline(textstring):
    body.append(C.statement(textstring[0] + ".name = " + "\"" + textstring[2] + "\""))
    body.append(C.statement(textstring[0] + ".value = " + textstring[3]))
    body.append(C.statement(textstring[0] + ".dtype = " + formatdatatype(textstring[1])))

def addvarsfromtypedef(t, textstring):
    global mapnum
    newtextstring = {}
    if t in typedeflist:
        for key in typedefdict:
            if t in key:
                type2=typedefdict[key]
                if "[" in key.split(':')[1]:
                    # hopefully this is all the recursion i will need
                    # i could definitely tighten this up and implement further
                    # but i think this will suffice
                    continue
                else:
                    newtextstring[0] = "map["+str(mapnum)+"]"
                    newtextstring[1] = type2;
                    newtextstring[2] = textstring[2] + "_" + key.split(':')[1]
                    newtextstring[3] = textstring[3] + " + offsetof(" + key.split(':')[0] + ", " + key.split(':')[1] + ")"
                    if addvarsfromtypedef(type2, newtextstring):
                        continue
                    else:
                        addcodeline(newtextstring)
                        mapnum = mapnum + 1
        return True
    else:
        return False

def addline(v, t, b):
    global mapnum
    textstring = {}

    textstring[0] = "map["+str(mapnum)+"]"
    textstring[1] = t

    offsetstring = "offsetof(struct "
    if(b == False):
       offsetstring = "offsetof("
       

    if "]" in v:
        array_length=v[v.find("[")+1:v.find("]")]
        v=v[0:v.find("[")]
        for i in range(int(array_length)):
            textstring[0] = "map["+str(mapnum)+"]"
            textstring[2] = structname+"_"+ v + str(i)
            textstring[3] = "((char*) " + structvar + " + " + offsetstring + structname + ", " + v + ")) + (sizeof(" + type + ") * " + str(i) + ")"
            if addvarsfromtypedef(t, textstring):
                continue
            else:
                addcodeline(textstring)
                mapnum = mapnum + 1
    else:
        textstring[2] = structname + "_" + v
        textstring[3] = "((char*) " + structvar + " + " + offsetstring + structname + ", " + v + "))"
        if not addvarsfromtypedef(t, textstring):
            addcodeline(textstring)
            mapnum = mapnum + 1

with open(includefilename) as topo_file:
    for line in topo_file:
        if ("#" not in line and "{" not in line and "}" not in line and not line.lstrip().startswith("/") and line.lstrip() != '\r\n'):


            if ("typedef struct" in line and "simdata" not in line):
                typedefname = line.split('//')[1]
                typedefname = typedefname.strip()
                typedeflist.append(typedefname)
                usestruct = True
                instruct = False
                intypedef = True
                continue
            elif ("typedef struct //simdata" in line.lstrip()):
                usestruct = False
                instruct = True
                intypedef = False
                structname = "simdata"
                setstructname(game)

            elif ("struct" in line):
                usestruct = True
                instruct = True
                intypedef = False
                structname = line.split()[1]
                setstructname(game)
            elif (intypedef == True):
                if(line.lstrip()):
                    typename=line.split()[0]
                    varname=line.split()[1][0:-1]
                    if ("signed" in typename):
                        typename=line.split()[1]
                        varname=line.split()[2][0:-1]
                    typedefdict.update({typedefname + ":" + varname : typename})

            elif (line.isspace()):
                continue
            elif (structname == ""):
                continue
            else:
                rawline=line.split()
                variable=rawline[1][0:-1]
                type=rawline[0]
                addline(variable, type, usestruct)



#print(typedefdict)
#print(typedeflist)

body.append(C.blank())
body.append(C.statement('return 0'))
simmap.code.append(body)

with open(mapperfilename, "w") as f:
    f.write(str(simmap))

filedata=""
with open('../simmap/' + mapperheaderfilename) as topo_file:


    with open(mapperheaderfilename, 'w') as topo_file2:

        for line in topo_file:
            if mapsizevariablename in line:
                topo_file2.write(mapsizevariablename + "    " + str(mapnum) + "\n")
            else:
                topo_file2.write(line)


