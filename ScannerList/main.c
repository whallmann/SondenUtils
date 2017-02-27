/*
 * ScannerList für rtl_power (csv) nach txt aufbereiten
 *
 * Copyright (C) Wolfgang Hallmann <df7pn@darc.de>
 *
 * License-Identifier:	GPL-2.0+
*/

/*
 *  Übergabewerte:
 * -v                   Verbose
 * -f <rtlcsvdatei>     Dateiname den rtl_power erstellt hat
 * -o <sdrcfg-datei>    Ausgabe einer dxlAPRS konformen Freuquenzliste für den Empfang [f <frqnz> <afc> 0 0 <bandwidth>]
 * -d <sdrcfg-datei>    Default: <noFile> Liste mit bekannten Frequenzen, die NICHT in der Ausgabe (-o) erscheinen sollen
 * -a <afc-Wert>        Default: 5 / Abweichung nach oben und unten von der Frequenz / i.d.Regel 5 / wird in -o mit ausgegeben
 * -b <BandbreiteHz>    Default: intern berechnet / Wenn angegeben, wird interner Wert überschrieben für ALLE Einträge auf diesen Fix-Wert
 * -L <above signallevel>Default: -20 / Wert in db, ab dem ein Signal als solches erkannt werden soll (z.B: Rauschen -35, Signal wenn kleiner -20)
 *                      10 db Differenz scheint guter Wert zu sein, da ab diesem Wert erst sauber dekodiert werden kann.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PUFFERLEN 32767
#define SCHWELLWERT -20
#define BANDWITHDEFAULT 8000


int main(int argc, char *argv[])
{
//----------- Übergabeparameter auslesen und merken ----------------
    // Default-Werte besetzen
    int arg_verbose = 0;
    char arg_deleteFrequencyFile [255];
    arg_deleteFrequencyFile[0] = 0;
    int arg_afc = 5;
    long arg_bandwithHz = 0;
    int arg_signallevel = -20;
    char arg_outputfilename [255];
    arg_outputfilename[0] = 0;
    char arg_inputfilename [255];
    arg_inputfilename[0] = 0;
    int argindex = 0;
    for(argindex=1; argindex < argc; argindex++)
    {
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
           strcpy(&arg_deleteFrequencyFile[0], argv[argindex+1]);
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
    } // for argv
    //--- Testweise Ausgbe der Werte und Abort
    if (arg_verbose)
    {
      fputs("\n--------- Parameter -------------\n",stdout);
      printf("-v %i \n", arg_verbose);
      printf("-d %s \n", arg_deleteFrequencyFile);
      printf("-a %i \n", arg_afc);
      printf("-b %ld \n", arg_bandwithHz);
      printf("-L %i \n", arg_signallevel);
      printf("-o %s \n", arg_outputfilename);
      printf("-f %s \n", arg_inputfilename);
      fputs("\n--------- Parameter -------------\n",stdout);
    }
    int filefound = 0;
    int needabort = 0;
    FILE *f;
    //--- Check: testing write to outputfile
    if (strlen(arg_outputfilename) > 0)
    {
      f = fopen(arg_outputfilename,"r"); // erst lesend. vlt ist sie schon da.
      if (f == NULL)
      {
        f = fopen(arg_outputfilename,"at"); // können wir anlegen und schreiben (Inhalt futsch)
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
    if (strlen(arg_deleteFrequencyFile) > 0)
    {
      f = fopen(arg_deleteFrequencyFile,"rt");
      if (f != NULL) filefound = 1;
    }
    if(!filefound)
    {
      if (arg_verbose) fputs("WARNING: The file with known frequency not found (-d)\n",stdout);
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
    //printf("Dateiinhalt anzeigen:\n");

//  char c;
    static char zeile[PUFFERLEN];
    zeile[0] = 0;
    char *zeiger;
    int i = 1;
    int myint;
    // int dBListe [3000];  // for future use maybe
    int Anzahl_dbListe = 0;  // 1...n
    double QrgListe [200];
    int Anzahl_QrgListe = 0;
    double NewQrgListe [200];
    int Anzahl_NewQrgListe = 0;
    double NewQrgBreite [200];
    int Anzahl_NewQrgBreite = 0;
    double rundung;
    double startfrequenz = 0.0f;   // ist im 3. Feld der CSV Liste
    double realIntervall = 0;  //Wird überschrieben
    double untereQRG, obereQRG;
    int indexQRGblock;

    f = fopen(arg_inputfilename,"rt");
    if (f != NULL)
    {
      // erste Zeile lesen
      fgets(&zeile[0], PUFFERLEN-2, f);

      // gibt die ganze Zeile aus
      //MoreVerbose: fputs(zeile, stdout);
      if(arg_verbose) fputs("\n--- Lese Zeile \n", stdout);
      // Zerlegen in Teile mit "," getrennt
      while (feof(f) == 0)
      {
        // fputs("\n---> Kein EOF\n", stdout);
        i = 1;
        zeiger = strtok(zeile,",");
        while(zeiger != NULL)
        {
          if (i==3)
          {
            startfrequenz = atof(zeiger);
            // printf("Startwert: %9.0f",startfrequenz);
          }
          if (i==5)
          {
            realIntervall = atof(zeiger);
            //realIntervall = realIntervall / 100 * QRGKORREKTUR;
            //printf("\nRealIntervall: %9.2f",realIntervall);
            //fputs("\n----------------\n", stdout);
          }
          if (i>=7)
          {
            //printf("%s ",zeiger);
            myint = atoi(zeiger);
            //printf("(%i) ",myint);
            //Müssen wir nicht aufheben, evtl für andere Zwecke später mal
            //dBListe[Anzahl_dbListe] = myint;
            if(myint >= SCHWELLWERT)
            {
              // Frequenz errechnen und aufheben
              QrgListe[Anzahl_QrgListe] = startfrequenz + realIntervall * (i-6);
              // Zeigt schon gerundet an ==> printf("%02d: %9.0f\n",Anzahl_QrgListe,QrgListe[Anzahl_QrgListe]);
              rundung = QrgListe[Anzahl_QrgListe]/1000.0f;  // wegen HZ in KHZ
              rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
              // -------- Frequenz nun gerundet auf KHz
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
        if(arg_verbose) fputs("\n--- Lese Zeile \n", stdout);
        fgets(&zeile[0], PUFFERLEN-2, f);
      }
      fclose(f);
      if (arg_verbose) printf("\n\nEintraege in QRG-Liste: %i\n\n",Anzahl_QrgListe);


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
      while(i < Anzahl_QrgListe)
      {
        untereQRG = QrgListe[indexQRGblock];
        if (arg_verbose) printf(":: untereQRG %9.0f\n",untereQRG);

        //     noch ein Index hinter dem aktuellen?     Ist die nächste QRG dicht genug dran?
        while((indexQRGblock < Anzahl_QrgListe-1) && ((QrgListe[indexQRGblock+1] - QrgListe[indexQRGblock]) < 3.0f))
        {
          indexQRGblock++;
        }
        if (arg_verbose) printf(":: Obere QRG %9.0f, indexQRGblock %i, \n",QrgListe[indexQRGblock],indexQRGblock);
        // Hier angekommen: QRG Diff zu hoch oder Listenende
        // Ab 3 hintereinander folgende Frequenzen wird diese in die NewQRGListe mit ihrem Mittelwert übernommen
        if ((indexQRGblock - i) >= 2)
        {
          if (arg_verbose) fputs("Block mit mehr als 2 dichten QRGs\n",stdout);
          obereQRG = QrgListe[indexQRGblock];
          rundung = (obereQRG - untereQRG);
          // Bandbreite aufheben - gleicher Index wie NewQRG
          NewQrgBreite[Anzahl_NewQrgBreite] = rundung;
          Anzahl_NewQrgBreite++;
          rundung = rundung / 2;
          rundung = (rundung > (floor(rundung)+0.5f)) ? ceil(rundung) : floor(rundung);
          NewQrgListe[Anzahl_NewQrgListe] = untereQRG + rundung;
          Anzahl_NewQrgListe++;
          if (arg_verbose) printf("::NeuQRG %9.0f\n",NewQrgListe[Anzahl_NewQrgListe]);
        }
        indexQRGblock++;
        i = indexQRGblock;
        if (arg_verbose) printf(":: weiter bei Index %i\n",i);
      }
      if (arg_verbose) printf("\n%i Frequenzen extrahiert die breit genug sind\n\n",Anzahl_NewQrgListe);
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
    //-------- Zieldatei schreiben ---------------
    char ausgabezeile[255];
    ausgabezeile[0] = 0;
    double aktBandwidth = 0;

    f = fopen(arg_outputfilename,"wt");
    if (f != NULL)
    {
      for(i=0;i<Anzahl_NewQrgListe;i++)
      {
        fputs("f ",f);
        // was angegeben als Parameter, dann nimm den für alle
        if (arg_bandwithHz)
        {
          aktBandwidth = arg_bandwithHz;
        }
        else
        {
          // autom. festgestellte Bandbreite weitergeben
          aktBandwidth = NewQrgBreite[i] * 1000;
          if(aktBandwidth <= BANDWITHDEFAULT)
          {
            aktBandwidth = BANDWITHDEFAULT;
          }
          else
          {
            if(aktBandwidth >= 11000.0f)  aktBandwidth = aktBandwidth - 3000.0f;
          }
        }
        sprintf(ausgabezeile, "%6.3f %i 0 0 %-6.0f\n", NewQrgListe[i] / 1000.0f, arg_afc, aktBandwidth );
        fputs(ausgabezeile, f);
      }
      fclose(f);
    }
    else
    {
      if (arg_verbose) fputs("ERROR: output file not written (-o)",stdout);
    }

    if (arg_verbose) fputs("===== Done ======\n\n",stdout);
    //fgetc(stdin);
    return EXIT_SUCCESS;
}
