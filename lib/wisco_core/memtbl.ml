(*
 * Copyright 2022 Muqiu Han
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *)

open Core
open Wisco_utils

module Record = struct
  (** Each MemTableRecord holds the key and the position of the record in the ValueLog.
      [key] The key of the record
      [value_loc] the location of the key in the value log. *)
  type record =
    { key : string;
      value_loc : int }

  let compare (r : record) (c : record) : int = compare_string r.key c.key

  let to_string {key : string; value_loc : int} : string =
    Format.sprintf "{key = %s; value_loc: %d}" key value_loc
  ;;

  type t = record
end

include Skiplist.Make (Record)

(** In-memory table of the records that have been modified most recently.
    At any given time, there is only one active MemTable in the database engine.
    The MemTable is always the first store to be searched when a key-value pair is requested.
    [records] Array of the records sort by key *)
type memtbl = unit t

(** Creates a new empty memtbl. *)
let make () : memtbl = create ()
