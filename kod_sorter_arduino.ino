#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_TCS34725.h"


LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

Servo servoPodajnik;   // Serwo tarczy (90 - 35 - 0)
Servo servoSortujace;  // Serwo kierujące do kubłów

const int POZ_POBOR = 90;  
const int POZ_POMIAR = 35; 
const int POZ_WYRZUT = 0;  

int nCzer = 0, nPom = 0, nZol = 0, nZie = 0, nNie = 0, nFio = 0, nInny = 0;
int obecnaPozycja = POZ_POBOR;
int licznikPustych = 0; 

void plynnyRuch(int cel, int czas) {
  if (obecnaPozycja < cel) {
    for (int i = obecnaPozycja; i <= cel; i++) {
      servoPodajnik.write(i);
      delay(czas); 
    }
  } else {
    for (int i = obecnaPozycja; i >= cel; i--) {
      servoPodajnik.write(i);
      delay(czas); 
    }
  }
  obecnaPozycja = cel;
}

void wyswietlStatystyki() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("R"); lcd.print(nCzer); lcd.print(" O"); lcd.print(nPom); lcd.print(" Y"); lcd.print(nZol); lcd.print(" G"); lcd.print(nZie);
  lcd.setCursor(0, 1);
  lcd.print("B"); lcd.print(nNie); lcd.print(" V"); lcd.print(nFio); lcd.print(" ?"); lcd.print(nInny);
}

void setup() {
  Serial.begin(9600); 
  lcd.init();
  lcd.backlight();
  lcd.print("START SYSTEMU");

  servoSortujace.attach(2);  
  servoPodajnik.attach(3);   
  
  if (!tcs.begin()) {
    lcd.clear(); lcd.print("BLAD CZUJNIKA!");
    while (1);
  }
  
  servoPodajnik.write(POZ_POBOR); 
  obecnaPozycja = POZ_POBOR;
  servoSortujace.write(160); // Startowo na pozycję "Nieznany"       
  
  delay(1000);
  lcd.clear(); lcd.print("GOTOWY!");
}

void loop() {
  uint16_t r, g, b, c;

  // 1. POBIERANIE CUKIERKA
  plynnyRuch(POZ_POBOR, 40);
  delay(800); 

  // 2. TRANSPORT DO POMIARU
  plynnyRuch(POZ_POMIAR, 15);
  delay(500); 

  // Uśrednianie pomiaru
  uint32_t sumR = 0, sumG = 0, sumB = 0, sumC = 0;
  for(int i=0; i<5; i++) {
    tcs.getRawData(&r, &g, &b, &c);
    sumR += r; sumG += g; sumB += b; sumC += c;
    delay(60);
  }
  r = sumR/5; g = sumG/5; b = sumB/5; c = sumC/5;

  
  int katKubelka = 160; // Domyślnie NIEZNANY
  String nazwaKoloru = "NIEZNANY";

  // TEST OBECNOŚCI
  if (c < 40) {
    licznikPustych++;
    lcd.clear(); lcd.print("PUSTO");
    if (licznikPustych >= 10) { 
        lcd.setCursor(0,1); lcd.print("KONIEC PRACY");
        while(1);
    }
    plynnyRuch(POZ_POBOR, 20);
    return;
  }
  licznikPustych = 0;

  uint32_t suma = r + g + b;
  float pR = (float)r / suma * 100.0;
  float pG = (float)g / suma * 100.0;
  float pB = (float)b / suma * 100.0;

  // B. IDENTYFIKACJA I USTAWIANIE KĄTÓW 
  
  // 1. NIEBIESKI
  if (pB > 45.0) {
    nazwaKoloru = "NIEBIESKI"; katKubelka = 110; nNie++;
  }
  // 2. ZIELONY
  else if (pG > 48.0 && pR < 30.0) {
    nazwaKoloru = "ZIELONY"; katKubelka = 90; nZie++;
  }
  // 3. POMARAŃCZOWY
  else if (pR > 51.0 && pB < 22.0) {
    nazwaKoloru = "POMARANCZOWY"; katKubelka = 50; nPom++;
  }
  // 4. CZERWONY
  else if (pR > 43.0 && pB > 24.0) {
    nazwaKoloru = "CZERWONY"; katKubelka = 20; nCzer++;
  }
  // 5. ŻÓŁTY
  else if (pR > 37.0 && pG > 39.0) {
    nazwaKoloru = "ZOLTY"; katKubelka = 70; nZol++;
  }
  // 6. FIOLETOWY
  else if (pB > pR && pB > pG && c < 80) {
    nazwaKoloru = "FIOLETOWY"; katKubelka = 135; nFio++;
  }
  else {
    nazwaKoloru = "NIEZNANY"; katKubelka = 160; nInny++;
  }

  // 3. WYSYŁKA I RUCH SERWA SORTUJĄCEGO
  Serial.println(nazwaKoloru);
  lcd.clear(); lcd.print("KOLOR:");
  lcd.setCursor(0, 1); lcd.print(nazwaKoloru);
  
  servoSortujace.write(katKubelka);
  delay(800); // Czas na ustawienie rynny

  // 4. WYRZUT
  plynnyRuch(POZ_WYRZUT, 15); 
  delay(500); 

  // 5. POWRÓT I AKTUALIZACJA LCD
  wyswietlStatystyki();
  delay(1000); 
}