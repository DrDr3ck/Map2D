#!/usr/bin/python2.6
# coding: utf-8

import sys
import os.path
import re

class Translator:
    def __init__(self):
        pass
        
    def readFile(self, filename):
        lines = list()
        results = list()
        with open(filename) as f:
            content = f.readlines()
            for line in content:
                a,b = line.split("','")
                a = a[2:]
                b = b[:-3]
                lines.append(a)
                results.append(b)
        return lines, results
        
    def getAllTr(self, directory):
        lines = list()
        pattern = "[ ]tr[ ]*\([ ]*\"(.*)\"[ ]*\)";
        for filename in os.listdir(directory):
            if filename.endswith(".cpp") or filename.endswith(".h"): 
                #print(os.path.join(directory, filename))
                with open(filename) as f:
                    content = f.readlines()
                    for line in content:
                        matches = re.findall(pattern, line)
                        if len(matches) == 1:
                            lines.append(matches[0])
        return lines                   

if __name__ == "__main__":
    argc = len(sys.argv)
    
    filename = "language/fr.txt"
    if argc > 2:
        print "Error: need 1 argument (destination filename)"
        sys.exit()
    elif argc == 2:
        filename = sys.argv[1]
        
    print "Build translation for file", filename
    
    t = Translator()
    old_lines = list()
    results = list()
    if os.path.exists(filename):
        # read already existing sentences
        print("reading existing file")
        old_lines,results = t.readFile(filename)
    
    file = open(filename,"w")
    
    new_lines = t.getAllTr(".")
    
    for line in new_lines:
        index = -1
        if line in old_lines:
            index = old_lines.index(line)
        result = ""
        if index >= 0:
            result = results[index]
        final_line = "('" + line + "','"+result+"')\n"
        file.write(final_line)
    
    file.close();