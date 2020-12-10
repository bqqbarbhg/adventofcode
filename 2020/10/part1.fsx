open System

let input =
    Seq.initInfinite (ignore >> Console.ReadLine)
    |> Seq.map Int32.TryParse
    |> Seq.takeWhile fst
    |> Seq.map snd
    |> Seq.sort
    |> Seq.toList

let counts =
    Seq.concat [ [0]; input; [List.last input + 3] ]
    |> Seq.windowed 2
    |> Seq.map (fun a -> a.[1] - a.[0])
    |> Seq.countBy id
    |> Map.ofSeq

Console.WriteLine(counts.[1] * counts.[3])
