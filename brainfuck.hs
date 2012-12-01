import System.Environment
import Data.Char
import Debug.Trace

-- XXX lazy evaluation to see partial results
-- XXX ASM interpreter?

-- Data types
type Operator = Char
type Program = ([Operator], [Operator])
type Memory = ([Int], [Int])
type Stream = [Char]

-- Operations
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

outputb :: Stream -> Memory -> Stream
outputb output mem
    -- | trace ("outputb " ++ (show (chr (current mem)))) False = undefined
    | otherwise = output ++ [chr (current mem)]

inputb :: Stream -> Memory -> Memory
inputb input (a,b) 
    -- | trace ("inputb " ++ show (head input)) False = undefined
    | otherwise = (a, (ord (head input)) : (tail b))

bf :: Program -> Memory -> Stream -> Stream -> Stream
-- return current output if we reached the end of the program
bf (_,[]) _ _ output = output
bf _ _ [] output = output
bf prg mem input output
    -- | trace trace_bf False = undefined
    | (current prg) == '>' = bf (advance prg) (advance mem) input output
    | (current prg) == '<' = bf (advance prg) (recede mem) input output
    -- increment/decrement pointed data
    | (current prg) == '+' = bf (advance prg) (increment mem) input output
    | (current prg) == '-' = bf (advance prg) (decrement mem) input output
    -- input/output byte (the $! is to FORCE the evaluation)
    | (current prg) == '.' = bf (advance prg) mem input $! outputb output mem
    | (current prg) == ',' = bf (advance prg) (inputb input mem) (tail input) output
    -- loops
    | (current prg) == '[' = bf (loop_start prg mem) mem input output
    | (current prg) == ']' = bf (loop_end prg mem) mem input output
    -- just ignore current character
    | otherwise = bf (advance prg) mem input output
    where trace_bf = 
            ("bf" ++ 
             " prg: " ++ (show (current prg)) ++ 
             " mem: " ++ (show (current mem)))

-- Tests

chr2bf :: Char -> [Char]
chr2bf c = take (ord c) (repeat '+') ++ ".>"

str2bf :: [Char] -> [Char]
str2bf s = concat $ map chr2bf s

hello = str2bf "hello world"

looptest = "++[>++[-]<-]."

-- Main

str2program :: Stream -> Program
str2program program = ([], program)

buildmem :: Int -> Memory
buildmem size = ([], take size (repeat 0))

interpret :: Stream -> Stream -> Stream
interpret program input = bf (str2program program) 
                             (buildmem 30000)
                             input 
                             []

printOutput :: String -> IO()
printOutput output = putStr output

main = do 
       (filename:_) <- getArgs
       program <- readFile filename
       input <- getContents
       let output = interpret program input
       printOutput output
