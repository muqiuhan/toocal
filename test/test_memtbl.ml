open Wiscodb
open Core

let test_set () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 9;
      match memtbl.memtbl_records |> Array.to_list with
      | record :: _ ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int) "same value_loc" 0 record.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_set_multi_elements () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      Memtbl.set memtbl "bbb" 0;
      Memtbl.set memtbl "ccc" 0;
      match memtbl.memtbl_records |> Array.to_list with
      | record1 :: record2 :: record3 :: _ ->
        Alcotest.(check string) "same key" "ccc" record1.key;
        Alcotest.(check int) "same key_len" 3 record1.key_len;
        Alcotest.(check int) "same value_loc" 0 record1.value_loc;
        Alcotest.(check string) "same key" "bbb" record2.key;
        Alcotest.(check int) "same key_len" 3 record2.key_len;
        Alcotest.(check int) "same value_loc" 0 record2.value_loc;
        Alcotest.(check string) "same key" "aaa" record3.key;
        Alcotest.(check int) "same key_len" 3 record3.key_len;
        Alcotest.(check int) "same value_loc" 0 record3.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_set_overwrite () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      Memtbl.set memtbl "aaa" 1;
      match memtbl.memtbl_records |> Array.to_list with
      | record1 :: record2 :: _ ->
        Alcotest.(check string) "same key" "aaa" record1.key;
        Alcotest.(check int) "same key_len" 3 record1.key_len;
        Alcotest.(check int) "same value_loc" 1 record1.value_loc;
        Alcotest.(check string) "same key" "aaa" record2.key;
        Alcotest.(check int) "same key_len" 3 record2.key_len;
        Alcotest.(check int) "same value_loc" 0 record2.value_loc
      | _ -> Alcotest.fail "test_set"
;;

let test_get () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      match Memtbl.get memtbl "aaa" with
      | None -> Alcotest.fail "test_get"
      | Some record ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int) "same value_loc" 0 record.value_loc
;;

let test_get_not_exists () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      match Memtbl.get memtbl "bbb" with
      | None -> ()
      | Some _ -> Alcotest.fail "test_get_not_exists"
;;

let test_get_same_key () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      Memtbl.set memtbl "aaa" 1;
      match Memtbl.get memtbl "aaa" with
      | None -> Alcotest.fail "test_get"
      | Some record ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int) "same value_loc" 1 record.value_loc
;;

let test_delete () =
  let memtbl = Memtbl.create () in
      Memtbl.set memtbl "aaa" 0;
      Memtbl.delete memtbl "aaa";
      match Memtbl.get memtbl "aaa" with
      | None -> Alcotest.fail "test_get"
      | Some record ->
        Alcotest.(check string) "same key" "aaa" record.key;
        Alcotest.(check int) "same key_len" 3 record.key_len;
        Alcotest.(check int) "same value_loc" (-1) record.value_loc
;;

let tests =
  Alcotest.(
    ( "Memtbl",
      [ test_case "Memtbl.set" `Quick test_set;
        test_case "Memtbl.set muti elements" `Quick test_set_multi_elements;
        test_case "Memtbl.set overwrite" `Quick test_set_overwrite;
        test_case "Memtbl.get" `Quick test_get;
        test_case "Memtbl.get same key" `Quick test_get_same_key;
        test_case "Memtbl.get not exists" `Quick test_get_not_exists;
        test_case "Memtbl.delete" `Quick test_delete ] ))
;;
