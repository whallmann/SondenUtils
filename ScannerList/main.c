/*
 * ScannerList for rtl_power (csv) nach txt aufbereiten
 *
 * Copyright (C) Wolfgang Hallmann <df7pn@darc.de>
 *
 * License-Identifier:	GPL-2.0+
*/

/*
 *  parameters:
 * -v                   Verbose level
 * -f <rtlcsvfile>      Input her, written by rtl_power
 * -o <sdrcfg-file>     Output file, dxlAPRS conform frequency list for the sdrtst receiver [f <frqnz> <afc> 0 0 <bandwidth>]
 * -d <sdrcfg-file>     Default: <noFile> Blacklist of frequencies. Will never put in output file -o
 * -w <sdrcfg-file>     Default: <noFile> Whitelist of frequencies. Always added to output file -o
 * -a <afc-Value>       Default: 5 / Value in KHz to follow the signal up and down if drifting
 * -b <BandwithHz>      Default: will be calculated / otherwise you can define a constant value. Will be used in sdrcfg-file (-o)
 * -L <above signallevel>Default: -20 / Units in db, detector for activities only for signals who are above this level
 * -h <minutes>         Default: 10. Holds frequencies with action for that minutes if there is a gap for a short time
 * -H <holdingfilename> file name nessesery for holding the frequencies with timestamps.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PUFFERLEN 32767
#define SCHWELLWERT -30
#define BANDWITHDEFAULT 8000
#define HOLDINGMINUTES 10
#define clrscr() printf("\x1B[2J")
#define MAXARRAY 200

char static datetimestamp [30];
struct tm *tmnow;
time_t static t;

void getTime()
{
    time(&t);
    tmnow = localtime(&t);
    sprintf(datetimestamp, "%02d.%02d.%d %02d:%02d:%02d",
    tmnow->tm_mday, tmnow->tm_mon + 1, tmnow->tm_year + 1900, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);
}


int main(int argc, char *argv[])
{

//----------- Übergabeparameter auslesen und merken ----------------
    // Default-Werte besetzen
    char VERSIONNUMB[20] = {"0.9"};
    char VERSIONDATE[20] = {"2017-03-05"};

    int arg_verbose = 0;
    char arg_BlacklistFile [255];
    arg_BlacklistFile[0] = 0;
    char arg_WhitelistFile [255];
    arg_WhitelistFile[0] = 0;
    int arg_afc = 5;
    long arg_bandwithHz = 0;
    int arg_signallevel = -20;
    char arg_outputfilename [255];
    arg_outputfilename[0] = 0;
    char arg_inputfilename [255];
    arg_inputfilename[0] = 0;
    char arg_holdingfilename [255];
    arg_holdingfilename[0] = 0;
    int arg_holdingtimer = 10;

    int argindex = 0;
    //-------------------- give help -------------------
    if ( ((argc==2) && (argv[1]="--help")) || (argc==1))
    {
      //clrscr();
      fputs("=======================================================",stdout); fputs("\n",stdout);
      fputs("!          Frequency Scanner for dxl-Sonde chain      !",stdout); fputs("\n",stdout);
      fputs("!          by Wolfgang Hallmann, DF7PN @ DARC.DE      !",stdout); fputs("\n",stdout);
      fputs("!          Version ",stdout);
      fputs(VERSIONNUMB,stdout);
      fputs("                                !",stdout); fputs("\n",stdout);
      fputs("!          Date    ",stdout);
      fputs(VERSIONDATE,stdout);
      fputs("                         !",stdout); fputs("\n",stdout);
      fputs("=======================================================",stdout); fputs("\n",stdout);
      fputs("This program uses the output of rtl_power and detects  ",stdout); fputs("\n",stdout);
      fputs("weather sonde signals, put them into a sdrcfg-file and ",stdout); fputs("\n",stdout);
      fputs("holds frequencies for a defined time, if there is a gap ",stdout); fputs("\n",stdout);
      fputs("on reception. More usage hints, in README.TXT ",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("Parameters, and what they are good for:",stdout); fputs("\n",stdout);
      fputs("-a <afc-Wert>",stdout); fputs("\n",stdout);
      fputs(":: Default: 5. Value is in KHz. Second parameter in",stdout); fputs("\n",stdout);
      fputs("   sdrcfg.txt file. Automatic frequency control.       ",stdout); fputs("\n",stdout);
      fputs("   decoder will follow signal up or down within Value ",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-b <BandwithInHerz>",stdout); fputs("\n",stdout);
      fputs(":: Default: calculating. Units in Herz. Overwrite is",stdout); fputs("\n",stdout);
      fputs("   possible with -b. Otherwise calc. bandwith will be used.",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-d <file name>",stdout); fputs("\n",stdout);
      fputs(":: Default: <noFile>. If announced, this frequencies will",stdout); fputs("\n",stdout);
      fputs("   be filtered in output (-o) list (blacklist)",stdout); fputs("\n",stdout);
      fputs("   One frequency per line. Units in KHz (ex: 402700)",stdout); fputs("\n",stdout);
      fputs("   Round the frequency up/down to next full 10 KHz. e.g. 400237 -> 400240",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-f <rtlcsv file name>",stdout); fputs("\n",stdout);
      fputs(":: Default: NO. You have to give a file name for input.",stdout); fputs("\n",stdout);
      fputs("   Its the output file from rtl_power. Format: CSV. ",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-h <value minutes>",stdout); fputs("\n",stdout);
      fputs(":: Default: 10. Unit is minutes. Give a value other than",stdout); fputs("\n",stdout);
      fputs("   this, to hold signals for a while in list (-o)",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-H <file name>",stdout); fputs("\n",stdout);
      fputs(":: Default: NO. You have to give a file name.",stdout); fputs("\n",stdout);
      fputs("   This file will contains the list of activ frequencies",stdout); fputs("\n",stdout);
      fputs("   and there last-time-heard.",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-L <levelAboveNoise>",stdout); fputs("\n",stdout);
      fputs(":: Default: -20. Unit is db (negative value). Only",stdout); fputs("\n",stdout);
      fputs("   Signals above this value will be taken.",stdout); fputs("\n",stdout);
      fputs("   Sample: noise floor aprox. -35 db. -L -20. Signal ",stdout); fputs("\n",stdout);
      fputs("   has a value of -15 (very loud). This will be taken.",stdout); fputs("\n",stdout);
      fputs("   Set this to -L -50 if you like to check each noise.",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-o <sdrcfg file name>",stdout); fputs("\n",stdout);
      fputs(":: Default: NO. You have to give a file name for output.",stdout); fputs("\n",stdout);
      fputs("   Its the output file for the dxlChain. This frequencies",stdout); fputs("\n",stdout);
      fputs("   get watched. Write it to the dxlAPRS path. ",stdout); fputs("\n",stdout);
      fputs("   Gets immeadeadly active!",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-v ",stdout); fputs("\n",stdout);
      fputs(":: Verbose infos on console.",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("-w <file name>",stdout); fputs("\n",stdout);
      fputs(":: Default: <noFile>. If announced, this frequencies will",stdout); fputs("\n",stdout);
      fputs("   always be delivered in output (-o) list (whitelist)",stdout); fputs("\n",stdout);
      fputs("   One frequency per line. Units in KHz (ex: 402700)",stdout); fputs("\n",stdout);
      fputs("   Round the frequency up/down to next full 10 KHz. e.g. 400699 -> 402700",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("----------- Samples ---------------------------------",stdout); fputs("\n",stdout);
      fputs("sudo ./scannerlist -L -50 -H /tmp/holding.txt -o ~/dxlAPRS/sdrcfg.txt",stdout); fputs("\n",stdout);
      fputs("     -f /tmp/scan.csv -v >> /tmp/scannerlist.log",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      fputs("Usage hint of rtl_power:",stdout); fputs("\n",stdout);
      fputs("rtl_power -f 400M:406M:1000 -d0 -g 38 -p 56 /tmp/scan.csv -1 2>&1 > /dev/null",stdout); fputs("\n",stdout);
      fputs(":: Use a steprate (bin-count) of 1000. Unit is Herz. -d = Devicenbr,",stdout); fputs("\n",stdout);
      fputs("   -g is gain, -p is correction value",stdout); fputs("\n",stdout);
      fputs("   of the dvbt-stick, scan.csv names the output file and format.",stdout); fputs("\n",stdout);
      fputs("",stdout); fputs("\n",stdout);
      //fgetc(stdin);
      return EXIT_FAILURE;
    }

    //--------------------------------------------------
    printf("Argv[0] =%s\n",argv[0]);
    for(argindex=1; argindex < argc; argindex++)
    {
      printf("%02i=%s\n",argindex,argv[argindex]);
      // 0 : programmname
      if (strcmp(argv[argindex],"-v") == 0)
      {
        arg_verbose = 1;
      }
      if (strcmp(argv[argindex],"-f") == 0)
      {
         if(argindex+1 < argc)
         {
           strcpy(&arg_inputfilename[0], argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-H") == 0)
      {
         if(argindex+1 < argc)
         {
           strcpy(&arg_holdingfilename[0], argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-o") == 0)
      {
         if(argindex+1 < argc)
         {
           strcpy(&arg_outputfilename[0], argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-d") == 0)
      {
         if(argindex+1 < argc)
         {
           strcpy(&arg_BlacklistFile[0], argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-w") == 0)
      {
         if(argindex+1 < argc)
         {
           strcpy(&arg_WhitelistFile[0], argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-a") == 0)
      {
         if(argindex+1 < argc)
         {
           arg_afc = atoi(argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-b") == 0)
      {
         if(argindex+1 < argc)
         {
           arg_bandwithHz = atol(argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-L") == 0)
      {
         if(argindex+1 < argc)
         {
           arg_signallevel = atoi(argv[argindex+1]);
         }
      }
      if (strcmp(argv[argindex],"-h") == 0)
      {
         if(argindex+1 < argc)
         {
           arg_holdingtimer = atoi(argv[argindex+1]);
           if (arg_holdingtimer < 0) arg_holdingtimer = HOLDINGMINUTES;
         }
      }
    } // for argv
    //--- Testweise Ausgbe der Werte und Abort
    if (arg_verbose)
    {
      fputs("\nStart at: ",stdout);
      getTime();
      fputs(datetimestamp,stdout);
      printf("\nVersion: %s   released at: %s\n",VERSIONNUMB,VERSIONDATE);
      fputs("\n--------- Parameter -------------\n",stdout);
      printf("-v %i \n", arg_verbose);
      printf("-d %s \n", arg_BlacklistFile);
      printf("-w %s \n", arg_WhitelistFile);
      printf("-a %i \n", arg_afc);
      printf("-b %ld \n", arg_bandwithHz);
      printf("-L %i \n", arg_signallevel);
      printf("-o %s \n", arg_outputfilename);
      printf("-f %s \n", arg_inputfilename);
      printf("-H %s \n", arg_holdingfilename);
      printf("-h %i \n", arg_holdingtimer);
      fputs("\n--------- Parameter -------------\n",stdout);
    }
    int filefound = 0;
    int needabort = 0;
    FILE *f;
    //--- Check: testing write to outputfile
    if (strlen(arg_outputfilename) > 0)
    {
      f = fopen(arg_outputfilename,"r"); // try to read. maybe it is already there
      if (f == NULL)
      {
        f = fopen(arg_outputfilename,"at"); // test if we can write (simply the content will be destroyed)
        if (f != NULL) filefound = 1;
      }
      else filefound = 1;
    }
    if(!filefound)
    {
      fputs("WARNING: Output file error (-o)\n",stdout);
      needabort = 1;
    }
    else
      fclose(f);

    //--- Check: testing write to holdingfilename
    if (strlen(arg_holdingfilename) > 0)
    {
      f = fopen(arg_holdingfilename,"r"); // try to read. maybe it is already there
      if (f == NULL)
      {
        f = fopen(arg_holdingfilename,"at"); // test if we can write (simply the content will be destroyed)
        if (f != NULL) filefound = 1;
      }
      else filefound = 1;
    }
    if(!filefound)
    {
      fputs("WARNING: Cannot create HoldingFile, Error (-H)\n",stdout);
      needabort = 1;
    }
    else
      fclose(f);


    //--- Check: input file exists
    filefound = 0;
    if (strlen(arg_inputfilename) > 0)
    {
      f = fopen(arg_inputfilename,"rt");
      if (f != NULL) filefound = 1;
    }
    if(!filefound)
    {
      fputs("WARNING: No Input CSV file name found (-f)\n",stdout);
      needabort = 1;
    }
    else
      fclose(f);

    //--- Check: input file exists
    filefound = 0;
    if (strlen(arg_BlacklistFile) > 0)
    {
      f = fopen(arg_BlacklistFile,"rt");
      if (f != NULL) filefound = 1;
    }
    if(!filefound)
    {
      if (arg_verbose) fputs("ADVICE: The file with Blacklist frequencies not found (-d)\n",stdout);
      // needabort = 1;
    }
    else
      fclose(f);

    //--- Check: input file exists
    filefound = 0;
    if (strlen(arg_WhitelistFile) > 0)
    {
      f = fopen(arg_WhitelistFile,"rt");
      if (f != NULL) filefound = 1;
    }
    if(!filefound)
    {
      if (arg_verbose) fputs("ADVICE: The file with Whitelist frequencys not found (-w)\n",stdout);
      // needabort = 1;
    }
    else
      fclose(f);

    if(needabort)
    {
      fputs("ERROR: Program aborts. To few good parameters\n",stdout);
      //fgetc(stdin);
      return EXIT_FAILURE;
    }

//------------------------------------------------------------------
    static char zeile[PUFFERLEN];
    zeile[0] = 0;
    char *zeiger;
    int i = 1;
    int j = 0;
    int myint = -1;
    // int dBListe [3000];  // for future use maybe
    int Anzahl_dbListe = 0;  // 1...n
    double QrgListe [MAXARRAY];
    int Anzahl_QrgListe = 0;
    double NewQrgListe [MAXARRAY];
    int Anzahl_NewQrgListe = 0;
    double NewQrgBreite [MAXARRAY];
    int Anzahl_NewQrgBreite = 0;

    double HoldingQrgListe [MAXARRAY];
    int Anzahl_HoldingQrgListe = 0;
    double HoldingQrgBreite [MAXARRAY];
    int Anzahl_HoldingQrgBreite = 0;
    double HoldingTimestamp [MAXARRAY];
    int Anzahl_HoldingTimestamp = 0;

    double rundung;
    double startfrequenz = 0.0f;
    double realIntervall = 0;
    double untereQRG, obereQRG;
    int indexQRGblock;

    f = fopen(arg_inputfilename,"rt");
    if (f != NULL)
    {
      // read fist line
      fgets(&zeile[0], PUFFERLEN-2, f);

      //MoreVerbose: fputs(zeile, stdout);
      if(arg_verbose) fputs("--- read line \n", stdout);
      // separate with delimitter ","
      while ((feof(f) == 0) && (Anzahl_QrgListe < MAXARRAY))
      {
        // fputs("\n---> No EOF\n", stdout);
        i = 1;
        zeiger = strtok(zeile,",");
        while(zeiger != NULL)
        {
          if (i==3)
          {
            startfrequenz = atof(zeiger);
            // printf("start frequ.: %9.0f",startfrequenz);
          }
          if (i==5)
          {
            realIntervall = atof(zeiger);
          }
          if (i>=7)   // data fields in csv starts here
          {
            rundung = atof(zeiger);
            rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
            myint = rundung;
            //Save in dBListe we need later, if we going to create waterfall diagramms etc.
            //also need all data for calculate noise floor
            //dBListe[Anzahl_dbListe] = myint;
            if(myint >= arg_signallevel)
            {
              // calculate frequency with origin intervall from csv file
              QrgListe[Anzahl_QrgListe] = startfrequenz + realIntervall * (i-6);
              // shows already rounded data  ==> printf("%02d: %9.0f\n",Anzahl_QrgListe,QrgListe[Anzahl_QrgListe]);
              rundung = QrgListe[Anzahl_QrgListe]/1000.0f;  // wegen HZ in KHZ
              rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
              // -------- frequ. now rounded to unit "KHz"
              QrgListe[Anzahl_QrgListe] = rundung; //  /100.0f;

              if (arg_verbose) printf("%02d: %6.0f\n",Anzahl_QrgListe,QrgListe[Anzahl_QrgListe]);
              Anzahl_QrgListe++;
            }
            Anzahl_dbListe++;
            //if (i % 10 == 0) printf("\n");
          }
          zeiger = strtok(NULL, ",");
          i++;
        }
        // nächste Zeile lesen
        if(arg_verbose) fputs("--- read line \n", stdout);
        fgets(&zeile[0], PUFFERLEN-2, f);
      }
      fclose(f);
      if (arg_verbose) printf("\nEntries in frequency list: %i\n\n",Anzahl_QrgListe);

    }
    //--------------------------------------------------------------
    // Frequenzliste nun nach Sonden-Signalen durchsuchen
    // Erkennungsmuster
    // Vom aktuellen Signal aus den Array scannen
    // nächste QRG darf nicht weiter als 2 KHz entfernt sein, sonst diese als neuen Startwert nehmen
    // Unteren Wert merken wenn nächstes Signal max 2 KHz entfernt
    // weiter im Index suchen bis Signal mehr als 2 KHz entfernt
    // dann aktuellen Wert als Obergrenze nehmen
    // Signalbreite ausrechnen und Mittelwert bestimmen
    // Mittelwert auf nächste 10 KHz runden, da Sondenraster = 10 Khz ist
    // Diese Frequenz in neue Liste aufnehmen und Signalbreite dazu merken (für SDRCFG-Textdatei später benötigt)
    // ---ENDE DER SCHLEIFE ---
    // Neue SDRCFG-Datei bilden (später optional eine Differenz von bekannten Frequenzen die eh dauerüberwacht werden bilden)
    //--------------------------------------------------------------

    if(Anzahl_QrgListe > 0)
    {
      i = 0;  // erster Index der QrgListe
      indexQRGblock = 0;   // von hier ab solange in QrgListe suchen bis Listenende oder nächste QRG >= 3 khz Abstand
      while ((i < Anzahl_QrgListe) && (Anzahl_NewQrgListe < MAXARRAY))
      {
        untereQRG = QrgListe[indexQRGblock];
        if (arg_verbose) printf(":: lower f %9.0f\n",untereQRG);

        //     noch ein Index hinter dem aktuellen?     Ist die nächste QRG dicht genug dran?
        while((indexQRGblock < Anzahl_QrgListe-1) && ((QrgListe[indexQRGblock+1] - QrgListe[indexQRGblock]) < 3.0f))
        {
          indexQRGblock++;
        }
        if (arg_verbose) printf(":: upper f %9.0f, at index %i, \n",QrgListe[indexQRGblock],indexQRGblock);
        // Hier angekommen: QRG Diff zu hoch oder Listenende
        // Ab 3 hintereinander folgende Frequenzen wird diese in die NewQRGListe mit ihrem Mittelwert übernommen
        if ((indexQRGblock - i) >= 3)  // war mal 2
        {
          if (arg_verbose) fputs("big signal detected with more or equal 3 nearby frequ.\n",stdout);
          obereQRG = QrgListe[indexQRGblock];
          rundung = (obereQRG - untereQRG);
          // Bandbreite aufheben - gleicher Index wie NewQRG
          NewQrgBreite[Anzahl_NewQrgBreite] = rundung;
          Anzahl_NewQrgBreite++;
          rundung = rundung / 2;
          rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
          NewQrgListe[Anzahl_NewQrgListe] = untereQRG + rundung;
          Anzahl_NewQrgListe++;
          if (arg_verbose) printf("::take center frequ. %9.0f\n",NewQrgListe[Anzahl_NewQrgListe-1]);
        }
        indexQRGblock++;
        i = indexQRGblock;
        if (arg_verbose) printf(":: go ahead on index %i\n",i);
      }
      if (arg_verbose) printf("\n%i wide signals saved in list\n",Anzahl_NewQrgListe);
      for(i=0;i<Anzahl_NewQrgListe;i++)
      {
        if (arg_verbose) printf("%6.0f - %3.0f\n",NewQrgListe[i],NewQrgBreite[i]);
      }
    }
    else
    {
        fputs("\nINFO: No frequencies detected\n",stdout);
        return EXIT_FAILURE;
    }
    /* ----------------------------------------------------------
    Frequenzen, die einmal erkannt wurden, sollen für x-Minuten gehalten werden.
    Grund: Es gibt immer mal kurze Einbrüche die beim RTL-POWER Scan herausfallen
    können und somit bis zum nächsten Scan herausfallen.
    Howto:
    - Hilfsdatei benutzen, da Programm nach einem Durchlauf beendet wird
    - Hilfsdatei sammelt Frequenzen mit TimeStamp
    - a) Hilfsdatei lesen
    - b) aktuelle QRGs dort hinein ergänzen:
         Schon vorhanden, dann Timestamp erneuern
         Nicht vorhanden, mit akt-Timestamp anhängen
    - c) alle QRGs entfernen, deren Timestamp älter als x-Minuten liegt
    - d) Hilfsdatei überschreiben
    - e) sdrcfg-Datei bilden aus Hilfsliste
    + Parameter für Verfallzeit aufnehmen:  -h (steht für "holding")

    ------------------------------------------------------------- */

    //-------- HoldingDatei lesen ----------------
    // Besteht aus: <Frequenz:6><Blank><Bandbreite:3> in KHz
    char tmpStr[255];
    tmpStr[0]=0;
    if(arg_verbose) fputs("--- read Holding file \n", stdout);
    f = fopen(arg_holdingfilename,"rt");
    if (f != NULL)
    {
      // erste Zeile lesen
      fgets(&zeile[0], PUFFERLEN-2, f);

      // gibt die ganze Zeile aus
      //MoreVerbose: fputs(zeile, stdout);
      // Zerlegen in Teile mit "," getrennt
      while ((feof(f) == 0) && (Anzahl_HoldingQrgListe < MAXARRAY))
      {
        if(arg_verbose)
        {
          printf("Read line: %s", zeile);
        }
        // fputs("\n---> Kein EOF\n", stdout);
        // Besteht aus: <Frequenz:6><Blank><Bandbreite:3><Blank><TimeStampInSekunden>
        if (strlen(zeile) >= 10)
        {
          strncpy(&tmpStr[0],&zeile[0],6);
          tmpStr[6] = 0;
          HoldingQrgListe [Anzahl_HoldingQrgListe] = atof(tmpStr);

          strncpy(&tmpStr[0],&zeile[7],3);
          tmpStr[3] = 0;
          HoldingQrgBreite[Anzahl_HoldingQrgListe] = atof(tmpStr);

          strcpy(&tmpStr[0],&zeile[11]);
          HoldingTimestamp[Anzahl_HoldingTimestamp] = atof(tmpStr);

          if(arg_verbose)
          {
            printf("%03i: separated: <%6.0f> <%3.0f> <%f>\n",
                   Anzahl_HoldingQrgListe+1,
                   HoldingQrgListe [Anzahl_HoldingQrgListe],
                   HoldingQrgBreite[Anzahl_HoldingQrgListe],
                   HoldingTimestamp[Anzahl_HoldingTimestamp] );
          }
          Anzahl_HoldingQrgListe++;
          Anzahl_HoldingQrgBreite++;
          Anzahl_HoldingTimestamp++;
        }
        fgets(&zeile[0], PUFFERLEN-2, f);
      }
      fclose(f);

    }
    else
    {
      fputs("No Holding file found - closing\n",stdout);
      return EXIT_FAILURE;
    }
    //--------- Holdingliste gelesen -
    // Prüfen, ob Einträge veraltet und löschen
    time(&t);
    double diffInSeconds = arg_holdingtimer * 60; // Sekunden
    // vor n-Minuten war es...
    // alle Timer in der Liste prüfen ob sie älter (kleine) n-Minuten sind, dann entfernen, weil kein Update mehr kam
    t = t - diffInSeconds;
    if(Anzahl_HoldingTimestamp > 0)
    {
      for (i=0; i<Anzahl_HoldingTimestamp; i++)
      {
        if(HoldingTimestamp[i] < t)
        {
          if(arg_verbose)
            printf("%02i: outdated: %6.0f %03.0f %f\n",
                 i,
                 HoldingQrgListe [i],
                 HoldingQrgBreite[i],
                 HoldingTimestamp[i]);
          //zu alt - leeren
          HoldingQrgListe [i] = 0;
          HoldingQrgBreite[i] = 0;
          HoldingTimestamp[i] = 0;
        }
      }
    }
    char ausgabezeile[255];
    ausgabezeile[0] = 0;

    // Aktuelle Signale in HoldingListe updaten oder hinzufügen
    // Frequenzen werden auf volle 10 Khz auf/abgerundet
    if (Anzahl_NewQrgListe > 0)
    {
      int isInHoldingList = 0;
      for(i=0;i<Anzahl_NewQrgListe;i++)
      {
        isInHoldingList = 0;
        rundung = NewQrgListe[i] / 10;
        rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
        rundung = rundung * 10;
        for(j=0;j<Anzahl_HoldingQrgListe;j++)
        {
          if(rundung == HoldingQrgListe[j])
          {
            time(&t);
            HoldingTimestamp[j] = t;
            isInHoldingList = 1;
            if(arg_verbose)
              printf("%02i: update: %6.0f\n", j, HoldingQrgListe [j]);
          }
        }
        if(!isInHoldingList)
        {
          if (Anzahl_HoldingQrgListe < MAXARRAY)
          {
            // nicht in Liste, also anhängen in HoldingList
            HoldingQrgListe [Anzahl_HoldingQrgListe] = rundung;  // wurde oben gerunden auf volle 10 Khz
            HoldingQrgBreite[Anzahl_HoldingQrgListe] = NewQrgBreite[i];
            time(&t);
            HoldingTimestamp[Anzahl_HoldingTimestamp]= t;
            if(arg_verbose)
              printf("%02i: add: %6.0f\n", Anzahl_HoldingQrgListe+1, rundung);

            Anzahl_HoldingQrgListe++;
            Anzahl_HoldingQrgBreite++;
            Anzahl_HoldingTimestamp++;
          }
        }
      }  // for(i=0;i<Anzahl_NewQrgListe
    }  // if (Anzahl_NewQrgListe > 0

    // Holdingliste wegschreiben
    if(arg_verbose)
      fputs("---- write Holding file ---\n",stdout);
    f = fopen(arg_holdingfilename,"wt");
    if (f != NULL)
    {
      int somethingwritten = 0;
      for(i=0;i<Anzahl_HoldingQrgListe;i++)
      {
        // Leere Zeilen die gelöscht wurden fallen unter den Tisch
        if(HoldingQrgListe[i] > 0.0f)
        {
          sprintf(ausgabezeile, "%6.0f %03.0f %f\n", HoldingQrgListe[i], HoldingQrgBreite[i], HoldingTimestamp[i] );
          fputs(ausgabezeile, f);
          somethingwritten = 1;  // yes
        }
      }
      // if no data to write, make it really empty
      // On later read, all lines shorter then 10 chars are ignored
      if (!somethingwritten)
      {
        fputs(" \n",f);
      }
      fclose(f);
      if(arg_verbose)
        printf("Holdinglist written %i entries \n",Anzahl_HoldingQrgListe);

    }  // if (f != NULL)
    else
    {
      //if (arg_verbose)
      {
        fputs("ERROR: output file ",stdout);
        fputs(arg_holdingfilename,stdout);
        fputs("not written (-o)\n",stdout);
      }
    } // else if (f != NULL)

    /* --------------------------------------------------------------------
     * Blacklist Datei einlesen und aus Holdingfile löschen
    ----------------------------------------------------------------------- */
    // Frequencies to delete
    double Blacklist [MAXARRAY];
    int Anzahl_Blacklist = 0;
    // file name given with -d ?
    if (strlen(arg_BlacklistFile)>0)
    {
      if(arg_verbose) fputs("--- read Blacklist file \n", stdout);
      f = fopen(arg_BlacklistFile,"rt");
      if (f != NULL)
      {
        // erste Zeile lesen
        fgets(&zeile[0], PUFFERLEN-2, f);

        while (feof(f) == 0)
        {
          if(arg_verbose)
          {
            printf("Read line: %s", zeile);
          }
          if ((strlen(zeile) >= 6) && (Anzahl_Blacklist < MAXARRAY))
          {
            Blacklist [Anzahl_Blacklist] = atof(zeile);
            if(arg_verbose)
            {
              printf("%03i: Black: <%6.0f>\n", Anzahl_Blacklist+1, Blacklist[Anzahl_Blacklist]);
            }
            Anzahl_Blacklist++;
          }
          fgets(&zeile[0], PUFFERLEN-2, f);
        }
        fclose(f);
      }
      else
      {
        fputs("The Blacklist file does not exists? - closing\n",stdout);
        return EXIT_FAILURE;
      }
    } // if (strlen(arg_BlacklistFile)>0)

    // --- delete them from holding list
    int zaehler = 0;
    if (Anzahl_Blacklist > 0)
    {
      for (i=0; i<Anzahl_Blacklist; i++)
      {
        for(j=0; j<Anzahl_HoldingQrgListe;j++)
          {
            if(Blacklist[i]==HoldingQrgListe[j])
            {
              HoldingQrgListe[j] = 0;
              HoldingQrgBreite[j] = 0;
              HoldingTimestamp[j] = 0;
              zaehler++;
            }
          }
      }
      printf("%i blacklisted frequencies deleted from Holding list\n", zaehler);
    }

    /* --------------------------------------------------------------------
     * Whitelist Datei einlesen und in Holdingfile aufnehmen wenn nicht vorhanden
    ----------------------------------------------------------------------- */
    // Frequencies to add
    double Whitelist [MAXARRAY];
    int Anzahl_Whitelist = 0;
    double WhitelistWidth [MAXARRAY];
    int Anzahl_WhitelistWidth = 0;
    // file name given with -w ?
    if (strlen(arg_WhitelistFile)>0)
    {
      tmpStr[0]=0;
      if(arg_verbose) fputs("--- read Whitelist file \n", stdout);
      f = fopen(arg_WhitelistFile,"rt");
      if (f != NULL)
      {
        // erste Zeile lesen
        fgets(&zeile[0], PUFFERLEN-2, f);

        // Zerlegen in Teile mit "," getrennt
        while ((feof(f) == 0) && (Anzahl_Whitelist < MAXARRAY))
        {
          if(arg_verbose)
          {
            printf("Read line: %s", zeile);
          }
          // fputs("\n---> Kein EOF\n", stdout);
          // Besteht aus: <Frequenz:6><Blank><Bandbreite:3>
          if (strlen(zeile) >= 10)
          {
            strncpy(&tmpStr[0],&zeile[0],6);
            tmpStr[6] = 0;
            Whitelist [Anzahl_Whitelist] = atof(tmpStr);

            strcpy(&tmpStr[0],&zeile[7]);
            WhitelistWidth[Anzahl_WhitelistWidth] = atof(tmpStr);

            if(arg_verbose)
            {
              printf("%03i: Whitelist separated: <%6.0f> <%3.0f>\n",
                     Anzahl_Whitelist+1,
                     Whitelist [Anzahl_Whitelist],
                     WhitelistWidth[Anzahl_WhitelistWidth] );
            }
            Anzahl_Whitelist++;
            Anzahl_WhitelistWidth++;
          }
          fgets(&zeile[0], PUFFERLEN-2, f);
        }
        fclose(f);
      }
      else
      {
        fputs("The Whitelist file does not exists? - closing\n",stdout);
        return EXIT_FAILURE;
      }
    } // (strlen(arg_WhitelistFile)>0)
    //--- Add entries from Whitelist to Holdinglist if new
    zaehler = 0;
    int found = 0;
    if (Anzahl_Whitelist > 0)
    {
      for (i=0; i<Anzahl_Whitelist; i++)
      {
        found = 0;
        for(j=0; j<Anzahl_HoldingQrgListe;j++)
        {
          if (!found)
          {
            if(Whitelist[i]==HoldingQrgListe[j])
            {
              found = 1; //yes - is already in Holdinglist - do nothing
            }
          }
        }
        if (!found)
        {
          // adding to end of Holdinglist
          HoldingQrgListe[Anzahl_HoldingQrgListe] = Whitelist[i];
          HoldingQrgBreite[Anzahl_HoldingQrgBreite] = WhitelistWidth[i];
          HoldingTimestamp[Anzahl_HoldingTimestamp] = 0;
          Anzahl_HoldingQrgListe++;
          Anzahl_HoldingQrgBreite++;
          Anzahl_HoldingTimestamp++;
          zaehler++;
        }
      }
      printf("%i whitelisted frequencies added to Holding list\n", zaehler);
    }


    //-------- Holding-Datei als SDRCFG-Zieldatei schreiben ---------------
    ausgabezeile[0] = 0;
    double aktBandwidth = 0;
    if(arg_verbose)
      fputs("---- write sdrcfg file ---\n",stdout);

    f = fopen(arg_outputfilename,"wt");
    if (f != NULL)
    {
      getTime();
      fputs("# Created: ",f);
      fputs(datetimestamp,f);
      fputs("\n",f);

      for(i=0;i<Anzahl_HoldingQrgListe;i++)
      {
        // Leere Zeilen die gelöscht wurden fallen unter den Tisch
        if(HoldingQrgListe[i] > 0.0f)
        {
          // was angegeben als Parameter? dann nimm den für alle
          if (arg_bandwithHz)
          {
            aktBandwidth = arg_bandwithHz;
          }
          else
          {
            // autom. festgestellte Bandbreite weitergeben
            // KHz in Herz
            aktBandwidth = HoldingQrgBreite[i] * 1000;
            if(aktBandwidth <= BANDWITHDEFAULT)
            {
              // schwache Signale mit schnaler Bandbreite auf Minimum BANDWITHDEFAULT hoch setzen
              aktBandwidth = BANDWITHDEFAULT;
            }
            else
            {
              // Hohe Bandbreiten etwas reduzieren, weil i.d.R. durch lautes übersteuertes Signal bedingt
              if(aktBandwidth >= 11000.0f)  aktBandwidth = aktBandwidth - 3000.0f;
            }
          }
          // Dieses Format wird vom dxlChain als Input verlangt
          // Aufbau: Frequenz in MHZ, AFC+/-, unwichtig, unwichtig, Filterbreite in Herz
          sprintf(ausgabezeile, "f %6.3f %i 0 0 %-6.0f\n", HoldingQrgListe[i] / 1000.0f, arg_afc, aktBandwidth );
          if (arg_verbose)
            printf("%s",ausgabezeile);
          fputs(ausgabezeile, f);
        }
      }
      fclose(f);
    }
    else
    {
      if (arg_verbose) fputs("ERROR: output file not written (-o)",stdout);
    }

    if (arg_verbose) printf("===== Done at %s ======\n\n", datetimestamp);
//    fgetc(stdin);
    return EXIT_SUCCESS;
}
