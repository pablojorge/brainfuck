use std::collections::HashMap;

mod brainfuck;

macro_rules! map {
    ($( $x:expr ),*) => {{
        let mut tmp_map = HashMap::new();
        $(
            tmp_map.insert($x.0, $x.1);
        )*
        tmp_map
    }};
}

macro_rules! assert_fails {
    ($expression:expr, $error:pat) => (
        match $expression {
            Err($error) => 0,
            _ => panic!("Expression '{}' did not fail with '{}'",
                        stringify!($expression),
                        stringify!($error))
        };
    )
}

fn main() {
    assert_eq!(
        brainfuck::bf_jumps("[[]]".as_bytes()).unwrap(),
        map![(0,3), (1,2), (2,1), (3,0)]
    );

    assert_fails!(brainfuck::bf_jumps("[[]]]".as_bytes()),
                  brainfuck::InvalidProgramError::UnexpectedClosingBracket(4));

    assert_fails!(brainfuck::bf_jumps("-=[[[[]]]".as_bytes()),
                  brainfuck::InvalidProgramError::ExcessiveOpeningBrackets(2));
}