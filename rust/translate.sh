# set -x

TARGET=`basename ${1//.bf/}`

if [ -z "$2" ] || [ "$2" = "rs" ]; then
	SOURCE="src/bin/$TARGET.rs"
	cargo run --bin translate rs < $1 > $SOURCE
	time cargo build --bin $TARGET
	time cargo run --bin $TARGET
elif [ "$2" = "c" ]; then
	SOURCE="$TARGET.c"
	cargo run --bin translate c < $1 > $SOURCE
	time make $TARGET
	time ./$TARGET
	rm $TARGET
else
	echo "Unsupported target language"
	exit 1
fi

rm $SOURCE