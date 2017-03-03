#set `ifconfig`
#shift 8
#k=`expr length $1`
#echo $k
#g=`expr $k - 5`
#echo $g
#s=${1:5:g}
#echo $s > ipaddress
set `ifconfig | awk '/inet addr/{print substr($2,6)}'`
echo $2 > ipaddress
