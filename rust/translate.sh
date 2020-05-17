set -x

SOURCE=${1//.bf/.${2}}
TARGET=`basename ${1//.bf/}`

time make translate
time make $SOURCE
time make $TARGET
time ./$TARGET

rm $TARGET src/`basename $SOURCE`
