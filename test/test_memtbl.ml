open Wiscodb

let test_set () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      match memtbl.memtbl_records with
      | [|record|] ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int64)
          "same value_loc"
          Int64.zero
          record.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let tests =
  Alcotest.(
    "Memtbl", [test_case "Memtbl.set" `Quick test_set])
;;
