import numpy as np
import pandas as pd
import pywt
import matplotlib.pyplot as plt

# Đọc file CSV (giả sử mỗi hàng là một tín hiệu ECG)
file_path = "ecg.csv"  # Đường dẫn tới file CSV của bạn
data = pd.read_csv(file_path, header=None)  # header=None để đảm bảo không bỏ qua dòng đầu

# Chọn hàng cụ thể để xử lý (giả sử hàng thứ 3)
row_index = 4998  # Thay đổi chỉ số này để chọn hàng khác
signal = data.iloc[row_index].values

# Thực hiện phân tích wavelet
wavelet = 'db4'  # Chọn loại wavelet (ở đây là Daubechies 4)
cA_signal = signal
coeffs_signal = []

# Phân tích wavelet qua nhiều cấp độ
for i in range(9):
    cA_signal, cD_signal = pywt.dwt(cA_signal, wavelet, mode='symmetric', axis=-1)
    coeffs_signal.append((cA_signal, cD_signal))

# Chọn một cấp độ cụ thể (ví dụ: cấp độ 3) và tái tạo tín hiệu
level = 3
cA_level, _ = coeffs_signal[level - 1]
reconstructed_signal = pywt.upcoef('a', cA_level, wavelet, level=level, take=len(signal))

# Lưu tín hiệu đã tái tạo vào DataFrame
processed_df = pd.DataFrame([reconstructed_signal])

# Lưu dữ liệu đã phân tích wavelet vào file CSV mới
output_path = 'wavelet_processed_single_ecg.csv'
processed_df.to_csv(output_path, index=False, header=False)
print(f"Dữ liệu đã phân tích wavelet và lưu tại: {output_path}")

# Vẽ tín hiệu đã xử lý
plt.figure(figsize=(14, 6))
plt.plot(reconstructed_signal, color='blue', label='Processed ECG Signal (Selected Row)')
plt.legend()
plt.title("Processed ECG Signal (Selected Row)")
plt.tight_layout()
plt.show()
