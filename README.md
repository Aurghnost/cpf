# C Programmer's Friend

C projects and packages manager fully written in C from scratch.

# Goals: 

- Creation of C projects with: 
	+ optimized build using static analysis of the modified code dependencies to compile only what is necessary 
- Static/dynamic analysis tools: 
	+ dead code 
	+ type checking using symbolic execution 
	+ Test generation according to formal specification 
- management of C packages database: 
	+ Build/add new libraries (headers, lib file) 
	+ versioning + dependencies between libraries 
	+ documentation (?) 
	+ library import (with dependencies) into a project directory

# Done 

- creation of C projects command line
- syntactical analysis library

# TODO :

## Containers

- Optimiser la structure hashset en utilisant un ArenaAllocator maison via mmap puisqu'on sait que les objets font tous la même taille

## Parser

- construire un DFA par expressions régulières au format texte
- Finir l'implémentation de l'analyseur syntaxique LR(1)
- Étendre la grammaire à LR(1) par IELR
- Sémantique d'arbre *somehow*

## Command Line

- définir un langage pour l'affichage de help pages (grâce au parser)
- écrire les help pages

## Build

- écrire un module C pour configurer et exécuter un *build* et remplacer le *Makefile*

## Outils de dev.

- écrire un analyseur statique (*so simple*)
