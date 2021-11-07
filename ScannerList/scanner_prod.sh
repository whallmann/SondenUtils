rtl_power -f 402M:404M:1000 -d 2 -g 38 -p 0 /tmp/scan402404.csv -1 > /dev/null 2>&1
cd /home/wolfgang/scanner
./scannerlist -q 0 -n 4 -L -30 -H /tmp/holding402404.txt -o /tmp/outlist402404.txt -f /tmp/scan402404.csv -v -d /home/wolfgang/scanner/scannerblacklist.txt -w /home/wolfgang/scanner/scannerwhitelist402404.txt  > /tmp/scannerlist.log
echo Copy to Debian-Laptop -auch WS- >> /tmp/scannerlist.log
cp /tmp/outlist402404.txt /home/wolfgang/ws/sdrcfg-rtl0.txt >> /tmp/scannerlist.log
echo ---SCAN 402 to 404 complete ----------------  >> /tmp/scannerlist.log
sleep 2
rtl_power -f 404M:406M:1000 -d 2 -g 38 -p 0 /tmp/scan404406.csv -1 > /dev/null 2>&1
cd /home/wolfgang/scanner
./scannerlist -q 0 -n 4 -L -30 -H /tmp/holding404406.txt -o /tmp/outlist404406.txt -f /tmp/scan404406.csv -v -d /home/wolfgang/scanner/scannerblacklist.txt -w /home/wolfgang/scanner/scannerwhitelist404406.txt  >> /tmp/scannerlist.log
echo Copy to Debian-Laptop -auch WS- >> /tmp/scannerlist.log
cp /tmp/outlist404406.txt /home/wolfgang/ws/sdrcfg-rtl1.txt >> /tmp/scannerlist.log
echo ---SCAN 404 to 406 complete ----------------  >> /tmp/scannerlist.log
#sleep 2
#rtl_power -f 400M:402M:1000 -d 2 -g 38 -p 0 /tmp/scan401402.csv -1 > /dev/null 2>&1
#cd /home/wolfgang/scanner
#./scannerlist -q 0 -n 4 -L -30 -H /tmp/holding401402.txt -o /tmp/outlist401402.txt -f /tmp/scan401402.csv -v -d /home/wolfgang/scanner/scannerblacklist.txt -w /home/wolfgang/scanner/scannerwhitelist401402.txt  >> /tmp/scannerlist.log
#echo Copy to Debian-Laptop -auch WS- >> /tmp/scannerlist.log
#cp /tmp/outlist401402.txt /home/wolfgang/ws/sdrcfg-rtl2.txt >> /tmp/scannerlist.log
#echo ---SCAN 401 to 402 complete ----------------  >> /tmp/scannerlist.log
echo ---Reset USB Device --- >> /tmp/scannerlist.log
#/usr/sbin/usb_modeswitch -w 2 -s2 -b 6 -g 2 -R -v 0bda -p 2838 -V 0bda -P 2838  >> /tmp/scannerlist.log
busnum="`udevadm info -q property -n /dev/rtl_sdr |grep "BUSNUM" | awk -F= '{print $2}'`"
echo 'Erkannte BusNummer:'  >> /tmp/scannerlist.log
echo $busnum >> /tmp/scannerlist.log
/usr/sbin/usb_modeswitch -w 2 -s2 -b $busnum -g 2 -R -v 0bda -p 2838 -V 0bda -P 2838  >> /tmp/scannerlist.log
echo ---Done--- >> /tmp/scannerlist.log
