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
        bf::Expression::IncValue,
        bf::Expression::DecValue,
        bf::Expression::IncValue,
        bf::Expression::Loop(vec![
            bf::Expression::MoveForward,
            bf::Expression::InputValue,
            bf::Expression::OutputValue,
            bf::Expression::MoveBack,
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
