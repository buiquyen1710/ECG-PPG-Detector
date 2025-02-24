import numpy as np
import pandas as pd
import pywt
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
import math as npmath
# Tải dữ liệu từ file CSV
inputdata = pd.read_csv('datac.csv')
fs = 1000

# Xác định độ dài của data
data_length = len(inputdata)
print(data_length)
# Tính phần dư của data khi chia cho 256
remainder = data_length % 64
# Nếu phần dư khác 0, bỏ phần dư ở cuối
dataa = inputdata.iloc[:data_length - remainder]
# Lưu dữ liệu đã xử lý vào file CSV mới
output_file_path = 'data.csv'
dataa.to_csv(output_file_path, index=False)

# Đọc data vừa chỉnh sửa
data = pd.read_csv('data.csv')

# Lấy data từ các cột trong file csv
pcg_signal = data.iloc[:, 7].values
ecg_signal = data.iloc[:, 8].values

# Bước 2: Định nghĩa wavelet
lendata = len(data)
wavelet = 'db4'



# Thực hiện phép phân tích wavelet cho tín hiệu ECG
cA_ecg = ecg_signal
coeffs_ecg = []
for i in range(9):
    cA_ecg, cD_ecg = pywt.dwt(cA_ecg, wavelet, mode='symmetric', axis=-1)
    coeffs_ecg.append((cA_ecg, cD_ecg))

# tín hiệu ECG theo lv
level_ecg = 3

cA8_ecg, cD8_ecg = coeffs_ecg[level_ecg - 1]
A8_ecg = pywt.upcoef('a', cA8_ecg, wavelet, level= level_ecg, take=lendata)


# Thực hiện phép phân tích wavelet cho tín hiệu PCG
cA_pcg = pcg_signal
coeffs_pcg = []
for i in range(9):
    cA_pcg, cD_pcg = pywt.dwt(cA_pcg, wavelet, mode='symmetric', axis=-1)
    coeffs_pcg.append((cA_pcg, cD_pcg))

# Mở gói các hệ số để vẽ tín hiệu PCG
level_pcg = 2

cA8_pcg, cD8_pcg = coeffs_pcg[level_pcg - 1]
A8_pcg = pywt.upcoef('d', cA8_pcg, wavelet, level= level_pcg, take = lendata)


# Thuật toán tìm đỉnh
ampl_pcg_data_filtered, __ = find_peaks(A8_pcg, distance=int(1 * 1411800))  # Chỉnh PPG tại đây
indices_pcg_data_filtered = [i for i in range(len(A8_pcg))]

ampl_ecg_data_filtered, __ = find_peaks(A8_ecg, distance=int(1 * 1451100))  # Chỉnh PPG tại đây
indices_ecg_data_filtered = [i for i in range(len(A8_ecg))]

plt.figure(figsize=(14, 10))

plt.subplot(2, 1, 1)
plt.plot(indices_pcg_data_filtered, A8_pcg, color='blue', label='ECG Signal')  # Đường tín hiệu màu xanh dương
plt.plot(ampl_pcg_data_filtered, A8_pcg[ampl_pcg_data_filtered], "o", color='orange', label='Detected Peaks')  # Đỉnh màu cam
plt.legend()
plt.title("ECG Signal")

plt.subplot(2, 1, 2)
plt.plot(indices_ecg_data_filtered, A8_ecg, color='red', label='PPG Signal')  # Đường tín hiệu màu xanh lá cây
plt.plot(ampl_ecg_data_filtered, A8_ecg[ampl_ecg_data_filtered], "o", color='purple', label='Detected Peaks')  # Đỉnh màu tím
plt.legend()
plt.title("PPG Signal")

plt.tight_layout()
plt.show()

# Tạo DataFrame chỉ chứa các chỉ số và tín hiệu từ hai kênh
output_data = pd.DataFrame({
    'Indices_PCG': indices_pcg_data_filtered,
    'D8_PCG': D8_pcg,  # Tín hiệu PCG đã phân tích
    'Indices_ECG': indices_ecg_data_filtered,
    'A8_ECG': A8_ecg,  # Tín hiệu ECG đã phân tích
})

# Lưu dữ liệu hai kênh vào file CSV
output_file_path = 'ecg_pcg_signals.csv'
output_data.to_csv(output_file_path, index=False, encoding='utf-8')

print(f"Tín hiệu ECG và PCG (không bao gồm đỉnh) đã được lưu vào {output_file_path}")
