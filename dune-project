(lang dune 3.7)

(name wiscodb)

(generate_opam_files true)

(source
 (github muqiuhan/wiscodb))

(authors "Muqiu Han")

(maintainers "Muqiu Han")

(license MIT)

(documentation https://github.com/muqiuhan/wiscodb)

(package
 (name wiscodb)
 (synopsis "WiscKey-based key-value database written in OCaml")
 (description "WiscKey-based key-value database written in OCaml")
 (depends ocaml core stdint alcotest)
 (tags
  (database "wisc key" wisckey)))

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/dune-files.html#dune-project
