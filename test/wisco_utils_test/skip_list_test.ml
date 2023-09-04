open Alcotest
module M = Wisco_utils.Skiplist

let test_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let actual = IntSList.is_empty sl in
  let expected = true in
      check bool "empty" expected actual
;;

let test_len_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let actual = IntSList.length sl in
  let expected = 0 in
      check int "empty" expected actual
;;

let test_len_insert () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:(n + 2) ~value:n;
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:(n + 1) ~value:n;
      IntSList.remove sl (n + 2);
      IntSList.add sl ~key:(n + 2) ~value:n;
      let actual = IntSList.length sl in
      let expected = 3 in
          check int "three" expected actual
;;

let test_len_remove () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:(n + 2) ~value:n;
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:(n + 1) ~value:n;
      IntSList.remove sl (n + 2);
      IntSList.remove sl n;
      let actual = IntSList.length sl in
      let expected = 1 in
          check int "one" expected actual
;;

let test_len_remove_all () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:(n + 2) ~value:n;
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:(n + 1) ~value:n;
      IntSList.remove sl (n + 2);
      IntSList.remove sl n;
      IntSList.remove sl (n + 1);
      let actual = IntSList.length sl in
      let expected = 0 in
          check int "empty" expected actual
;;

let test_insertion () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      let actual = IntSList.find sl n in
      let expected = Some n in
          check (option int) "same number" expected actual
;;

let test_insertion_update () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:n ~value:(n + 1);
      let actual = IntSList.find sl n in
      let expected = Some (n + 1) in
          check (option int) "same number" expected actual
;;

let test_find_finger_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
  let actual = IntSList.find_finger sl n in
  let expected = None in
      check (option int) "same number" expected actual
;;

let test_find_finger () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      let actual = IntSList.find_finger sl n in
      let expected = Some n in
          check (option int) "same number" expected actual
;;

let test_find_finger_ordered () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n1 = 13 in
  let n2 = n1 + 1 in
  let n3 = n2 + 1 in
  let actual = [IntSList.find_finger sl n1] in
      IntSList.add sl ~key:n1 ~value:n1;
      IntSList.add sl ~key:n2 ~value:n2;
      IntSList.add sl ~key:n3 ~value:n3;
      let actual = IntSList.find_finger sl n1 :: actual in
      let actual = IntSList.find_finger sl n3 :: actual in
      let expected = [None; Some n1; Some n3] in
          check (list (option int)) "same number" expected (List.rev actual)
;;

let test_find_finger_unordered () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n1 = 13 in
  let n2 = n1 + 1 in
  let n3 = n2 + 1 in
  let actual = [IntSList.find_finger sl n1] in
      IntSList.add sl ~key:n1 ~value:n1;
      IntSList.add sl ~key:n2 ~value:n2;
      IntSList.add sl ~key:n3 ~value:n3;
      let actual = IntSList.find_finger sl n3 :: actual in
      let actual = IntSList.find_finger sl n1 :: actual in
      let expected = [None; Some n3; Some n1] in
          check (list (option int)) "same number" expected (List.rev actual)
;;

let test_find_finger_unordered_none () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n1 = 0 in
  let n2 = 2 in
  let n3 = 4 in
      IntSList.add sl ~key:n1 ~value:n1;
      IntSList.add sl ~key:n2 ~value:n2;
      let actual = [IntSList.find_finger sl n3] in
      let actual = IntSList.find_finger sl n2 :: actual in
      let expected = [None; Some n2] in
          IntSList.to_string sl |> Printf.printf "%s";
          check (list (option int)) "same number" expected (List.rev actual)
;;

let test_find_finger_remove () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:0;
      let actual = [IntSList.find_finger sl n] in
          IntSList.remove sl n;
          let actual = IntSList.find_finger sl n :: actual in
              IntSList.add sl ~key:n ~value:0;
              let actual = IntSList.find_finger sl n :: actual in
              let expected = [Some 0; None; Some 0] in
                  check (list (option int)) "same number" expected (List.rev actual)
;;

let test_remove () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      IntSList.remove sl n;
      let actual = IntSList.find sl n in
      let expected = None in
          check (option int) "none" expected actual
;;

let test_find_none () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
  let actual = IntSList.find sl n in
  let expected = None in
      check (option int) "none" expected actual
;;

let test_find_range_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
  let actual = IntSList.find_range ~start:0 ~stop:n sl in
  let expected = [] in
      check (list (pair int int)) "empty" expected actual
;;

let test_find_range () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 6 in
      for i = 0 to n do
        IntSList.add sl ~key:i ~value:None
      done;
      let actual = IntSList.find_range ~start:1 ~stop:4 sl in
      let expected = [3, None; 2, None; 1, None] in
          check (list (pair int (option int))) "range" expected actual
;;

let test_find_nearest_none () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
  let actual = IntSList.find_nearest sl n in
  let expected = `Empty in
  let empty' =
    match actual, expected with
    | `Empty, `Empty -> true
    | _, _ -> false
in
      check bool "none" empty' true
;;

let test_find_nearest_eq () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      for i = 0 to n do
        IntSList.add sl ~key:i ~value:None
      done;
      let actual = IntSList.find_nearest sl 2 in
      let eq' =
        match actual with
        | `Eq (2, _) -> true
        | _ -> false
    in
          check bool "eq" eq' true
;;

