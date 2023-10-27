open Alcotest

let test_write_and_read_page () =
  let dal = Dal.make "db.db" (Dal.Page.system_page_size ()) in
  let write_page =
    Dal.Page.
      {num = Dal.Freelist.get_next_page dal.freelist; data = "data" |> Bytes.of_string}
  in
    Dal.write_page dal write_page;
    let read_page = Dal.read_page dal dal.freelist.max_page in
      (check bytes) "same data" write_page.data (Bytes.sub read_page.data 0 4)
;;

let suit =
  ( "Toocal.Dal",
    [ test_case "Test write and read page" `Quick test_write_and_read_page;
      test_case
        "Test get system memory page size"
        `Quick
        Test_system_memory_page_size.test_system_memory_page_size ] )
;;
