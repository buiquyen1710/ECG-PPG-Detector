import serial
import matplotlib.pyplot as plt
import mplcursors
import numpy as np
from scipy.signal import butter, filtfilt, find_peaks

# Thiết lập kết nối serial
try:
    ser = serial.Serial('COM4', 115200)
except Exception as e:
    raise RuntimeError(f"Lỗi kết nối Serial: {e}")

ir_data = []
red_data = []

try:
    while True:
        # Đọc một dòng từ serial
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(line)
            # Phân tích dữ liệu
            parts = line.split(',')
            if len(parts) == 2:
                try:
                    ir_value = int(parts[1].strip())
                    red_value = int(parts[0].strip())
                    ir_data.append(ir_value)
                    red_data.append(red_value)
                except ValueError as e:
                    print(f"Lỗi khi phân tích dữ liệu: {line}, error: {e}")
            else:
                print(f"Không đúng định dạng dòng: {line}")
        
        # Dừng sau khi đọc đủ dữ liệu
        if len(ir_data) >= 4000:
            break
finally:
    ser.close()

# Lật mảng (do tín hiệu PPG thường ngược)
ir_data_flip = np.flip(ir_data)
red_data_flip = np.flip(red_data)

# Chia và lấy phần giữa của dữ liệu
def split_and_extract_middle(data):
    n = len(data)
    quarter = n // 4
    start_index = quarter
    end_index = n - quarter if n % 4 == 0 else n - quarter + 1
    return data[start_index:end_index]

red_data_mid = split_and_extract_middle(red_data_flip)
ir_data_mid = split_and_extract_middle(ir_data_flip)

# Bộ lọc Butterworth
fs = 25
   # Tần số lấy mẫu (Hz)
order = 4
cutoff_frequency = 12  # Tần số cắt (Hz)
nyquist_frequency = 0.5 * fs
normal_cutoff = cutoff_frequency / nyquist_frequency
b, a = butter(order, normal_cutoff, btype='low', analog=False)

ppg_red_data_filtered = filtfilt(b, a, red_data_mid)
ppg_ir_data_filtered = filtfilt(b, a, ir_data_mid)

# Hàm tính SpO2
def calculate_spo2(red_data, ir_data):
    try:
        red_dc = np.mean(red_data)
        ir_dc = np.mean(ir_data)

        red_ac = red_data - red_dc
        ir_ac = ir_data - ir_dc

        peaks_red, _ = find_peaks(red_ac)
        peaks_ir, _ = find_peaks(ir_ac)

        if len(peaks_red) == 0 or len(peaks_ir) == 0:
            return None

        red_ac_std = np.std(red_ac[peaks_red])
        ir_ac_std = np.std(ir_ac[peaks_ir])

        R = (red_ac_std / red_dc) / (ir_ac_std / ir_dc)
        spo2 = 110 - 25 * R
        return spo2
    except Exception as e:
        print(f"Lỗi trong calculate_spo2: {e}")
        return None

# Hàm tính nhịp tim (HR)
def hr_cal(data):
    try:
        peaks, _ = find_peaks(data, height=0.5, distance=fs * 0.5)
        if len(peaks) < 2:
            return None

        rr_intervals = np.diff(peaks) / fs
        if len(rr_intervals) == 0:
            return None

        average_rr = np.mean(rr_intervals)
        heart_rate = 60 / average_rr
        return heart_rate
    except Exception as e:
        print(f"Lỗi trong hr_cal: {e}")
        return None

# Tính toán SpO2 và HR
spo2 = calculate_spo2(ppg_red_data_filtered, ppg_ir_data_filtered)
hr = hr_cal(ppg_red_data_filtered)

# Tạo biểu đồ
fig, ax1 = plt.subplots(1, 1, figsize=(12, 12))

# Vẽ đồ thị PPG
ax1.plot(red_data_mid, label='PPG_RED Data')
ax1.plot(ppg_red_data_filtered, label='Filtered PPG_RED Data', alpha=0.8, lw=3)
ax1.plot(ppg_ir_data_filtered, label='Filtered PPG_IR Data', alpha=0.8, lw=3)
ax1.legend()
ax1.set_ylabel('Giá Trị PPG')

# Thiết lập tiêu đề với kiểm tra giá trị
if spo2 is None or hr is None:
    ax1.set_title("PPG Data: Không thể tính toán SpO2 hoặc HR")
else:
    ax1.set_title(f'PPG Data, SpO2: {spo2:.2f}%, HR: {hr:.2f} bpm')

# Hiển thị giá trị khi hover trên đồ thị
cursor1 = mplcursors.cursor(ax1, hover=True)
cursor1.connect("add", lambda sel: sel.annotation.set_text(f'PPG: {sel.target[1]:.2f}'))

plt.tight_layout()
plt.show()
