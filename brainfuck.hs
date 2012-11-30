import Data.Char
import Debug.Trace

type Operator = Char
type Program = ([Operator], [Operator])
type Memory = ([Int], [Int])
type Stream = [Char]

loop prg mem cond move symbol match
    | cond (current mem) = advance (fmatch (move prg))
    | otherwise = advance prg
    where fmatch prg
            -- | trace (trace_fmatch prg) False = undefined
            | (current prg) == symbol = fmatch (move (fmatch (move prg)))
            | (current prg) == match = prg
            | otherwise = fmatch (move prg)
          trace_fmatch prg =
            ("fmatch" ++  
             " prg: " ++ show (current prg)) 

loop_start :: Program -> Memory -> Program
loop_start prg mem = loop prg mem (\x -> x == 0) advance '[' ']'

loop_end :: Program -> Memory -> Program
loop_end prg mem = loop prg mem (\x -> x > 0) recede ']' '['

current :: ([a], [a]) -> a
current (a,b) = head b

advance :: ([a], [a]) -> ([a], [a])
advance (a,b) = (a ++ [head b], tail b)

recede :: ([a], [a]) -> ([a], [a])
recede (a,b) = (init a, (last a) : b)

increment :: Memory -> Memory
increment (a,b) = (a, ((head b) + 1) : (tail b))

decrement :: Memory -> Memory
decrement (a,b) = (a, ((head b) - 1) : (tail b))

bf :: Program -> Memory -> Stream -> Stream
-- return current output if we reached the end of the program
bf (_,[]) mem output = output
bf prg mem output
    -- | trace trace_bf False = undefined
    | (current prg) == '>' = bf (advance prg) (advance mem) output
    | (current prg) == '<' = bf (advance prg) (recede mem) output
    -- increment/decrement pointed data
    | (current prg) == '+' = bf (advance prg) (increment mem) output
    | (current prg) == '-' = bf (advance prg) (decrement mem) output
    -- input/output byte
    | (current prg) == '.' = bf (advance prg) mem (output ++ [chr (current mem)])
    | (current prg) == ',' = error "',' not supported" -- XXX support for ','
    -- loops
    | (current prg) == '[' = bf (loop_start prg mem) mem output
    | (current prg) == ']' = bf (loop_end prg mem) mem output
    -- just ignore current character
    | otherwise = bf (advance prg) mem output
    where trace_bf = 
            ("bf" ++ 
             " prg: " ++ (show (current prg)) ++ 
             " mem: " ++ (show (current mem)))

-- Tracing
            
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
-- XXX ASM interpreter?

interpreter input = show $ bf program mem output
    where program = ([], input)
          mem = ([], take 30000 (repeat 0))
          output = []

main = interact interpreter
