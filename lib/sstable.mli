(** On-disk String-Sorted Table(SSTable) of the keys.
    On-disk storage of the keys and their value locations.
    Records in a SSTable are sorted by their key as to support
    binary search. If a record isn't found in the MemTable,
    then the database searches the SSTables starting with the
    lowest one in the hierarchy.  *)

(** Minimum number of records in a SSTable index array. *)
val min_size : int

(** Single Record in a SSTable.
    Each SSTableRecord holds the key and the position of the record in the ValueLog. *)
module Record : sig
  type t =
    { (* Key of the record. *)
      key : string;
      (* Length of they key. *)
      key_len : int;
      (* Location of the value in the ValueLog.  *)
      value_loc : int }
end

(** *)
type t =
  { (* Path of the SSTable on-disk. *)
    path : string;
    (* Creation timestamp in microseconds. *)
    timestamp : float;
    (* Compaction level. *)
    level : compation_level;
    (* File that the keys reside on. *)
    file : Core_unix.File_descr.t;
    (* In-memory index of the location of the keys.
       Growable array with size and capacity. Uses 8 bytes per key. *)
    records : int;
    (* Capacity of the growable in-memory index. *)
    capacity : int;
    (* Size of the growable in-memory index. *)
    size : int;
    (* Lowest key in the SSTable. Used to check if a key could possibly be in this SSTable. *)
    low_key : string;
    low_key_len : int;
    (* Highest key in the SSTable. Used to check if a key could possibly be in this SSTable. *)
    high_key : string;
    high_key_len : int }

and compation_level = |

(** @brief Parses the creation timestamp in microseconds from a SSTable filename. *)
val parse_timestamp : string -> float

(** @brief Parses the compaction level from a SSTable filename. *)
val parse_compaction_level : string -> compation_level

(** @brief Loads a SSTable at a path.
    This operation scans the entire SSTable to build the index. *)
val create : string -> t

(** @brief Creates a new SSTable from a full MemTable. 
    @note This function will create a new SSTable at a path.
           If a file already exists at this path, then the file will be overwritten. *)
val create_from_memtbl : string -> Memtbl.t -> t

(** @brief Gets the location of a value on the ValueLog from a key.
    @note This function uses the in-memory index to seek each record on disk.
           This function uses binary search for a runtime of O(log(n)) seeks. 

    @return This function returns the position in the ValueLog if the key is found.
             -2 if the key is not in the SSTable.
             -1 if there is an error reading the record. *)
val get_value_loc : t -> string -> int

(** @brief Checks if the given key could be in this SSTable.
    @note This function runs in constant time without any operations on disk. 

    @return This function returns 1 if the key is in the range and 0 if it is not. *)
val in_key_ranges : t -> int -> int
