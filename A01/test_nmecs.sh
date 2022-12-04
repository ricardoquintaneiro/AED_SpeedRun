#!/bin/bash

for n in {1..200000}
do
    echo -ne "\rA testar para $n..." | sed -e "s/12345678/${replace}/g"
    ./speed_run $n
    if [ $? == 1 ]; then
        echo "ERRO para $n"
        exit 1
    fi
done
echo ""