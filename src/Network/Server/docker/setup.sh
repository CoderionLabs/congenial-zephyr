#!/bin/bash

#cd /path/of/my/stuff

tab=" --tab"
options=()

cmds[1]="docker start -a -i demspkg0"
cmds[2]="docker start -a -i dems0"
cmds[3]="docker start -a -i dems1"
cmds[4]="docker start -a -i dems2"
cmds[5]="docker start -a -i dems3"

for i in 1 2 3 4 5; do
options+=($tab -e "bash -c '${cmds[i]} ; bash'" )
done

gnome-terminal "${options[@]}"

exit 0