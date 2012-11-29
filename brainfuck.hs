import Data.Char
import Debug.Trace

alter :: [Int] -> Int -> (Int -> Int) -> [Int]
alter mem ptr f = pre ++ [value] ++ post
    where pre = take ptr mem
          value = f $ mem!!ptr
          post = drop (ptr+1) mem

increment mem ptr = alter mem ptr (\x -> x+1)
decrement mem ptr = alter mem ptr (\x -> x-1)

loop prg mem ip ptr cond advance symbol match
    | cond (mem!!ptr) = (fmatch (advance ip)) + 1
    | otherwise = ip + 1
    where fmatch ip
            -- | trace (trace_fmatch ip) False = undefined
            | prg!!ip == symbol = fmatch (advance (fmatch (advance ip)))
            | prg!!ip == match = ip
            | otherwise = fmatch (advance ip)
          trace_fmatch ip =
            ("fmatch" ++  
             " ip: " ++ show ip ++ 
             " prg!!ip: " ++ show (prg!!ip)) 

loop_start :: [Char] -> [Int] -> Int -> Int -> Int
loop_start prg mem ip ptr = loop prg mem ip ptr cond advance '[' ']'
    where cond = (\x -> x == 0)
          advance = (\x -> x + 1)

loop_end :: [Char] -> [Int] -> Int -> Int -> Int
loop_end prg mem ip ptr = loop prg mem ip ptr cond advance ']' '['
    where cond = (\x -> x > 0)
          advance = (\x -> x - 1)

bf :: [Char] -> [Int] -> [Char] -> Int -> Int -> [Char]
bf prg mem output ip ptr
    -- | trace trace_bf False = undefined
    -- return current output if we reached the end of the program
    | ip >= (length prg) = output
    -- increment/decrement data pointer
    | prg!!ip == '>' = bf prg mem output (ip+1) (ptr+1)
    | prg!!ip == '<' = bf prg mem output (ip+1) (ptr-1)
    -- increment/decrement pointed data
    | prg!!ip == '+' = bf prg (increment mem ptr) output (ip+1) ptr
    | prg!!ip == '-' = bf prg (decrement mem ptr) output (ip+1) ptr
    -- input/output byte
    | prg!!ip == '.' = bf prg mem (output ++ [chr (mem!!ptr)]) (ip+1) ptr
    | prg!!ip == ',' = error "',' not supported" -- XXX support for ','
    -- loops
    | prg!!ip == '[' = bf prg mem output (loop_start prg mem ip ptr) ptr
    | prg!!ip == ']' = bf prg mem output (loop_end prg mem ip ptr) ptr
    -- just ignore current character
    | otherwise = bf prg mem output (ip+1) ptr
    where trace_bf = 
            ("bf" ++ 
             " ip: " ++ show ip ++ 
             " symbol: " ++ (showpos prg ip) ++ 
             " ptr: " ++ show ptr ++
             " value: " ++ (showpos mem ptr))

-- Tracing
            
showpos list pos 
    | pos >= (length list) = "<OOB>"
    | otherwise = show (list!!pos)

-- Tests

chr2bf :: Char -> [Char]
chr2bf c = take (ord c) (repeat '+') ++ ".>"

str2bf :: [Char] -> [Char]
str2bf s = concat $ map chr2bf s

hello = str2bf "hello world"

looptest = "++[>++[-]<-]."

-- Todo
--
-- XXX lazy evaluation to see partial results
-- XXX program name as argument, stdin program input
--     http://learnyouahaskell.com/input-and-output#files-and-streams
-- XXX version with lists manipulation instead of indexes
-- XXX ASM interpreter?

interpreter input = show $ bf program mem output ip ptr
    where program = input
          mem = take 30000 (repeat 0)
          output = []
          ip = 0
          ptr = 0

main = interact interpreter
