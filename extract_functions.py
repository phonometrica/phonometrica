# Author: Léa Courdès-Murphy
# Purpose: extract function declarations from the documentation, so that they can be used in call tips
# Arguments: input and output directories
import os
import re, sys

directory  = sys.argv[1]


# Expression régulière : ".. function:: NOM(ARGUMENTS)\n\nDESCRIPTION."
regex = re.compile(r"""
                   \.\.\ function::\ 
                   (.*?)             #nom de la fonction
                   (\(.*?\))\n       #arguments entre parenthèses
                   \n
                   (.*?(\n.*?)?\.)   #description de la fonction (attention éventuellement sur deux lignes)
                   """, re.VERBOSE)


# dictionnaire : key = NOM DE LA FONCTION ; value -> liste où chaque élément = (ARGUMENTS)\nDESCRIPTION
dictionnaire = {}


# Loop qui ouvre et lit chaque fichier au format .rst du directory
for rst_file in os.scandir(directory):
    if rst_file.path.endswith(".rst"):
        functions_file = open(rst_file,"r", encoding="utf-8")
        file = functions_file.read()
        file = file.replace('"', "'")
        file = file.replace('``', '`')
        file = file.replace('\\_', '_')
        # Trouve les matches de l'expression régulière dans le fichier .rst ouvert
        matches = re.findall(regex,file)
        # Loop qui pour chaque match SOIT créé une entrée dans "dictionnaire" (nouvelle fonction) SOIT ajoute un élément dans la liste (value) (fonction déjà existante)
        for match in matches:
            description = match[2].replace('\n\n', '\n')
            description = description.replace("\n", "\\n")
            description = description.replace(" (:math:`\chi^2`)", "")
            
            if match[0] in dictionnaire:
                dictionnaire[match[0]].append(match[1]+"\\n"+ description)
            else:
                dictionnaire[match[0]] = [match[1]+"\\n"+ description]
    functions_file.close()

# écriture dans un fichier "function_declarations.hpp"
path = os.path.join(sys.argv[2], "function_declarations.hpp")
new_file = open(path,"w", encoding="utf-8")
new_file.write("static std::vector<std::pair<const char*, std::vector<wxString>>> function_declarations = {\n")    

longueur_dictionnaire = len(dictionnaire)
n=0

# Loop qui permet d'écrire dans le fichier .hpp pour chaque entrée du dictionnaire
for key, value in dictionnaire.items():
    new_file.write("\t{ \"" + key + "\",  {\n")
    # Si fonction a 1 description
    if len(value)== 1:
        new_file.write("\t\t\"" + key + value[0] + "\"\n")
    # Si fonction a 2 descriptions
    elif len(value)==2:
        new_file.write("\t\t\"" + key + value[0] + "\\002\",\n")
        new_file.write("\t\t\"" + key + value[1] + "\\001\"\n")
    # Si fonction a 3+ descriptions
    else:
        new_file.write("\t\t\"" + key + value[0] + "\\002\",\n")
        for i in range (1,len(value)-1):
            new_file.write("\t\t\"" + key + value[i] + "\\001\\002\",\n")
        new_file.write("\t\t\"" + key + value[-1] + "\\001\"\n")

    # Permet de séparer les fonctions par "}}," et d'omettre la virgule pour la dernière fonction écrite dans le .txt
    n+=1
    if n==longueur_dictionnaire:
        new_file.write("\t}}\n")
    else:
        new_file.write("\t}},\n")


new_file.write("};")
new_file.close()

