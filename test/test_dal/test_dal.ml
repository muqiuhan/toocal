open Alcotest

let test_dal () =
  (* Initialize database *)
  let dal = Dal.make "db.db" (Dal.Page.system_page_size ()) in
  let page =
    Dal.Page.
      {num = Dal.Freelist.get_next_page dal.freelist; data = "data" |> Bytes.of_string}
  in
    Dal.write_page dal page
;;

let suit =
  ( "Toocal.Dal",
    [ test_case "test_dal" `Quick test_dal;
      test_case
        "test_system_memory_page"
        `Quick
        Test_system_memory_page_size.test_system_memory_page_size ] )
;;
