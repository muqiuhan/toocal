open Alcotest

let test_system_memory_page_size () =
  (check int) "same int" 4096 (Dal.Page.system_page_size ())
;;