let test_find_nearest_gt () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      for i = 0 to n do
        IntSList.add sl ~key:i ~value:None
      done;
      let actual = IntSList.find_nearest sl 22 in
      let expected = 13 in
      let gt' =
        match actual with
        | `Gt (x, _) -> x
        | _ -> -1
    in
          check int "gt" gt' expected
;;

let test_find_nearest_lt () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      for i = 4 to n do
        IntSList.add sl ~key:i ~value:None
      done;
      let actual = IntSList.find_nearest sl 2 in
      let expected = 4 in
      let lt' =
        match actual with
        | `Lt (x, _) -> x
        | _ -> -1
    in
          check int "lt" lt' expected
;;

let test_find_nearest_lt_in_range () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 32 in
      for i = 0 to n do
        IntSList.add sl ~key:(i * 2) ~value:None
      done;
      let actual = IntSList.find_nearest sl 3 in
      let expected = 4 in
      let lt' =
        match actual with
        | `Lt (x, _) -> x
        | _ -> -1
    in
          check int "lt range" expected lt'
;;

let test_mem_exists () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      let actual = IntSList.mem sl n in
      let expected = true in
          check bool "exist" expected actual
;;

let test_non_mem () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
  let actual = IntSList.mem sl n in
  let expected = false in
      check bool "missing" expected actual
;;

let test_list_transform () =
  let module IntSList = M.Make (Int) in
  let l = [7, None; 1, None; 9, None; 3, None; 5, None] in
  let sl = IntSList.of_alist l in
  let actual = IntSList.to_alist sl in
  let expected = [9, None; 7, None; 5, None; 3, None; 1, None] in
      check (list (pair int (option int))) "same list" expected actual
;;

let test_list_transform_empty () =
  let module IntSList = M.Make (Int) in
  let l = [] in
  let sl = IntSList.of_alist l in
  let actual = IntSList.to_alist sl in
  let expected = [] in
      check (list (pair int int)) "empty list" expected actual
;;

let test_min () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:(n + 1) ~value:n;
      IntSList.add sl ~key:n ~value:n;
      let actual = IntSList.min sl in
      let expected = Some (n, n) in
          check (option (pair int int)) "min number" expected actual
;;

let test_max () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:(n - 1) ~value:n;
      let actual = IntSList.max sl in
      let expected = Some (n, n) in
          check (option (pair int int)) "max number" expected actual
;;

let test_min_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      IntSList.remove sl n;
      let actual = IntSList.min sl in
      let expected = None in
          check (option (pair int int)) "none" expected actual
;;

let test_max_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:n ~value:n;
      IntSList.remove sl n;
      IntSList.remove sl n;
      let actual = IntSList.max sl in
      let expected = None in
          check (option (pair int int)) "none" expected actual
;;

let test_copy_empty () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let sl' = IntSList.copy sl in
  let actual = IntSList.to_alist sl in
  let expected = IntSList.to_alist sl' in
      check (list (pair int int)) "empty list" expected actual
;;

let test_copy () =
  let module IntSList = M.Make (Int) in
  let sl = IntSList.create () in
  let n = 13 in
      IntSList.add sl ~key:(n + 2) ~value:n;
      IntSList.add sl ~key:n ~value:n;
      IntSList.add sl ~key:(n + 1) ~value:n;
      let sl' = IntSList.copy sl in
      let actual = IntSList.to_alist sl in
      let expected = IntSList.to_alist sl' in
          check (list (pair int int)) "same list" expected actual
;;

let test_seq_transform () =
  let module IntSList = M.Make (Int) in
  let l = [7, None; 1, None; 9, None; 3, None; 5, None] in
  let actual = List.to_seq l |> IntSList.of_seq |> IntSList.to_seq |> List.of_seq in
  let expected = [1, None; 3, None; 5, None; 7, None; 9, None] in
      check (list (pair int (option int))) "same list" expected actual
;;

let suite =
  ( "Wisco_utils.Skiplist",
    [ "empty list", `Quick, test_empty; "empty list with length", `Quick, test_len_empty;
      "length of list", `Quick, test_len_insert;
      "length of list remove", `Quick, test_len_remove;
      "length of list remove all", `Quick, test_len_remove_all;
      "can insert element", `Quick, test_insertion;
      "can update element", `Quick, test_insertion_update;
      "can remove element", `Quick, test_remove; "find on empty", `Quick, test_find_none;
      "find range on empty", `Quick, test_find_range_empty;
      "find range", `Quick, test_find_range;
      "find nearest on empty", `Quick, test_find_nearest_none;
      "find nearest exist", `Quick, test_find_nearest_eq;
      "find nearest lt", `Quick, test_find_nearest_lt;
      "find nearest gt", `Quick, test_find_nearest_gt;
      "find nearest in range returns gt", `Quick, test_find_nearest_lt_in_range;
      "find with finger", `Quick, test_find_finger;
      "find with finger empty", `Quick, test_find_finger_empty;
      "find with finger readd", `Quick, test_find_finger_remove;
      "find with finger ordered", `Quick, test_find_finger_ordered;
      "find with finger unordered", `Quick, test_find_finger_unordered;
      "find with finger unordered non exist", `Quick, test_find_finger_unordered_none;
      "mem exists", `Quick, test_mem_exists; "not a member", `Quick, test_non_mem;
      "list transformations", `Quick, test_list_transform;
      "list transformations on empty", `Quick, test_list_transform_empty;
      "min element", `Quick, test_min; "max element", `Quick, test_max;
      "min element empty", `Quick, test_min_empty;
      "max element empty", `Quick, test_max_empty;
      "can copy empty", `Quick, test_copy_empty; "can copy", `Quick, test_copy;
      "sequence transformations", `Quick, test_seq_transform ] )
;;
