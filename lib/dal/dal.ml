(** Data Access Layer (DAL) handles all disk operations and how data is organized on the disk.
    It’s responsible for managing the underlying data structure,
    writing the database pages to the disk, and reclaiming free pages to avoid fragmentation. *)

open Utils
module Page = Page
module Page_num = Page_num
module Freelist = Freelist

type t =
  { file : Unix.File_descr.t;
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
  let offset = (page.num |> Page_num.to_int) * dal.page_size in
    Unix.read dal.file ~buf:page.data ~pos:offset |> ignore;
    page
;;

let write_page (dal : t) (page : Page.t) =
  let offset = page.num |> Page_num.to_int |> ( * ) dal.page_size in
    Unix.write dal.file ~buf:page.data ~pos:offset |> ignore
;;
