(*
 * Copyright (c) 2021 Nikos Leivadaris
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *)

type level = {
  id : int;
  mutable length : int;
}

type ('k, 'v) node =
  | Nil
  | NInf of ('k, 'v) node Array.t
  | Node of {
      key : 'k;
      mutable value : 'v;
      mutable next : ('k, 'v) node Array.t;
    }

type ('k, 'v) t = {
  mutable head : ('k, 'v) node;
  mutable tail : ('k, 'v) node;
  mutable length : int;
  mutable cur_level : int;
  mutable level : level Array.t;
  mutable finger : ('k, 'v) node Array.t;
  max_level : int;
}

module SNode : sig
  val create : 'k -> 'v -> int -> ('k, 'v) node
  val value : ('k, 'v) node -> ('k * 'v) option
  val update : ('k, 'v) node -> 'v -> unit

  (* val next : ('k, 'v) node -> int -> ('k, 'v) node *)

  val has_next : int -> ('k, 'v) node -> bool
  val link : int -> ('k, 'v) node -> ('k, 'v) node -> unit
  val unlink : int -> ('k, 'v) node -> ('k, 'v) node -> unit
end = struct
  let create k v l =
      Node {key = k; value = v; next = Array.init l (fun _ -> Nil)}

  let value = function
      | Nil | NInf _ -> None
      | Node n -> Some (n.key, n.value)

  let update c v =
      match c with
      | NInf _ | Nil -> assert false
      | Node n -> n.value <- v

  let next n i =
      match n with
      | Nil -> Nil
      | NInf l -> l.(i)
      | Node l -> l.next.(i)

  let has_next i n =
      match next n i with
      | Nil -> false
      | Node _ -> true
      | NInf _ -> assert false

  let link i n1 n2 =
      let n3 = next n1 i in
          match (n1, n2, n3) with
          | NInf l1, Node l2, Nil ->
              l1.(i) <- n2;
              l2.next.(i) <- n3
          | NInf l1, Node l2, Node _ ->
              l1.(i) <- n2;
              l2.next.(i) <- n3
          | Node l1, Node l2, Nil ->
              l1.next.(i) <- n2;
              l2.next.(i) <- n3
          | Node l1, Node l2, Node _ ->
              l1.next.(i) <- n2;
              l2.next.(i) <- n3
          | Nil, _, _ -> assert false
          | NInf _, _, _ -> assert false
          | Node _, _, _ -> assert false

  let clear n i =
      match n with
      | NInf l ->
          let nn = l.(i) in
              l.(i) <- Nil;
              nn
      | Node l ->
          let nn = l.next.(i) in
              l.next.(i) <- Nil;
              nn
      | Nil -> Nil

  let unlink i n1 n2 =
      let n3 = clear n2 i in
          match (n1, n3) with
          | NInf l1, Nil -> l1.(i) <- n3
          | NInf l1, Node _ -> l1.(i) <- n3
          | Node l1, Nil -> l1.next.(i) <- n3
          | Node l1, Node _ -> l1.next.(i) <- n3
          | Nil, _ -> assert false
          | NInf _, _ -> assert false
          | Node _, _ -> assert false
end

let prng = lazy (Random.State.make_self_init ())

let flip (p : int) : int =
    let lvl = ref 0 in
        while Random.State.bool (Lazy.force prng) && !lvl < p - 1 do
          incr lvl
        done;
        !lvl

module type OrderedType = sig
  type t

  val compare : t -> t -> int
  val to_string : t -> string
end

