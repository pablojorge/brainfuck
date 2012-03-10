-- Brainfuck to C translator

translate :: [Char] -> [Char]
translate (x:xs) = equiv x ++ translate xs
    where equiv x | x == '>' = "++ptr;"
                  | x == '<' = "--ptr;"
                  | x == '+' = "++(*ptr);"
                  | x == '-' = "--(*ptr);"
                  | x == '.' = "putchar(*ptr);" ++ 
                               "fflush(stdout);"
                  | x == ',' = "*ptr = getchar();" ++ 
                               "if (*ptr == EOF) exit(0);"
                  | x == '[' = "while(*ptr) {"
                  | x == ']' = "}"
          equiv x = ""
translate [] = ""

translator :: [Char] -> [Char]
translator input = "#include <stdio.h>\n" ++
                   "#include <stdlib.h>\n" ++
                   "int main() {" ++ 
                   "  char mem[30000]," ++
                   "       *ptr = mem;" ++ 
                   translate input ++
                   "  return 0;" ++
                   "}"

main = interact translator
