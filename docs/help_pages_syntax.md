# Catégories syntaxiques souhaitées

- Titre
- Section
- Modificateur(s) de police (gras, italique, souligné, couleur)
- tableau avec Modificateur de police par colonne
- liste d'entités syntaxiques séparées par des virgules
- description longue
- description courte

# Exemple de *help page* :

```
#title "CPF's build help page"

#set (
	"section", 
	{mod:[b#00FF00]}
)

#set (
	"title", 
	{mod:"U"}
)

#section "Usage"

#plain "des trucs"

#section "Options"

#array {
	"cols":(, "Description"),
	"mod":(b#0000FF, p),
	"data":(
		(("-a", "all"), "Build all"),
		(("-r", "release"), "Build executable target in release mode"),
		(("-q", "quiet"), "Do no print log messages"),
		(("-h", "help"), "Display the build help message and do nothing"),
		(("-L", "staticlib"), "Build library target as a static library"),
		(("-l", "dynlib"), "Build library as a dynamic library"),
		(("-d", "docs"), "Build documentation"),
		(("-t", "tests"), "Build tests"),
		(("-e", "examples"), "Build examples"),
		(("-C", ("config-file", "<FILE>")), "Specify a configuration file"),
	)
}
```

*Idée :* pouvoir définir des commandes :

```
#define {
	name:"section", 
	params:("mod"), 
	func:t -> `#plain {data:t":", mod:@mod}`}
}

#define {
	name:"par",
	params:("mod")
}
```

# Syntaxe des help pages

## Commandes

De la forme `"#"COMMAND`,
où `COMMAND -> "title" | "mod" | "section" | "array"`

## Data

`DATA -> LIST | DICT | STRING | MOD | MOD STRING | BOOL | epsilon`

## Booleans

`BOOL -> true | false`

## Strings

## Dictionnary

De la forme `DICT -> "{" SYMBOL ":" DATA DDEFS | "{" "}"`

où : `DDEFS -> "," SYMBOL ":" DATA DDEFS | "}" `

## Style modificator 

- b : bold 
- i : italic 
- u : underlined
- p : paragraph
- U : uppercase

`MOD -> *("b" | "i" | "u" | "p" | "U") 0*1("#"6(HEXA))`

## List

De la forme :
`L -> "(" DATA LP | "(" ")"`
`LP -> "," DATA LP | ")"`