module type S = sig
  type key
  type 'a t

  val create : ?max_level:int -> unit -> 'a t
  val is_empty : 'a t -> bool
  val length : 'a t -> int
  val min : 'a t -> (key * 'a) option
  val max : 'a t -> (key * 'a) option
  val find : 'a t -> key -> 'a option
  val find_finger : 'a t -> key -> 'a option

  val find_nearest :
    'a t -> key -> [`Gt of key * 'a | `Lt of key * 'a | `Eq of key * 'a | `Empty]

  val find_range : start:key -> stop:key -> 'a t -> (key * 'a) list
  val add : key:key -> value:'a -> 'a t -> unit
  val remove : 'a t -> key -> unit
  val mem : 'a t -> key -> bool
  val iter : f:(key -> 'a -> unit) -> 'a t -> unit
  val filter_map_inplace : f:(key -> 'a -> 'a option) -> 'a t -> unit
  val fold : 'a t -> init:'b -> f:(key:key -> value:'a -> 'b -> 'b) -> 'b
  val flip : int -> int
  val clear : 'a t -> unit
  val copy : ?max_level:int -> 'a t -> 'a t
  val of_alist : (key * 'a) list -> 'a t
  val to_alist : 'a t -> (key * 'a) list
  val to_string : 'a t -> string
  val pp : Format.formatter -> 'a t -> unit
  val to_seq : 'a t -> (key * 'a) Seq.t
  val to_seq_keys : _ t -> key Seq.t
  val to_seq_values : 'a t -> 'a Seq.t
  val add_seq : 'a t -> (key * 'a) Seq.t -> unit
  val of_seq : (key * 'a) Seq.t -> 'a t
end

module Make (Ord : OrderedType) : S with type key = Ord.t = struct
  type key = Ord.t
  type 'a skiplist = (key, 'a) t
  type 'a t = 'a skiplist

  type 'a ranged =
    [ `Gt of 'a
    | `Lt of 'a
    | `Eq of 'a
    | `Empty ]

  let default_max_level = 16
  let flip = flip

  let create ?(max_level = default_max_level) () : 'a t =
      let lvl = Array.init max_level (fun _ -> Nil) in
      let head = NInf lvl in
      let level = Array.init max_level (fun i -> {id = i + 1; length = 0}) in
      let finger = Array.init max_level (fun _ -> head) in
          {
            length = 0;
            tail = Nil;
            cur_level = 0;
            head;
            level;
            max_level;
            finger;
          }

  let is_empty (sl : 'a t) = sl.length = 0
  let length (sl : 'a t) : int = sl.length

  (* let level (sl : 'a t) : int = sl.max_level *)

  let min (sl : 'a t) : (key * 'a) option =
      match sl.head with
      | NInf l -> SNode.value l.(0)
      | Nil -> assert false
      | Node _ -> assert false

  let max (sl : 'a t) : (key * 'a) option =
      match sl.tail with
      | Nil -> None
      | Node n -> Some (n.key, n.value)
      | NInf _ -> assert false

  let compare_key k1 k2 =
      match Ord.compare k1 k2 with
      | -1 -> `Lt
      | 0 -> `Eq
      | 1 -> `Gt
      | _ -> assert false

  (* let compare_key_node k = function | NInf _ -> `Gt | Nil -> `Empty | Node {
      key; _ } -> ( match Ord.compare k key with | -1 -> `Lt | 0 -> `Eq | 1 ->
      `Gt | _ -> assert false) *)

  let search_level k i c =
      let rec aux_iter_lvl c pc m =
          match c with
          | NInf n -> aux_iter_lvl n.(i) c m
          | Nil -> (pc, pc, m)
          | Node n -> (
              match compare_key k n.key with
              | `Lt -> (pc, c, `Lt)
              | `Eq -> (pc, c, `Eq)
              | `Gt -> aux_iter_lvl n.next.(i) c m
              | _ -> assert false)
      in
          aux_iter_lvl c Nil `Gt

  let rec find_node_eq key c pn i =
      if i >= 0 then
        match c with
        | NInf n -> find_node_eq key n.(i) c i
        | Nil -> find_node_eq key pn c (i - 1)
        | Node {key = k; value; next} -> (
            match Ord.compare key k with
            | -1 -> find_node_eq key pn c (i - 1)
            | 0 -> Some value
            | 1 -> find_node_eq key next.(i) c i
            | _ -> assert false)
      else
        None

  let find_nearest_nodes (sl : 'a t) (key : key) lvl :
      [`Add of (int * (key, 'a) node) list | `Update of (key, 'a) node] =
      let rec aux_find c i v =
          if i < 0 then
            `Add v
          else
            let pn, n, m = search_level key i c in
                match m with
                | `Eq -> `Update n
                | _ -> aux_find pn (i - 1) ((i, pn) :: v)
      in
          aux_find sl.head lvl []

  let rec find_node_nearest k c pn i d : (key * 'a) ranged =
      if i >= 0 then
        match c with
        | NInf n -> find_node_nearest k n.(i) c i d
        | Nil -> find_node_nearest k pn c (i - 1) d
        | Node {key; value; next} -> (
            match Ord.compare k key with
            | -1 -> find_node_nearest k pn c (i - 1) (`Lt (key, value))
            | 0 -> `Eq (key, value)
            | 1 -> find_node_nearest k next.(i) c i (`Gt (key, value))
            | _ -> assert false)
      else
        d

  let find_linked (sl : 'a t) (key : key) :
      (int * (key, 'a) node) list * (key, 'a) node =
      let rec aux_find c i v n =
          if i < 0 then
            (v, n)
          else
            let pn, n', m = search_level key i c in
                match m with
                | `Eq -> aux_find pn (i - 1) ((i, pn) :: v) n'
                | _ -> aux_find pn (i - 1) v n
      in
          aux_find sl.head sl.cur_level [] Nil

  let find (sl : 'a t) (key : key) : 'a option =
      if is_empty sl then
        None
      else
        find_node_eq key sl.head Nil sl.cur_level

  let find_finger_point (key : key) (sl : 'a t) =
      let rec fwd k i sl =
          if i <= sl.cur_level then
            let c = sl.finger.(i) in
                match c with
                | NInf _ | Nil -> fwd k (i + 1) sl
                | Node n ->
                    if Ord.compare n.key k <= 0 then
                      (i, c)
                    else
                      fwd k (i + 1) sl
          else
            (i - 1, sl.head)
      in
          fwd key 0 sl

  let add_finger_point key lvl n sl =
      let rec aux_add_finger k i c p sl =
          match c with
          | NInf n -> aux_add_finger k i n.(i) c sl
          | Nil -> sl.finger.(i) <- p
          | Node n -> (
              match compare_key k n.key with
              | `Lt | `Eq -> sl.finger.(i) <- c
              | `Gt -> aux_add_finger k i n.next.(i) c sl)
      in
          for i = lvl downto 0 do
            aux_add_finger key i n sl.head sl
          done

  let find_finger (sl : 'a t) (key : key) : 'a option =
      if is_empty sl then
        None
      else
        let i, m = find_finger_point key sl in
            add_finger_point key i m sl;
            match sl.finger.(0) with
            | Nil | NInf _ -> None
            | Node n -> (
                match compare_key key n.key with
                | `Eq -> Some n.value
                | _ -> None)

  let find_nearest (sl : 'a t) (key : key) :
      [`Gt of key * 'a | `Lt of key * 'a | `Eq of key * 'a | `Empty] =
      find_node_nearest key sl.head Nil sl.cur_level `Empty

  let rec from k c =
      match c with
      | NInf n -> from k n.(0)
      | Node n ->
          if Ord.compare k n.key <= 0 then
            c
          else
            from k n.next.(0)
      | Nil -> Nil

  let rec until acc k c =
      match c with
      | NInf _ | Nil -> acc
      | Node n ->
          if Ord.compare n.key k < 0 then
            until ((n.key, n.value) :: acc) k n.next.(0)
          else
            acc

  let find_range ~(start : key) ~(stop : key) (sl : 'a t) : (key * 'a) list =
      if start > stop then
        []
      else
        from start sl.head |> until [] stop

  let add ~(key : key) ~(value : 'a) (sl : 'a t) : unit =
      (* let rec aux_add (p : (int * (key, 'a) node) list) n (i : int) = match p
         with | [] -> () | h :: t -> let l, pn = h in if i >= l then ( SNode.link
         l pn n; aux_add t n i) else () in *)
      let rec take m seq () =
          match seq () with
          | Seq.Nil -> Seq.Nil
          | Seq.Cons (((i, _) as x), next) ->
              if m >= i then
                Seq.Cons (x, take m next)
              else
                Seq.Nil
      in
      let op = find_nearest_nodes sl key (sl.max_level - 1) in
          match op with
          | `Update n -> SNode.update n value
          | `Add path ->
              let lvl = flip sl.max_level in
                  if sl.cur_level < lvl then sl.cur_level <- lvl;
                  let n = SNode.create key value sl.max_level in
                      sl.length <- sl.length + 1;
                      List.to_seq path
                      |> take lvl
                      |> Seq.iter (fun (l, pn) ->
                             let lvl = sl.level.(l) in
                                 lvl.length <- lvl.length + 1;
                                 SNode.link l pn n);
                      if SNode.has_next 0 n then
                        ()
                      else
                        sl.tail <- n

  let remove (sl : 'a t) (key : key) : unit =
      let rec aux_remove path n =
          match path with
          | [] -> if sl.length = 0 then sl.tail <- Nil
          | h :: t ->
              let l, pn = h in
                  SNode.unlink l pn n;
                  let lvl = sl.level.(l) in
                      lvl.length <- lvl.length - 1;
                      sl.finger.(l) <- pn;
                      if l = 0 && not (SNode.has_next 0 pn) then sl.tail <- pn;
                      aux_remove t n
      in
      let path, n = find_linked sl key in
          match n with
          | Node _ ->
              sl.length <- sl.length - 1;
              aux_remove path n
          | Nil | NInf _ -> ()

  let mem (sl : 'a t) (key : key) : bool =
      let f = find sl key in
          match f with
          | None -> false
          | Some _ -> true

  let filter_map_inplace ~(f : key -> 'a -> 'a option) (sl : 'a t) =
      let rec aux_do c =
          match c with
          | Nil -> ()
          | NInf n -> aux_do n.(0)
          | Node n -> (
              let next = n.next.(0) in
                  match f n.key n.value with
                  | None -> remove sl n.key
                  | Some v ->
                      n.value <- v;
                      aux_do next)
      in
          aux_do sl.head

  let fold (sl : 'a t) ~init ~f =
      let rec aux_fold c acc =
          match c with
          | Nil -> acc
          | NInf n -> aux_fold n.(0) acc
          | Node n -> aux_fold n.next.(0) (f ~key:n.key ~value:n.value acc)
      in
          aux_fold sl.head init

  let iter ~f sl = fold sl ~init:() ~f:(fun ~key ~value _ -> f key value)

  let clear (sl : 'a t) =
      sl.length <- 0;
      sl.cur_level <- 0;
      sl.tail <- Nil;
      sl.head <- NInf (Array.init sl.max_level (fun _ -> Nil));
      sl.level <- Array.init sl.max_level (fun i -> {id = i + 1; length = 0});
      sl.finger <- Array.init sl.max_level (fun _ -> sl.head)

  let copy ?max_level (sl : 'a t) : 'a t =
      let ml =
          match max_level with
          | None -> sl.max_level
          | Some n -> n
      in
      let sl' = create ~max_level:ml () in
          fold sl ~init:sl' ~f:(fun ~key ~value acc ->
              add acc ~key ~value;
              acc)

  let of_alist (el : (key * 'a) list) : 'a t =
      let sl = create () in
          List.iter (fun (key, value) -> add ~key ~value sl) el;
          sl

  let to_alist (sl : 'a t) : (key * 'a) list =
      fold sl ~init:[] ~f:(fun ~key ~value acc -> (key, value) :: acc)

  let to_string (sl : 'a t) : string =
      let rec aux_lvl c i str =
          match c with
          | Nil -> str ^ "]"
          | NInf n -> aux_lvl n.(i) i "[ "
          | Node n -> aux_lvl n.next.(i) i (str ^ Ord.to_string n.key ^ "; ")
      in
      let rec aux_str c i str =
          let i = i - 1 in
              if i < 0 then
                str
              else
                let lvl = aux_lvl c i "" in
                    aux_str c i (Printf.sprintf "%slevel: %d = %s\n" str i lvl)
      in
      let key_str k =
          match k with
          | None -> "None"
          | Some (k', _) -> Ord.to_string k'
      in
      let hdr =
          Printf.sprintf "Info: max_level: %d, level: %d, len: %#d\n"
            sl.max_level sl.cur_level sl.length
      in
      let aux_node c =
          match c with
          | Nil -> "nill"
          | NInf _ -> "ninf"
          | Node n -> Ord.to_string n.key
      in
      let rec finger_str f acc =
          match f with
          | [] -> "[" ^ acc ^ "]"
          | h :: t -> finger_str t (aux_node h ^ "; " ^ acc)
      in
      let info =
          Printf.sprintf "%s Elements: min: %s, max: %s\nfinger: %s\n" hdr
            (key_str (min sl))
            (key_str (max sl))
            (finger_str (Array.to_list sl.finger) "")
      in
          aux_str sl.head sl.max_level info

  let pp ppf sl = Format.pp_print_string ppf (to_string sl)

  let to_seq sl : (key * 'a) Seq.t =
      let rec aux_seq c () =
          match c with
          | Nil -> Seq.Nil
          | NInf n -> aux_seq n.(0) ()
          | Node n -> Seq.Cons ((n.key, n.value), aux_seq n.next.(0))
      in
          aux_seq sl.head

  let to_seq_keys sl : key Seq.t = Seq.map fst (to_seq sl)
  let to_seq_values sl : 'a Seq.t = Seq.map snd (to_seq sl)
  let add_seq sl i = Seq.iter (fun (key, value) -> add ~key ~value sl) i

  let of_seq i =
      let sl = create () in
          add_seq sl i;
          sl
end
