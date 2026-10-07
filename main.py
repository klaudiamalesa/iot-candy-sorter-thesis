import serial
import time
import matplotlib.pyplot as plt
from datetime import datetime

# KONFIGURACJA
PORT_ARDUINO = 'COM3'
BAUDRATE = 9600
NAZWA_PLIKU = 'raporty_sesji.txt'

# Lista kolorów
mozliwe_kolory = ['CZERWONY', 'ZIELONY', 'NIEBIESKI', 'ZOLTY', 'POMARANCZOWY', 'FIOLETOWY', 'NIEZNANY']
kolory_wykresu = ['red', 'green', 'blue', 'yellow', 'orange', 'purple', 'gray']

licznik_kolorow = {k: 0 for k in mozliwe_kolory}

# Start sesji
czas_startu = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Wykres
plt.ion()
fig, ax = plt.subplots(figsize=(10, 6))

try:
    print(f"Łączenie z Arduino na porcie {PORT_ARDUINO}...")
    ser = serial.Serial(PORT_ARDUINO, BAUDRATE, timeout=1)
    time.sleep(2)
    print(f"--- ROZPOCZĘTO SESJĘ: {czas_startu} ---")
    print("Wrzuć cukierki. Statystyki zapiszą się po zakończeniu programu.")

    while True:
        if ser.in_waiting > 0:
            linia = ser.readline().decode('utf-8').strip()

            if linia in licznik_kolorow:
                
                licznik_kolorow[linia] += 1
                print(f"Wykryto: {linia}")

                # Aktualizacja Wykresu
                ax.clear()
                wartosci = [licznik_kolorow[k] for k in mozliwe_kolory]
                bars = ax.bar(mozliwe_kolory, wartosci, color=kolory_wykresu)
                ax.set_title(f"Sesja od: {czas_startu}")
                ax.set_ylabel("Liczba sztuk")

                # Liczby nad słupkami
                for bar in bars:
                    height = bar.get_height()
                    if height > 0:
                        ax.text(bar.get_x() + bar.get_width() / 2., height,
                                f'{int(height)}', ha='center', va='bottom')

                plt.pause(0.1)

except KeyboardInterrupt:

    print("\n\nKończenie sesji... Zapisywanie raportu...")

    czas_konca = datetime.now().strftime("%H:%M:%S")
    suma_cukierkow = sum(licznik_kolorow.values())


    with open(NAZWA_PLIKU, "a", encoding="utf-8") as f:
        f.write("\n" + "=" * 40 + "\n")
        f.write(f" RAPORT SESJI SORTOWANIA\n")
        f.write(f" Start: {czas_startu}  |  Koniec: {czas_konca}\n")
        f.write("=" * 40 + "\n")


        for kolor, ilosc in licznik_kolorow.items():
            if ilosc > 0:
                f.write(f" - {kolor.ljust(15)} : {ilosc} szt.\n")

        f.write("-" * 40 + "\n")
        f.write(f" ŁĄCZNIE POSORTOWANO: {suma_cukierkow} szt.\n")
        f.write("=" * 40 + "\n\n")

    print(f"Raport dopisano do pliku: {NAZWA_PLIKU}")

    ser.close()
    plt.ioff()
    plt.show()

except Exception as e:
    print(f"Wystąpił błąd: {e}")