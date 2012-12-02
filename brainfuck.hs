import System.IO
import System.Environment
import Data.Char
import Debug.Trace

-- XXX better error handling
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

inputb :: Char -> Memory -> Memory
inputb byte (a,b) 
    -- | trace ("inputb " ++ show (head input)) False = undefined
    | otherwise = (a, (ord byte) : (tail b))

bf :: Program -> Memory -> IO ()
-- return current output if we reached the end of the program
bf (_,[]) _ = return ()
bf prg mem
    -- | trace trace_bf False = undefined
    | (current prg) == '>' = bf (advance prg) (advance mem)
    | (current prg) == '<' = bf (advance prg) (recede mem)
    -- increment/decrement pointed data
    | (current prg) == '+' = bf (advance prg) (increment mem)
    | (current prg) == '-' = bf (advance prg) (decrement mem)
    -- input/output byte
    | (current prg) == '.' = do putChar $ chr $ current mem
                                hFlush stdout
                                bf (advance prg) mem 
    | (current prg) == ',' = do byte <- (catch getChar) (\_ -> do return (chr 7))
                                if byte /= (chr 7) then
                                    bf (advance prg) (inputb byte mem)
                                else
                                    do putStrLn "EOF found"
                                       return ()
    -- loops
    | (current prg) == '[' = bf (loop_start prg mem) mem
    | (current prg) == ']' = bf (loop_end prg mem) mem
    -- just ignore current character
    | otherwise = bf (advance prg) mem
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

interpret :: Stream -> IO ()
interpret program = do bf (str2program program) 
                          (buildmem 30000)

main = do 
       (filename:_) <- getArgs
       program <- readFile filename
       interpret program
