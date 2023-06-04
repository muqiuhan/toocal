open Stdint

val max_size : uint64

(** Single Record in the MemTable *)
module type Record = sig
  (**  Each MemTableRecord holds the key and the position of the record in the ValueLog. 
       key: The key of the record.
       key_len: The length of the key.
       value_loc: The location of the value in the ValueLog. *)
  type t =
    { key : string
    ; key_len : uint64
    ; value_loc : int64
    }
end

(** MemTable of the Database. 
    memtbl_records: Array of records sorted by key.
    memtbl_size: The number of records filled in `records`. *)
type t

(** Creates a new empty MemTable. *)
val make : unit -> t

(** Gets a MemTableRecord from a MemTable by key. 
    This function will return None if none of the records in the MemTable.
    This function uses binary search for a runtime of O(log(n)). *)
val get : t -> string -> uint64

(** Deletes a record from a MemTable.
    This function uses binary search for a runtime of O(log(n)).
    Note: This function will set a tombstone to propagate the delete into the SSTables. *)
val delete : t -> string -> uint64

(** Sets a key-value pair in a MemTable.
    This function uses binary search for a runtime of O(log(n)) *)
val set : t -> string -> uint64 -> int64
