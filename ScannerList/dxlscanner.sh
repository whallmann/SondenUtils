#!/bin/bash

workingdir="$( cd "$(dirname "$0")" ; pwd -P )"

bindir=${workingdir}/bin                # Pfad zur Anwendungen
tempdir=/tmp               				# Pfad zum Temporären Verzeichnis
RTLPOWER=rtl_power                      # Pfad zur Anwendung von rtl_power
USBRESET=/usr/sbin/usb_modeswitch       # Pfad zur Anwendung usb_modeswitch
DXL=~/ws								# Pfad zur DXL Software (ziel für die sdrcfg Dateien)

SCANNER=${workingdir}/scannerlist           # Pfad zur Scanner-Anwendung

SCANNERDEVICE=2                         # SDR-Device ID 2 = third Stick
SCANNERGAIN=38                          # SDR Gain
SCANNERPPM=0                            # SDR Frequenzkorrektur (Mit TCXO sehr wahrscheinich 0)

SQUELCH=0                               # Squelch Level
SIGNAL=-30                              # Manuelle Signalerkennung, Empfangswerte die über dem angegebenen Pegel liegen werden als Signal erkannt.

function debugmsg () {
    green=`tput setaf 2`
    reset=`tput sgr0`
    tnow=`date "+%x_%X"`
    echo ${green}$tnow${reset} $1 
    echo $1 >> ${tempdir}/scannerlist.log
}

command -v ${SCANNER} >/dev/null 2>&1 || { debugmsg "Ich vermisse " ${SCANNER} >&2; exit 1; }
command -v ${RTLPOWER} >/dev/null 2>&1 || { debugmsg "Ich vermisse " ${RTLPOWER} >&2; exit 1; }
command -v ${USBRESET} >/dev/null 2>&1 || { debugmsg "Ich vermisse " ${USBRESET} >&2; exit 1; }

# Log neu aufsetzen
echo "." > ${tempdir}/scannerlist.log
debugmsg "Starte ScannerScript..."

${RTLPOWER} -f 402M:404M:1000 -d ${SCANNERDEVICE} -g ${SCANNERGAIN} -p ${SCANNERPPM}  ${tempdir}/scan402404.csv -1 > /dev/null 2>&1
${SCANNER} -q 0 -n 4 -L ${SIGNAL} -H ${tempdir}/holding402404.txt -o ${tempdir}/outlist402404.txt -f ${tempdir}/scan402404.csv -v -d ${workingdir}/scannerblacklist.txt -w ${workingdir}/scannerwhitelist402404.txt  > ${tempdir}/scannerlist.log
cp ${tempdir}/outlist402404.txt ${DXL}/sdrcfg-rtl1.txt
debugmsg "---Scanvorgang für 402 to 404 fertig..."

sleep 2

${RTLPOWER} -f 404M:406M:1000 -d ${SCANNERDEVICE} -g ${SCANNERGAIN} -p ${SCANNERPPM}  ${tempdir}/scan404406.csv -1 > /dev/null 2>&1
${SCANNER} -q 0 -n 4 -L ${SIGNAL} -H ${tempdir}/holding404406.txt -o ${tempdir}/outlist404406.txt -f ${tempdir}/scan404406.csv -v -d ${workingdir}/scannerblacklist.txt -w ${workingdir}/scannerwhitelist404406.txt  > ${tempdir}/scannerlist.log
cp ${tempdir}/outlist404406.txt ${DXL}/sdrcfg-rtl0.txt
debugmsg "---Scanvorgang für 404 to 406 fertig..."
sleep 2

debugmsg "---Reset USB Device ---"
#/usr/sbin/usb_modeswitch -w 2 -s2 -b 6 -g 2 -R -v 0bda -p 2838 -V 0bda -P 2838  >> /tmp/scannerlist.log
busnum="`udevadm info -q property -n /dev/rtl_sdr |grep "BUSNUM" | awk -F= '{print $2}'`"
debugmsg "Erkannte BusNummer:"
debugmsg $busnum 
#------ Werte aus dem Befehl "lsusb" übertragen ----------------
# ... Muster:
# Bus 006 Device 002: ID 0bda:2838 Realtek Semiconductor Corp. RTL2838 DVB-T
# busnum - hier 006 wird mit dem Befehl weiter oben ausgelesen
# -g - hier 2 ist das was hinter "Device" angegeben wird
# die Parameter -v/-p und -V/-P müssen zueinander identisch sein. So wird nochmal geprüft ob der Reset erfolgreich war.
${USBRESET} -w 2 -s2 -b $busnum -g 2 -R -v 0bda -p 2838 -V 0bda -P 2838  >> /tmp/scannerlist.log
debugmsg "---Done---"
