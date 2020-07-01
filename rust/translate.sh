set -x

SOURCE=src/`basename ${1//.bf/.${2:-rs}}`
TARGET=src/`basename ${1//.bf/}`

cargo run --bin translate ${2:-rs} < $1 > $SOURCE
time make $TARGET
time ./$TARGET
rm $SOURCE $TARGET