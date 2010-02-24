#! /usr/bin/env python

import os, fnmatch, sys

# globals
valid_plexil_home = 0
PLEXIL_HOME = ""
UE_TEST_DIR = ""
LUV_DIR = ""
DEBUG_FLAGS_TEMP = ""
COMPLETE_DEBUG = ""
DEBUG_DEFINITIONS = ""
COMMENT_FILE = ""

################ start of function definitions ###########################

# validate PLEXIL_HOME exists
def check_env_variables():
    for a in os.environ.keys():
        if a == "PLEXIL_HOME":
            valid_plexil_home = 1
            global PLEXIL_HOME
            PLEXIL_HOME = os.environ[a]
            global UE_TEST_DIR
            UE_TEST_DIR = PLEXIL_HOME + "/src/utils/test/"
            global LUV_DIR
            LUV_DIR = PLEXIL_HOME + "/luv/resources/"
            global DEBUG_FLAGS_TEMP
            DEBUG_FLAGS_TEMP = UE_TEST_DIR + "DebugFlags_temp"
            global COMPLETE_DEBUG
            COMPLETE_DEBUG = UE_TEST_DIR + "CompleteDebugFlags.cfg"
            global DEBUG_DEFINITIONS
            DEBUG_DEFINITIONS = UE_TEST_DIR + "DebugFlagDefinitions.txt"
            global COMMENT_FILE
            COMMENT_FILE = UE_TEST_DIR + "DebugFlagComments.txt"
            
    return valid_plexil_home            

# search through UE_HOME and save debug messages
def locate_debug_msgs(FILE_TYPE, PATTERN):

    output = open(DEBUG_FLAGS_TEMP + ".cfg", "a")
    
    still_looking = 0
    start_defining = 0
    
    for path, dirs, files in os.walk(os.path.abspath(PLEXIL_HOME)):
        for filename in fnmatch.filter(files, FILE_TYPE):
            filename = os.path.join(path, filename)
            count_line = 0
            defn = "DEF="
            input = open(filename, "r")
            #print filename
            for line in input:
                count_line = count_line + 1
                if still_looking or (PATTERN in line and not ("#define" in line or "debugMsg()" in line or "@see" in line)):
                    start_defining = 1
                    if "\"" in line:
                        src = "SRC=" + filename + " : %d" % (count_line, )
                        msg = line.strip()
                        still_looking = 0
                        output.write(src + "\n")
                        output.write("MSG=" + msg + "\n")                       
                    else:
                        still_looking = 1
                if start_defining:
                    line = line.strip()
                    defn = defn + line
                if start_defining and ";" in line:
                    output.write(defn + "\n")
                    defn = "DEF="
                    start_defining = 0
    output.close()
    
# extract debug tags from debug messages
def extract_tags():

    input = open(DEBUG_FLAGS_TEMP + ".cfg", "r")
    output = open(DEBUG_FLAGS_TEMP + "1.cfg", "w")
    src = ""
    defn = ""
    
    for line in input:
        if "SRC=" in line:
            src = line
            src = src.strip()
        elif "MSG=" in line:
            index = line.find("\"") + 1
            line = ":" + line[index:]
            index = line.find("\"")
            msg = line[0:index]       
            msg = msg.strip()  
        elif "DEF=" in line:
            defn = line
            defn = defn.strip()
            output.write("MSG=" + msg + src + defn)
            output.write("\n")                  
       
    output.close()    
	                                
# alphabetically sort debug tags in temp file 
def sort_tags_alphabetically():  
    input = open(DEBUG_FLAGS_TEMP + "1.cfg", "r")
    output = open(DEBUG_FLAGS_TEMP + "2.cfg", "w")  
     
    lines = input.readlines()
    lines.sort()
    map(output.write, lines)
    output.close()
    
