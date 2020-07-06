use brainfuck as bf;

#[test]
fn test_tokenizer_emits_expected_tokens() {
    let program = "+[,.]";
    let result = bf::tokenize(&program.chars().collect());
    let expected = vec![
        bf::Token::ProgramStart,
        bf::Token::IncValue(0),
        bf::Token::LoopStart(1),
        bf::Token::InputValue(2),
        bf::Token::OutputValue(3),
        bf::Token::LoopEnd(4),
        bf::Token::ProgramEnd,
    ];

    assert_eq!(result, expected);
}

#[test]
fn test_parse_wellformed_program() {
    let program = "+-+[>,.<]";
    let tokens = bf::tokenize(&program.chars().collect());
    let result = bf::parse(&tokens).unwrap();
    let expected = vec![
        bf::Expression::IncValue(1),
        bf::Expression::DecValue(1),
        bf::Expression::IncValue(1),
        bf::Expression::Loop(vec![
            bf::Expression::MoveForward(1),
            bf::Expression::InputValue,
            bf::Expression::OutputValue,
            bf::Expression::MoveBack(1),
        ]),
    ];

    assert_eq!(result, expected);
}

#[test]
fn test_parse_fails_with_extra_opening_bracket() -> Result<(), String>{
    let program = "+[[,.]";
    let tokens = bf::tokenize(&program.chars().collect());
    match bf::parse(&tokens) {
        Err(bf::InvalidProgramError::ExcessiveOpeningBrackets(0)) => Ok(()),
        _ => Err(String::from("Extra opening bracket error not triggered")),
    }
}

#[test]
fn test_parse_fails_with_extra_closing_bracket() -> Result<(), String>{
    let program = "+[,.]]";
    let tokens = bf::tokenize(&program.chars().collect());
    match bf::parse(&tokens) {
        Err(bf::InvalidProgramError::UnexpectedClosingBracket(5)) => Ok(()),
        _ => Err(String::from("Extra opening bracket error not triggered")),
    }
}

#[test]
fn test_execution_produces_expected_mem_contents() {
    let program = "+++>+++>+++--<-";
    let tokens = bf::tokenize(&program.chars().collect());
    let expressions = bf::parse(&tokens).unwrap();
    let result = bf::run(&expressions).unwrap();
    let expected = [3, 2, 1];

    assert_eq!(result.buf()[0..3], expected);
}

#[test]
fn test_optimizer() {
    let program = "+++>[+++>[+++--]<-]";
    let tokens = bf::tokenize(&program.chars().collect());

    let expressions = bf::parse(&tokens).unwrap();
    let expected = vec![
        bf::Expression::IncValue(1),
        bf::Expression::IncValue(1),
        bf::Expression::IncValue(1),
        bf::Expression::MoveForward(1),
        bf::Expression::Loop(vec![
            bf::Expression::IncValue(1),
            bf::Expression::IncValue(1),
            bf::Expression::IncValue(1),
            bf::Expression::MoveForward(1),
            bf::Expression::Loop(vec![
                bf::Expression::IncValue(1),
                bf::Expression::IncValue(1),
                bf::Expression::IncValue(1),
                bf::Expression::DecValue(1),
                bf::Expression::DecValue(1),
            ]),
            bf::Expression::MoveBack(1),
            bf::Expression::DecValue(1),
        ]),
    ];
    assert_eq!(expressions, expected);

    let optimized = bf::optimize(&expressions);
    let expected = vec![
        bf::Expression::IncValue(3),
        bf::Expression::MoveForward(1),
        bf::Expression::Loop(vec![
            bf::Expression::IncValue(3),
            bf::Expression::MoveForward(1),
            bf::Expression::Loop(vec![
                bf::Expression::IncValue(3),
                bf::Expression::DecValue(2),
            ]),
            bf::Expression::MoveBack(1),
            bf::Expression::DecValue(1),
        ]),
    ];

    assert_eq!(optimized, expected);
}
