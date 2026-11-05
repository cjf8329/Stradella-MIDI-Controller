import serial
import csv
from pathlib import path # to see if data.csv exists and is empty

csv_path = path('data.csv')
write_headers_flag = not csv_path.exists() or csv_path.stat().st_size == 0

if write_headers_flag:
    headers = ['distance_mm', 'adc_raw', 'voltage']
    with open('data.csv', 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(headers)


def log_reading(distance_mm, adc_raw, voltage):
    with open('data.csv', 'a', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([distance_mm, adc_raw, voltage])

com4 = serial.Serial('COM4', 115200)

