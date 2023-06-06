open Core
module Unix = Core_unix

type t =
  { (* The file that the values are written to. *)
    file : Core_unix.File_descr.t;
    (* The head of the ValueLog. This is where the next value will be written. *)
    mutable head : int;
    (* The tail of the ValueLog. This is the position of the oldest write that hasn't been overwritten or deleted. *)
    mutable tail : int }

let access : string -> Unix.open_flag =
  fun path ->
   match Unix.access path [`Exists] with
   | Ok _ -> Unix.O_RDONLY
   | Error _ -> Unix.O_WRONLY
;;

(** Creates a new ValueLog or loads an existing one from disk.
    If the ValueLog file already exists, this function will only open the file without scanning it. 
    This function doesn't check if file has been corrupted. *)
let create : string -> int -> int -> t =
  fun path head tail ->
   let mode = [access path] in
       {file = Unix.openfile path ~mode; head; tail}
;;

(** Appends a new key-value pair to the ValueLog. 
    In the event that the write was unsuccessful, the ValueLog won't shift the head forward.
    Therefore, writes can be retried without the risk of polluting the ValueLog or corrupting past entries.*)
let append : t -> string -> int -> string -> int -> int =
  fun vlog key key_len value value_len ->
   let ret = Unix.lseek vlog.file (Int64.of_int vlog.head) ~mode:Unix.SEEK_SET
   and pos = vlog.head in
       if Int64.equal ret (Int64.of_int (-1)) then
         failwith "lseek"
       else begin
         if
           [|string_of_int key_len; string_of_int value_len; key; value|]
           |> Array.map ~f:Bytes.of_string
           |> Array.map ~f:(fun buf -> Unix.write vlog.file ~buf)
           |> Array.for_all ~f:(fun ret -> ret <> -1)
         then
           ()
         else
           failwith "write"
       end;
       vlog.head <- vlog.head + Int.num_bits + Int.num_bits + key_len + value_len;
       pos
;;

let read_key_len : Unix.File_descr.t -> int =
  fun file ->
   let buf = Bytes.create Int.num_bits in
       match Unix.read file ~len:Int.num_bits ~buf with
       | -1 -> failwith "read key len error"
       | _ -> Bytes.to_string buf |> int_of_string
;;

let read_value_len : Unix.File_descr.t -> int =
  fun file ->
   let buf = Bytes.create Int.num_bits in
       match Unix.read file ~len:Int.num_bits ~buf with
       | -1 -> failwith "read value len error"
       | _ -> Bytes.to_string buf |> int_of_string
;;

let read_key : Unix.File_descr.t -> int -> string =
  fun file key_len ->
   let buf = Bytes.create key_len in
       match Unix.read file ~len:key_len ~buf with
       | -1 -> failwith "read key len error"
       | _ -> Bytes.to_string buf
;;

let read_value : Unix.File_descr.t -> int -> string =
  fun file value_len ->
   let buf = Bytes.create value_len in
       match Unix.read file ~len:value_len ~buf with
       | -1 -> failwith "read key len error"
       | _ -> Bytes.to_string buf
;;

(** Fetches a value from the ValueLog at a given position. *)
let get : t -> int -> int * int * string * string =
  fun vlog loc ->
   let ret = Unix.lseek vlog.file (Int64.of_int loc) ~mode:Unix.SEEK_SET in
       if Int64.equal ret (Int64.of_int (-1)) then
         failwith "lseek"
       else begin
         let key_len = read_key_len vlog.file
         and value_len = read_value_len vlog.file in
         let key = read_key vlog.file key_len
         and value = read_value vlog.file value_len in
             key_len, value_len, key, value
       end
;;

(** Syncs the ValueLog to the disk.
    This function forcefully flushes the changes to the ValueLog to disk.
    Use this function sparingly as is will heavily reduce the write throughput of the ValueLog. *)
let sync : unit -> unit = fun () -> Unix.sync ()
