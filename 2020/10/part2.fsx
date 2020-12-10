open System

let input =
    Seq.initInfinite (ignore >> Console.ReadLine)
    |> Seq.map Int32.TryParse
    |> Seq.takeWhile fst
    |> Seq.map snd
    |> Seq.sort
    |> Seq.toList

let rec waysOf counts delta =
    match delta with
    | 1 -> (List.take 3 counts |> List.sum) :: counts
    | _ -> waysOf (0L :: counts) (delta - 1)

let ways =
    Seq.concat [ [0]; input; [List.last input + 3] ]
    |> Seq.windowed 2
    |> Seq.map (fun a -> a.[1] - a.[0])
    |> Seq.fold waysOf [1L; 0L; 0L]
    |> List.head

Console.WriteLine(ways)
