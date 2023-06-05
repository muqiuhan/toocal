open Core

(** Max number of MemTableRecords in a MemTable *)
let max_size : int = 1024

(** Single Record in the MemTable *)
module Record = struct
  (**  Each MemTableRecord holds the key and the position of the record in the ValueLog. 
       key: The key of the record.
       key_len: The length of the key.
       value_loc: The location of the value in the ValueLog. *)
  type t =
    { key : string;
      key_len : int;
      value_loc : int64 }

  let empty = {key = ""; key_len = 0; value_loc = Int64.zero}

  module Cmp = struct
    let key : t -> string -> int =
      fun record key -> compare_string key record.key
    ;;
  end
end

(** MemTable of the Database. 
    memtbl_records: Array of records sorted by key.
    memtbl_size: The number of records filled in `records`. *)
type t =
  { memtbl_records : Record.t array;
    mutable memtbl_size : int }

(** Creates a new empty MemTable. *)
let make : unit -> t =
  fun () ->
   { memtbl_size = 0;
     memtbl_records =
       Array.create ~len:max_size Record.empty }
;;

let binary_search : t -> string -> int option =
  fun memtbl key ->
   Array.binary_search
     memtbl.memtbl_records
     `First_equal_to
     ~compare:Record.Cmp.key
     key
;;

let insert_point : t -> string -> int =
  fun memtbl key ->
   match
     Array.binary_search
       memtbl.memtbl_records
       `First_greater_than_or_equal_to
       ~compare:Record.Cmp.key
       key
   with
   | None -> 0
   | Some index -> index
;;

(** Gets a MemTableRecord from a MemTable by key. 
    This function will return None if none of the records in the MemTable.
    This function uses binary search for a runtime of O(log(n)). *)
let get : t -> string -> Record.t option =
  fun memtbl key ->
   Option.map (binary_search memtbl key) ~f:(fun index ->
     memtbl.memtbl_records.(index))
;;

(** Sets a key-value pair in a MemTable.
    This function uses binary search for a runtime of O(log(n)) *)
let set : t -> string -> int64 -> unit =
  fun memtbl key value_loc ->
   let insert_index = insert_point memtbl key in
       begin
         match binary_search memtbl key with
         | None ->
           Array.blit
             ~src:memtbl.memtbl_records
             ~src_pos:insert_index
             ~dst:memtbl.memtbl_records
             ~dst_pos:(insert_index + 1)
             ~len:(memtbl.memtbl_size - insert_index - 1);
           memtbl.memtbl_records.(insert_index)
             <- Record.
                  { key;
                    value_loc;
                    key_len = String.length key }
         | Some _ ->
           memtbl.memtbl_records.(insert_index)
             <- Record.
                  { key;
                    value_loc;
                    key_len = String.length key }
       end;
       memtbl.memtbl_size <- memtbl.memtbl_size + 1
;;

(** Deletes a record from a MemTable.
    This function uses binary search for a runtime of O(log(n)).
    Note: This function will set a tombstone to propagate the delete into the SSTables. *)
let delete : t -> string -> unit =
  fun memtbl key -> set memtbl key (Int64.of_int 1)
;;
