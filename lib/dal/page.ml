(** Reading and writing database pages will be managed by the DAL. *)

open Utils
module Num = Uint64

type t =
  { data : bytes;
    num : Num.t }

let make (page_size : int) = {data = Bytes.create page_size; num = 0I}
