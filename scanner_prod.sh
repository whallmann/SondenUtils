rtl_power -f 402M:404M:1000 -d0 -g 38 -p 28 /tmp/scan402404.csv -1 2>&1 > /dev/null
cd /home/pi
sudo ./scannerlist -L -30 -H /tmp/holding402404.txt -o /tmp/outlist402404.txt -f /tmp/scan402404.csv -v -d /home/pi/scannerblacklist.txt -w /home/pi/scannerwhitelist402404.txt  > /tmp/scannerlist.log
echo --------------------------------------------  >> /tmp/scannerlist.log
scp /tmp/outlist402404.txt pi@192.168.2.133:/home/pi/dxlAPRS/sdrcfg-rtl0.txt  >> /tmp/scannerlist.log
echo --------------------------------------------  >> /tmp/scannerlist.log
rtl_power -f 404M:406M:1000 -d0 -g 38 -p 28 /tmp/scan404406.csv -1 2>&1 > /dev/null
cd /home/pi
sudo ./scannerlist -L -30 -H /tmp/holding404406.txt -o /tmp/outlist404406.txt -f /tmp/scan404406.csv -v -d /home/pi/scannerblacklist.txt -w /home/pi/scannerwhitelist404406.txt  >> /tmp/scannerlist.log
echo --------------------------------------------  >> /tmp/scannerlist.log
scp /tmp/outlist404406.txt pi@192.168.2.118:/home/pi/dxlAPRS/sdrcfg-rtl0.txt >> /tmp/scannerlist.log
echo -------------------------------------------- >> /tmp/scannerlist.log
