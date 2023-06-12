module Unix = struct
  include Core_unix
end

module Array_list = struct
  open Core

  type 'a t =
    { mutable size : int;
      mutable mod_count : int;
      mutable element_data : 'a array }

  let out_of_bound_msg al i =
    "Index: " ^ string_of_int i ^ ", Size: " ^ string_of_int al.size
  ;;

  exception Out_of_bounds of string

  external unsafe_sub : 'a array -> int -> int -> 'a array = "caml_array_sub"
  external unsafe_set : 'a array -> int -> 'b -> unit = "%array_unsafe_set"

  let unsafe_copy a = unsafe_sub a 0

  let make capacity default =
    {size = 0; element_data = Array.create ~len:capacity default; mod_count = 0}
  ;;

  let max_array_length = Sys.max_array_length
  let of_array a = {size = Array.length a; element_data = Array.copy a; mod_count = 0}

  let of_list l =
    let a = Array.of_list l in
        of_array a
  ;;

  let copy al =
    {size = al.size; element_data = Array.copy al.element_data; mod_count = al.mod_count}
  ;;

  let to_array al = Array.sub al.element_data ~pos:0 ~len:al.size

  let huge_capacity _al min_capacity =
    if min_capacity < 0 then
      failwith "Out of memory"
    else if min_capacity > max_array_length then
      Int.max_value
    else
      max_array_length
  ;;

  let grow al min_capacity =
    let old_capacity = Array.length al.element_data in
    let new_capacity =
      let new_capacity = ref (old_capacity + (old_capacity lsr 1)) in
          if !new_capacity - min_capacity < 0 then new_capacity := min_capacity;
          if !new_capacity - Sys.max_array_length > 0 then
            new_capacity := huge_capacity al min_capacity;
          !new_capacity
  in
        al.element_data <- unsafe_copy al.element_data new_capacity
  ;;

  let ensure_capacity_internal al min_capacity =
    al.mod_count <- al.mod_count + 1;
    if min_capacity - Array.length al.element_data > 0 then grow al min_capacity
  ;;

  let trim_to_size al =
    al.mod_count <- al.mod_count + 1;
    let old_capacity = Array.length al.element_data in
        if al.size < old_capacity then
          al.element_data <- unsafe_copy al.element_data al.size
  ;;

  let ensure_capacity al min_capacity =
    if min_capacity > 0 then ensure_capacity_internal al min_capacity
  ;;

  let element_data al index = al.element_data.(index)

  let range_check al index =
    if index >= al.size then raise (Out_of_bounds (out_of_bound_msg al index))
  ;;

  let range_check_for_add al index =
    if index > al.size || index < 0 then raise (Out_of_bounds (out_of_bound_msg al index))
  ;;

  let get al index =
    range_check al index;
    element_data al index
  ;;

  let set al index elmt =
    range_check al index;
    al.element_data.(index) <- elmt
  ;;

  let add al index elmt =
    range_check_for_add al index;
    ensure_capacity_internal al (al.size + 1);
    Array.blit
      ~src:al.element_data
      ~src_pos:index
      ~dst:al.element_data
      ~dst_pos:(index + 1)
      ~len:(al.size - index);
    al.element_data.(index) <- elmt;
    al.size <- al.size + 1
  ;;

  let remove al index =
    range_check al index;
    al.mod_count <- al.mod_count + 1;
    let old_value = al.element_data.(index) in
    let num_moved = al.size - index - 1 in
        if num_moved > 0 then
          Array.blit
            ~src:al.element_data
            ~src_pos:(index + 1)
            ~dst:al.element_data
            ~dst_pos:index
            ~len:num_moved;
        al.size <- al.size - 1;
        unsafe_set al.element_data al.size 0;
        old_value
  ;;

  let fast_remove al index =
    al.mod_count <- al.mod_count + 1;
    let num_moved = al.size - index - 1 in
        if num_moved > 0 then
          Array.blit
            ~src:al.element_data
            ~src_pos:(index + 1)
            ~dst:al.element_data
            ~dst_pos:index
            ~len:num_moved;
        al.size <- al.size - 1;
        unsafe_set al.element_data al.size 0
  ;;

  let clear al =
    al.mod_count <- al.mod_count + 1;
    for i = 0 to al.size - 1 do
      unsafe_set al.element_data i 0
    done;
    al.size <- 0
  ;;

  let append al1 al2 =
    let num_new = al2.size in
        ensure_capacity_internal al1 (al1.size + num_new);
        Array.blit
          ~src:al2.element_data
          ~src_pos:0
          ~dst:al1.element_data
          ~dst_pos:al1.size
          ~len:num_new;
        al1.size <- al1.size + num_new;
        num_new <> 0
  ;;

  let iter f al =
    for i = 0 to al.size - 1 do
      f al.element_data.(i)
    done
  ;;

  let iteri f al =
    for i = 0 to al.size - 1 do
      f i al.element_data.(i)
    done
  ;;

  let fold_left f init al =
    let v = ref init in
        for i = 0 to al.size - 1 do
          v := f !v al.element_data.(i)
        done;
        !v
  ;;

  let fold_right f al init =
    let v = ref init in
        for i = al.size - 1 downto 0 do
          v := f al.element_data.(i) !v
        done;
        !v
  ;;

  let to_list al = fold_right (fun e acc -> e :: acc) al []
end
