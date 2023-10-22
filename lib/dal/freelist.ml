(** This component is part of the DAL. *)

open Utils

(** Manages the manages free and used pages.
    [max_page]: holds the latest page num allocated.
    [released_pages]: holds all the ids that were released during delete
    [max_page] is incremented and a new page is created thus increasing the file size. *)
type t =
  { mutable max_page : Page_num.t;
    mutable released_pages : Page_num.t array }

let make () = {max_page = Page.meta; released_pages = [||]}

(** If possible, fetch pages first from the released pages. Else, increase the maximum page *)
let get_next_page (freelist : t) =
  if Array.length freelist.released_pages <> 0 then (
    let page_id = freelist.released_pages |> Array.last in
      freelist.released_pages
      <- freelist.released_pages
         |> Array.sub ~pos:0 ~len:(Array.length freelist.released_pages - 1);
      page_id
  ) else (
    freelist.max_page <- Page_num.(freelist.max_page + 1I);
    freelist.max_page
  )
;;

let release_page (freelist : t) (page : Page_num.t) =
  freelist.released_pages <- Array.append freelist.released_pages [|page|]
;;
