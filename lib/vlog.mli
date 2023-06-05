(** Value Log of the Database. 
    The Value Log stores the values in the database in the order that they were written. 
    New entries are written to the head of the file. To remove values that have been overwritten or deleted *)

type t =
  { (* The file that the values are written to. *)
    file : Core_unix.File_descr.t;
    (* The head of the ValueLog. This is where the next value will be written. *)
    head : int;
    (* The tail of the ValueLog. This is the position of the oldest write that hasn't been overwritten or deleted. *)
    tail : int }

(** Creates a new ValueLog or loads an existing one from disk.
    If the ValueLog file already exists, this function will only open the file without scanning it. 
    This function doesn't check if file has been corrupted. *)
val create : string -> int -> int -> t

(** Appends a new key-value pair to the ValueLog. 
    In the event that the write was unsuccessful, the ValueLog won't shift the head forward.
    Therefore, writes can be retried without the risk of polluting the ValueLog or corrupting past entries.*)
val append : t -> int -> string -> string -> int

(** Fetches a value from the ValueLog at a given position. *)
val get : t -> string -> int -> int

(** Syncs the ValueLog to the disk.
    This function forcefully flushes the changes to the ValueLog to disk.
    Use this function sparingly as is will heavily reduce the write throughput of the ValueLog. *)
val sync : t -> unit
