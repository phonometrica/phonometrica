# Author: Léa Courdès-Murphy
# Purpose: extract name of functions from the documentation, so that they can be used in autocompletion
# Arguments: input and output directories
import os
import re, sys

directory  = sys.argv[1]

lst_types = ["Array", "Boolean", "File", "Float", "Function", "Integer", "List", "Module", "Number", "Object", "Regex", "Set", "String", "Table"]
lst_mots_cles = ["assert", "break", "class", "continue", "debug", "do", "downto", "else", "elsif", "false", "field", "foreach", "function", "inherits", "json", "local", "method", "null", "option", "pass", "phon", "print", "repeat", "return", "step", "super", "then", "this", "throw", "true", "until", "while"]
lst_functions = []


# Expression régulière : ".. function:: NOM("
regex = re.compile(r"""
                   (\.\.\ function::\ )
                   (.*?)\(             #nom de la fonction
                   """, re.VERBOSE)

# Loop qui ouvre et lit chaque fichier au format .rst du directory
for rst_file in os.scandir(directory):
    if rst_file.path.endswith(".rst"):
        functions_file = open(rst_file,"r", encoding="utf-8")
        file = functions_file.read()
        file = file.replace('\\_', '_')
        # Trouve les matches de l'expression régulière dans le fichier .rst ouvert
        matches = re.findall(regex,file)
        # Loop qui inscrit chaque nouvelle fonction (+ 3 caractères) dans la liste des fonctions
        for match in matches:
            if match[1] not in lst_functions:
                if len(match[1]) > 3:
                    lst_functions.append(match[1])
    functions_file.close()


# Création d'une liste finale triée par ordre alphabétique (Maj et Min ensemble)
def clef_casse(s):
    return s.lower()

lst_finale = sorted(lst_types + lst_mots_cles + lst_functions, key=clef_casse)


# écriture dans un fichier "autocompletion_list.hpp"
path = os.path.join(sys.argv[2], "autocompletion_list.hpp")
new_file = open(path,"w", encoding="utf-8")
new_file.write("static const char *autocompletion_list =\n")    

# Loop qui permet d'écrire chaque nom de fonction dans le fichier .hpp
n=0

for i in lst_finale:
    print(i)
    if i==lst_finale[-1]:
        new_file.write(i + "\";")
    else:
        if n == 0:
            new_file.write("\t\"" + i + " ")
            n+= 1
        elif n == 9:
            new_file.write(i + " \"\n")
            n=0
        else:
            new_file.write(i + " ")
            n+=1
        
new_file.close()












