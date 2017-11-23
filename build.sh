#!/bin/bash
make clean
qmake ./OTDRServer.pro
make
make clean
qmake ./otdrd.pro
make
echo -e "\e[1;41;37mKaterina, target complite\e[0m"
echo -e "\e[1;45;37mIf we, target complite executiv files there are in folder \"build\". Execute: \"sudo ./OTDRServer\"\e[0m"