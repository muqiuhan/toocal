(** Data Access Layer (DAL) handles all disk operations and how data is organized on the disk.
    It’s responsible for managing the underlying data structure,
    writing the database pages to the disk, and reclaiming free pages to avoid fragmentation. *)

open Utils
module Page = Page
module Page_num = Page_num
module Freelist = Freelist

type t =
  { file : Unix.file_descr;
    page_size : int;
    freelist : Freelist.t }

let make (path : string) (page_size : int) =
  { file = Unix.openfile ~mode:[Unix.O_CREAT; Unix.O_RDWR] ~perm:0666 path;
    page_size;
    freelist = Freelist.make () }
;;

let close (dal : t) = Unix.close dal.file

let read_page (dal : t) (num : Page_num.t) =
  let page = Page.make dal.page_size in
  let offset = (page.num |> Page_num.to_int) * dal.page_size
  and len = Bytes.length page.data in
    ExtUnix.All.pread dal.file offset page.data 0 len |> ignore;
    page
;;

let write_page (dal : t) (page : Page.t) =
  let offset = page.num |> Page_num.to_int |> ( * ) dal.page_size
  and data = page.data |> Bytes.to_string
  and len = Bytes.length page.data in
    ExtUnix.All.pwrite dal.file offset data 0 len |> ignore
;;
