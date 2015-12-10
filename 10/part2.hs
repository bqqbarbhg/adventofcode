import Data.List
import Data.Maybe

seesay [] = ""
seesay (x:xs) = let index = fromMaybe (length xs) (findIndex (/= x) xs) in
    show (index + 1) ++ [x] ++ seesay (drop index xs)

seesays 0 x = x
seesays n x = seesay (seesays (n - 1) x)

main = do
    line <- getLine
    putStrLn ((show . length . seesays 50) line)

