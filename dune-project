(lang dune 3.8)

(name toocal)

(generate_opam_files true)

(source
 (github muqiuhan/toocal))

(authors "Muqiu Han")

(maintainers "Muqiu Han")

(license MIT)

(documentation https://github.com/muqiuhan/toocal/)

(package
 (name toocal)
 (synopsis "A simple, persistent key/value store")
 (description "A simple, persistent key/value store")
 (depends
  ocaml
  dune
  core
  simlog
  stdint
  stdint-literals
  alcotest
  extunix)
 (tags
  (database "key value")))

; See the complete stanza docs at https://dune.readthedocs.io/en/stable/dune-files.html#dune-project
