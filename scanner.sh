rtl_power -f 400M:406M:1000 -d0 -g 38 -p 28 /tmp/scan.csv -1 2>&1 > /dev/null
cd /home/pi
sudo ./scannerlist -L -30 -H /tmp/holding.txt -o /tmp/outlist.txt -f /tmp/scan.csv -v -d /home/pi/scannerblacklist.txt -w /home/pi/scannerwhitelist.txt  > /tmp/scannerlist.log

