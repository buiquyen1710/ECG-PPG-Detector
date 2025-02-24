import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Flatten, Conv1D, MaxPooling1D, Dropout
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix

# Đọc và xử lý dữ liệu từ file CSV
data = pd.read_csv("ecg.csv")  # File CSV chứa dữ liệu
data = data.apply(pd.to_numeric, errors='coerce')  # Chuyển giá trị không hợp lệ thành NaN
data = data.dropna()  # Loại bỏ các hàng có NaN

# Tách dữ liệu và nhãn
X = data.iloc[:, :-1].values.astype('float32')  # Các cột đầu là dữ liệu đầu vào
y = data.iloc[:, -1].values.astype('float32')   # Cột cuối cùng là nhãn

# Chia dữ liệu thành tập train và test
X_train, X_test, Y_train, Y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Reshape dữ liệu để phù hợp với Conv1D (thêm một chiều kênh)
X_train = X_train[:, :, np.newaxis]
X_test = X_test[:, :, np.newaxis]

# Xây dựng mô hình mạng nơ-ron
model = Sequential([
    Conv1D(8, kernel_size=3, activation='relu', input_shape=(X_train.shape[1], X_train.shape[2])),
    MaxPooling1D(pool_size=2),
    Dropout(0.3),
    Flatten(),
    Dense(64, activation='relu'),
    Dense(1, activation='sigmoid')  # Output layer cho bài toán nhị phân
])

# Biên dịch mô hình
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])

# Huấn luyện mô hình
history = model.fit(X_train, Y_train, epochs=20, batch_size=32, validation_split=0.2)

# Đánh giá mô hình trên tập test
test_loss, test_accuracy = model.evaluate(X_test, Y_test)
print(f"Test Accuracy: {test_accuracy * 100:.2f}%")

# Dự đoán trên tập test
y_pred = model.predict(X_test)
y_pred_classes = (y_pred > 0.5).astype(int)  # Chuyển đổi xác suất thành nhãn 0 hoặc 1

# Tính toán ma trận nhầm lẫn
conf_matrix = confusion_matrix(Y_test, y_pred_classes)

# Lấy các giá trị từ ma trận nhầm lẫn
true_negative = conf_matrix[0, 0]  # Dự đoán 0, Thực tế 0
false_positive = conf_matrix[0, 1]  # Dự đoán 1, Thực tế 0
false_negative = conf_matrix[1, 0]  # Dự đoán 0, Thực tế 1
true_positive = conf_matrix[1, 1]  # Dự đoán 1, Thực tế 1

# In kết quả
print(f"Số lần dự đoán là 0 và thực tế là 0 (True Negative): {true_negative}")
print(f"Số lần dự đoán là 0 và thực tế là 1 (False Negative): {false_negative}")
print(f"Số lần dự đoán là 1 và thực tế là 0 (False Positive): {false_positive}")
print(f"Số lần dự đoán là 1 và thực tế là 1 (True Positive): {true_positive}")
