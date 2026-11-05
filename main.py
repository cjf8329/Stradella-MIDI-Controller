import serial
import csv
from pathlib import Path # to see if data.csv exists and is empty

csv_path = Path('data.csv')
write_headers_flag = not csv_path.exists() or csv_path.stat().st_size == 0 # check if data.csv exists and isn't empty

if write_headers_flag:
    headers = ['distance_mm', 'adc_raw']
    with open('data.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(headers)


def log_reading(distance_mm, adc_raw):
    with open('data.csv', 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([distance_mm, adc_raw])

com4 = serial.Serial('COM4', 115200)

while True:
    dst = float(input("Enter current distance in mm: "))
    com4.write(b'm')
    adc = com4.readline().decode('utf-8').strip()
    log_reading(dst, adc)