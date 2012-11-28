import Data.Char

alter :: [Char] -> Int -> (Int -> Int) -> [Char]
alter mem ptr f = pre ++ [value] ++ post
    where pre = take ptr mem
          value = chr $ f $ ord (mem!!ptr)
          post = drop (ptr+1) mem

increment mem ptr = alter mem ptr (\x -> x+1)
decrement mem ptr = alter mem ptr (\x -> x-1)

-- XXX single function that receives direction, condition, match

loop_start :: [Char] -> [Char] -> Int -> Int -> Int
loop_start prg mem ip ptr
    | ord (mem!!ptr) == 0 = find_matching_fwd prg (ip+1) 1
    | otherwise = ip + 1
    where find_matching_fwd prg ip count
            | count == 0 = ip
            | prg!!ip == '[' = find_matching_fwd prg (ip+1) (count+1)
            | prg!!ip == ']' = find_matching_fwd prg (ip+1) (count-1)
            | otherwise = find_matching_fwd prg (ip+1) count

loop_end :: [Char] -> [Char] -> Int -> Int -> Int
loop_end prg mem ip ptr
    | ord (mem!!ptr) > 0 = find_matching_bwd prg (ip-1) 1
    | otherwise = ip + 1
    where find_matching_bwd prg ip count
            | count == 0 = ip
            | prg!!ip == ']' = find_matching_bwd prg (ip-1) (count+1)
            | prg!!ip == '[' = find_matching_bwd prg (ip-1) (count-1)
            | otherwise = find_matching_bwd prg (ip-1) count

bf :: [Char] -> [Char] -> [Char] -> Int -> Int -> [Char]
bf prg mem output ip ptr
    -- return current status if we reached the end of the program
    | ip >= (length prg) = output
    -- increment/decrement data pointer
    | prg!!ip == '>' = bf prg mem output (ip+1) (ptr+1)
    | prg!!ip == '<' = bf prg mem output (ip+1) (ptr-1)
    -- increment/decrement pointed data
    | prg!!ip == '+' = bf prg (increment mem ptr) output (ip+1) ptr
    | prg!!ip == '-' = bf prg (decrement mem ptr) output (ip+1) ptr
    -- input/output byte
    | prg!!ip == '.' = bf prg mem (output ++ [mem!!ptr]) (ip+1) ptr
    | prg!!ip == ',' = error "',' not supported" -- XXX support for ','
    -- loops
    | prg!!ip == '[' = bf prg mem output (loop_start prg mem ip ptr) ptr
    | prg!!ip == ']' = bf prg mem output (loop_end prg mem ip ptr) ptr
    | otherwise = bf prg mem output (ip+1) ptr

chr2bf :: Char -> [Char]
chr2bf c = take (ord c) (repeat '+') ++ ".>"
hello = concat $ map chr2bf "hello world"

-- XXX lazy evaluation to see partial results
-- XXX program name as argument, stdin program input
--     http://learnyouahaskell.com/input-and-output#files-and-streams
-- XXX version with lists manipulation instead of indexes
-- XXX ASM interpreter?

interpreter input = show $ bf program mem output ip ptr
    where program = input
          mem = take 30000 (repeat (chr 0))
          output = []
          ip = 0
          ptr = 0

main = interact interpreter
