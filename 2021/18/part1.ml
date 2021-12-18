let (let*) = Option.bind

type node =
  | Num of int
  | Pair of node * node

let rec show = function
  | Num v -> Printf.sprintf "%d" v
  | Pair (a, b) ->
    let sa = show a
    and sb = show b in
    Printf.sprintf "[%s,%s]" sa sb

let rec add_left value = function
  | Pair (a, b) -> Pair (add_left value a, b)
  | Num v -> Num (v + value)

let rec add_right value = function
  | Pair (a, b) -> Pair (a, add_right value b)
  | Num v -> Num (v + value)

let rec explode depth = function
  | Num _ -> None
  | Pair (Num a, Num b) when depth >= 4 -> Some (a, Num 0, b)
  | Pair (a, b) ->
    match explode (depth+1) a with
      Some (l, n, r) -> Some (l, Pair (n, add_left r b), 0)
    | None -> match explode (depth+1) b with
        Some (l, n, r) -> Some (0, Pair (add_right l a, n), r)
      | None -> None

let rec split = function
  | Num v when v >= 10 -> Some (Pair (Num (v/2), Num ((v+1)/2)))
  | Num _ -> None
  | Pair (a, b) ->
    match split a with
      Some n -> Some (Pair (n, b))
    | None -> match split b with 
        Some n -> Some (Pair (a, n))
      | None -> None

let rec simplify node =
  match explode 0 node with
    Some (l, n, r) -> simplify n
  | None -> match split node with
      Some n -> simplify n
    | None -> node

let add a b = simplify (Pair (a, b))

let rec magnitude = function
  | Num v -> v
  | Pair (a, b) -> (magnitude a)*3 + (magnitude b)*2

let eat ch = function
  | head::tail when head = ch -> Some tail
  | _ -> None

let rec parse = function
  | [] -> None
  | ch::tail -> match ch with
      '[' -> begin
        let* (a, tail) = parse tail in
        let* tail = eat ',' tail in
        let* (b, tail) = parse tail in
        let* tail = eat ']' tail in
        Some (Pair (a, b), tail)
      end
    | '0'..'9' -> Some (Num (Char.code ch - Char.code '0'), tail)
    | _ -> None

let list_of_string s = List.init (String.length s) (String.get s)

let rec main total = try
    match (parse (list_of_string (input_line stdin))) with
      None -> main total
    | Some (node, _) -> 
        match total with
          Some pre -> main (Some (add pre node))
        | None -> main (Some node)
  with End_of_file -> total

let _ = Printf.printf "%d\n" (magnitude (Option.get (main None)))
