(** Databases organize data in pages.
    Pages are the smallest unit of data exchanged by the database and the disk.
    It’s convenient to have a unit of work of fixed size.
    Also, it makes sense to put related data in proximity so it can be fetched all at once. *)

open Utils

(** Reading and writing database pages will be managed by the DAL. *)
type t =
  { data : bytes;
    num : Page_num.t }

(** Get the system memory page size *)
external system_page_size : unit -> int = "__ocaml_system_memory_page_size"

(** This is the maximum pgnum that is used by the db for its own purposes.
    For now, only page 0 is used as the header page.
    It means all other page numbers can be used.*)
let meta = Page_num.of_int 0

let make (page_size : int) = {data = Bytes.create page_size; num = meta}
