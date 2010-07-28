#!/bin/sh
#
# hard to test for secure deletion, but we'll just test to make sure nothing
# breaks

FN1=/tmp/test-baleet-temp1
FN2=/tmp/test-baleet-temp2
FN3=/tmp/test-baleet-temp3
FN4=/tmp/test-baleet-temp4

echo "datas datas datas" > ${FN1}
echo "datas datas datas datas" > ${FN2}
echo "datas datas datas datas datas" > ${FN3}
echo "datas datas datas datas datas datas" > ${FN4}

[ -f ${FN1} ] || fail
[ -f ${FN2} ] || fail
[ -f ${FN3} ] || fail
[ -f ${FN4} ] || fail

./baleet ${FN1}

[ -f ${FN1} ] && fail
[ -f ${FN2} ] || fail
[ -f ${FN3} ] || fail
[ -f ${FN4} ] || fail

./baleet ${FN2} ${FN3}

[ -f ${FN1} ] && fail
[ -f ${FN2} ] && fail
[ -f ${FN3} ] && fail
[ -f ${FN4} ] || fail

fail () {
    rm -f ${FN1} ${FN2} ${FN3} ${FN4}
    exit 1
}

rm -f ${FN1} ${FN2} ${FN3} ${FN4}
exit 0