#!/bin/bash
#

#kill $(ps -ef |grep 'OTDRServer' | awk '{print $2}')

#kill $(ps -ef | grep'otdrd'| awk '{print $2}')
ps -ef | grep 'OTDRServer' | awk '{print $2}' | xargs kill
ps -ef | grep 'otdrd' | awk '{print $2}' | xargs kill


PID1=`ps -ef | grep 'OTDR' `

PID2=`ps -ef | grep 'otdrd'`

echo "$PID1"

echo "$PID2"

exit 0