# remove duplicate debug tags in temp file
def remove_duplicate_flags():

    input = open(DEBUG_FLAGS_TEMP + "2.cfg", "r")
    output1 = open(COMPLETE_DEBUG, "w")
    output2 = open(DEBUG_DEFINITIONS, "w")
    src = ""
    defn = ""
    count = 0
    sub_count = 0
    
    output1.write("# This file contains (ostensibly) all debug output tags used as of 2/17/2009.\n")
    output1.write("# Every tag has been commented out with the '#' character.\n")
    output1.write("# You can use this file by referencing it and/or copying it to your working\n")
    output1.write("# directory and uncommenting the desired debug tags.\n\n")
    
    output2.write("# This file contains a list of debug tags followed by a source and a description.\n")
    output2.write("# Each debug tag in the list is represented by the following elements:\n\n")
    output2.write("#     #MSG=DEBUG TAG\n")
    output2.write("#     COMMENT=description of debug tag\n")
    output2.write("#     #:##SRC=SOURCE FILE : LINE NUMBER\n")
    output2.write("#     #:##DEF=SOURCE CODE where DEBUG TAG was found\n\n")
    output2.write("# For example:\n\n")
    output2.write("#     68MSG=:Node:checkConditions\n")
    output2.write("#     COMMENT=Notification of checking condition change or (possible) transition for a node.\n")
    output2.write("#     68:1SRC=/home/laurel/plexil/universal-exec/Exec/base/Node.cc : 1243\n")
    output2.write("#     68:1DEF=debugMsg(\"Node:checkConditions\",\"Checking condition change for node \" << m_nodeId.toString());\n")
    output2.write("#     68:2SRC=/home/laurel/plexil/universal-exec/Exec/base/Node.cc : 1246\n")
    output2.write("#     68:2DEF=debugMsg(\"Node:checkConditions\",\"Can (possibly) transition to \" << toState.toString());\n\n")	
    output2.write("# ':Node:checkConditions' is the 68th debug tag in the list and was found in\n")
    output2.write("# two separate locations, thus the need for 68:1 and 68:2 SRC and DEF.\n\n")
 
    unique_line = ""  
    for line in input:
        msg_start = line.find("MSG=") + 4
        src_start = line.find("SRC=")
        def_start = line.find("DEF=")
        src = line[src_start:def_start]
        defn = line[def_start:]
        line = line[msg_start:src_start]       
        
        if line != unique_line:  
            count = count + 1
            sub_count = 0 
            
            comment = getCommentForTag(line)
            output1.write("#" + line + "\n")
            output2.write("\n%dMSG=" % (count, ) + line + "\n")
            output2.write(comment)
            unique_line = line            
       
        sub_count = sub_count + 1 
        src = "%d:%d" % (count,sub_count, ) + src + "\n"
        if sub_count > 9:
            defn = "%d:%d" % (count,sub_count, ) + defn
        else: 
            defn = "%d:%d" % (count,sub_count, ) + defn 
        output2.write(src)
        output2.write(defn)            
            
    output1.close()
    output2.close()
    
# retrieve comment for specific debug tag
def getCommentForTag(tag):
    input = open(COMMENT_FILE, "r") 
    found = 0
    comment = ""
    
    for line in input:
        if found and "COMMENT=" in line:
            # collecting comments
            comment = comment + line
        elif tag in line:
            # found tag
            comment = ""
            found = 1
        elif found and "MSG=" in line:
            break

    return comment

# delete temp files
def delete_temp_files():
    os.remove(DEBUG_FLAGS_TEMP + ".cfg")
    os.remove(DEBUG_FLAGS_TEMP + "1.cfg")
    os.remove(DEBUG_FLAGS_TEMP + "2.cfg")
    
################ end of function definitions #############################

################## entry point of script #################################

valid_plexil_home = check_env_variables()

if valid_plexil_home:
    if os.path.exists(DEBUG_DEFINITIONS):
        os.remove(DEBUG_DEFINITIONS)
    if os.path.exists(COMPLETE_DEBUG):
        os.remove(COMPLETE_DEBUG)

    print "\nSearching for debug messages in .cc, .cpp, .hh and .h files...\n\n"
    
    locate_debug_msgs(FILE_TYPE="*.cc", PATTERN="debugMsg(")
    locate_debug_msgs(FILE_TYPE="*.cpp", PATTERN="debugMsg(")
    locate_debug_msgs(FILE_TYPE="*.hh", PATTERN="debugMsg(")
    locate_debug_msgs(FILE_TYPE="*.ch", PATTERN="debugMsg(")
    
    locate_debug_msgs(FILE_TYPE="*.cc", PATTERN="condDebugMsg(")
    locate_debug_msgs(FILE_TYPE="*.cpp", PATTERN="condDebugMsg(")
    locate_debug_msgs(FILE_TYPE="*.hh", PATTERN="condDebugMsg(")
    locate_debug_msgs(FILE_TYPE="*.ch", PATTERN="condDebugMsg(")
    
    locate_debug_msgs(FILE_TYPE="*.cc", PATTERN="debugStmt(")
    locate_debug_msgs(FILE_TYPE="*.cpp", PATTERN="debugStmt(")
    locate_debug_msgs(FILE_TYPE="*.hh", PATTERN="debugStmt(")
    locate_debug_msgs(FILE_TYPE="*.ch", PATTERN="debugStmt(")
    
    locate_debug_msgs(FILE_TYPE="*.cc", PATTERN="condDebugStmt(")
    locate_debug_msgs(FILE_TYPE="*.cpp", PATTERN="condDebugStmt(")
    locate_debug_msgs(FILE_TYPE="*.hh", PATTERN="condDebugStmt(")
    locate_debug_msgs(FILE_TYPE="*.ch", PATTERN="condDebugStmt(")

    extract_tags()
    sort_tags_alphabetically()
    remove_duplicate_flags()
    delete_temp_files()
    print "Created a complete debug flag file:"
    print COMPLETE_DEBUG + "\n"
    print "Created a debug flag definition file:"
    print DEBUG_DEFINITIONS + "\n"
else:
    print "\nFAIL: PLEXIL_HOME was not set and so could not search for debug messages"
    


 




    


