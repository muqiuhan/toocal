open Wiscodb
open Core

let test_set () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      match memtbl.memtbl_records |> Array.to_list with
      | record :: _ ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_set_multi_elements () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      Memtbl.set memtbl "bbb" Int64.zero;
      Memtbl.set memtbl "ccc" Int64.zero;
      match memtbl.memtbl_records |> Array.to_list with
      | record1 :: record2 :: record3 :: _ ->
        Alcotest.(check string) "same key" "ccc" record1.key;
        Alcotest.(check int) "same key_len" 3 record1.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record1.value_loc;
        Alcotest.(check string) "same key" "bbb" record2.key;
        Alcotest.(check int) "same key_len" 3 record2.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record2.value_loc;
        Alcotest.(check string) "same key" "aaa" record3.key;
        Alcotest.(check int) "same key_len" 3 record3.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record3.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_set_overwrite () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      Memtbl.set memtbl "aaa" Int64.one;
      match memtbl.memtbl_records |> Array.to_list with
      | record1 :: record2 :: _ ->
        Alcotest.(check string) "same key" "aaa" record1.key;
        Alcotest.(check int) "same key_len" 3 record1.key_len;
        Alcotest.(check int64) "same value_loc" Int64.one record1.value_loc;
        Alcotest.(check string) "same key" "aaa" record2.key;
        Alcotest.(check int) "same key_len" 3 record2.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record2.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_get () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      match Memtbl.get memtbl "aaa" with
      | None -> Alcotest.fail "test_get"
      | Some record ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int64) "same value_loc" Int64.zero record.value_loc
;;

let test_get_same_key () =
  let memtbl = Memtbl.make () in
      Memtbl.set memtbl "aaa" Int64.zero;
      Memtbl.set memtbl "aaa" Int64.one;
      match Memtbl.get memtbl "aaa" with
      | None -> Alcotest.fail "test_get"
      | Some record ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int64) "same value_loc" Int64.one record.value_loc
;;

let tests =
  Alcotest.(
    ( "Memtbl",
      [ test_case "Memtbl.set" `Quick test_set;
        test_case "Memtbl.set muti elements" `Quick test_set_multi_elements;
        test_case "Memtbl.set overwrite" `Quick test_set_overwrite;
        test_case "Memtbl.get" `Quick test_get;
        test_case "Memtbl.get same key" `Quick test_get_same_key ] ))
;;
