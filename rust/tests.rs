use std::collections::HashMap;

mod bf;

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
        bf::bf_jumps("[[]]".as_bytes()).unwrap(),
        map![(0,3), (1,2), (2,1), (3,0)]
    );

    assert_fails!(bf::bf_jumps("[[]]]".as_bytes()),
                  bf::InvalidProgramError::UnexpectedClosingBracket(4));

    assert_fails!(bf::bf_jumps("-=[[[[]]]".as_bytes()),
                  bf::InvalidProgramError::ExcessiveOpeningBrackets(2));

    assert_eq!(
        bf::bf_eval("+++>+++>+++".as_bytes(), 3).unwrap(),
        vec![3, 3, 3]
    );

    assert_eq!(
        bf::bf_eval("+++>+++>+++--<-".as_bytes(), 3).unwrap(),
        vec![3, 2, 1]
    );

    assert_fails!(bf::bf_eval("-=[[[[]]]".as_bytes(), 5),
                  bf::BFEvalError::InvalidProgramError(
                      bf::InvalidProgramError::ExcessiveOpeningBrackets(2)
                  ));
}