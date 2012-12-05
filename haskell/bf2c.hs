-- Brainfuck to C translator

translate :: [Char] -> [Char]
translate (x:xs) = equiv x ++ translate xs
    where equiv x | x == '>' = "++ptr;\n"
                  | x == '<' = "--ptr;\n"
                  | x == '+' = "++(*ptr);\n"
                  | x == '-' = "--(*ptr);\n"
                  | x == '.' = "putchar(*ptr);\n" ++ 
                               "fflush(stdout);\n"
                  | x == ',' = "*ptr = getchar();\n" ++ 
                               "if (*ptr == EOF) exit(0);\n"
                  | x == '[' = "while(*ptr) {\n"
                  | x == ']' = "}\n"
          equiv x = ""
translate [] = ""

translator :: [Char] -> [Char]
translator input = "#include <stdio.h>\n" ++
                   "#include <stdlib.h>\n" ++
                   "int main() {\n" ++ 
                   "  char mem[30000],\n" ++
                   "       *ptr = mem;\n" ++ 
                   translate input ++
                   "  return 0;\n" ++
                   "}\n"

main = interact translator
