#!/bin/bash
POWERSUPPLY="/sys/class/power_supply/AC0/online"
TOO_LOW="58"
NOT_CHARGING="0"
export DISPLAY=:0
export USER=globik

while [ 1 ]
do
BATTERY_LEVEL=$(acpi -b | grep -P -o '[0-9]+(?=%)') 
STATUS="$(cat $POWERSUPPLY)"
if [  "$BATTERY_LEVEL"  -le  28  -a  "$STATUS" = 0  ]
#if [ "$BATTERY_LEVEL" -le 36 ]
#if [  "$STATUS" = 12  ]
then
#echo "baterry_level: $BATTERY_LEVEL"
#echo "status: $STATUS"
/usr/bin/notify-send -t 5000  "battery low" "battery level"
/usr/bin/aplay -q  /usr/share/sounds/freedesktop/stereo/bell.wav 
fi
#echo "status 0"
sleep 10
done
#exit 0

# function getrunningprocess
#{
#ps -aux | grep -v grep | grep battery.sh
#}
#if [[ -n "$(getrunningprocess)"  ]] ; then
#exit
#fi
#battery.sh &