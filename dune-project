(lang dune 3.20)

(name cpf)

(generate_opam_files true)

(source
 (github BastienGarnier/cpf))

(authors "Bastien Garnier <bastiengarnier17@proton.me>")

(maintainers "Bastien Garnier <bastiengarnier17@proton.me>")

(license MIT)

(version 0.1)

(documentation https://url/to/documentation)

(package
 (name cpf)
 (synopsis "The C/C++ Programmer's Friend")
 (description "C/C++ projects and packages manager")
 (depends ocaml)
 (tags
  ("C" "C++" cpf)))

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/reference/dune-project/index.html
