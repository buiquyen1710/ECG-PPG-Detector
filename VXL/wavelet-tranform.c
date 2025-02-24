#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <psapi.h>
#define BLOCK_SIZE 800  // Số lượng mẫu đọc trong mỗi khối
#define FILTER_SIZE 8  // Đ? dài c?a b? l?c db4
#define SR 1000

static const double db4_low_pass_filter[FILTER_SIZE] = {-0.010597401784997278, 0.032883011666982945, 0.030841381835986965, -0.18703481171888114, -0.02798376941698385, 0.6308807679295904, 0.7148465705525415, 0.23037781330885523};
static const double db4_high_pass_filter[FILTER_SIZE] = {-0.23037781330885523, 0.7148465705525415, -0.6308807679295904, -0.02798376941698385, 0.18703481171888114, 0.030841381835986965, -0.032883011666982945, -0.010597401784997278};

static const double rdb4_low_pass_filter[FILTER_SIZE] = {0.23037781330885, 0.7148465705525415, 0.6308807679295904, -0.02798376941698385, -0.18703481171888114, 0.030841381835986965, 0.032883011666982945, -0.010597401784997278};
static const double rdb4_high_pass_filter[FILTER_SIZE] = {-0.010597401784997278, -0.032883011666982945, 0.030841381835986965, 0.18703481171888114, -0.02798376941698385, -0.6308807679295904, 0.7148465705525415, -0.23037781330885523};

void convolve(const double *signal, int signal_len, const double *filter, int filter_len, double *result) {

    int output_len = signal_len + filter_len - 1;
    for (int i = 0; i < output_len; i++) {
        result[i] = 0.0;  // Kh?i t?o giá tr? 0
        for (int j = 0; j < filter_len; j++) {
            if (i - j >= 0 && i - j < signal_len) {  // Ki?m tra gi?i h?n
                result[i] += signal[i - j] * filter[j];
            }
        }
    }
}

void filter_and_downsample(const double *signal, int signal_len, const double *filter, int filter_len, double *result, int *result_len) {
    int filtered_len = signal_len + filter_len - 1;
    double *filtered_signal = (double *)malloc(filtered_len * sizeof(double));

    // Th?c hi?n tích ch?p
    convolve(signal, signal_len, filter, filter_len, filtered_signal);

    // Gi?m kích thư?c (downsample)
    int index = 0;
    for (int i = 0; i < filtered_len; i += 2) {  // Ch? l?y giá tr? t?i các v? trí ch?n
        result[index++] = filtered_signal[i];
    }

    *result_len = index;  // C?p nh?t đ? dài c?a m?ng sau gi?m kích thư?c
    free(filtered_signal);
}

void wavelet_transform(const double *signal, int signal_len, double *cA, int *cA_len, double *cD, int *cD_len) {
    // L?c tín hi?u b?ng b? l?c Low-pass và gi?m kích thư?c
    filter_and_downsample(signal, signal_len, db4_low_pass_filter, FILTER_SIZE, cA, cA_len);

    // L?c tín hi?u b?ng b? l?c High-pass và gi?m kích thư?c
    filter_and_downsample(signal, signal_len, db4_high_pass_filter, FILTER_SIZE, cD, cD_len);
}

// Hàm đọc một khối dữ liệu từ file CSV
int read_signal_block_from_csv(FILE *file, double *signal, int block_size, int col) {
    char buffer[1024];
    int count = 0;

    while (fgets(buffer, sizeof(buffer), file) && count < block_size) {
        char *token = strtok(buffer, ",");
        for (int i = 0; i < col; i++) {
            token = strtok(NULL, ",");
        }
        if (token) {
            signal[count++] = atof(token);
        }
    }

    return count;
}

// Hàm ghi kết quả vào file CSV
void write_signal_to_csv(const char *filename, const double *signal, int length, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (!file) {
        perror("Không thể mở file để ghi");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < length; i++) {
        fprintf(file, "%.6lf\n", signal[i]);
    }

    fclose(file);
}
void upsample(const double *signal, int signal_len, double *upsampled_signal, int upsample_factor) {
    int index = 0;
    for (int i = 0; i < signal_len; i++) {
        upsampled_signal[index++] = signal[i];
        for (int j = 1; j < upsample_factor; j++) {
            upsampled_signal[index++] = 0.0;  // Chèn giá trị 0
        }
    }
}

void reconstruct_signal(const double *cA, int cA_len, const double *cD, int cD_len, 
                        const double *rdb4_low_pass, const double *rdb4_high_pass, int filter_len, 
                        double *reconstructed_signal, int signal_len) {
    // Upsample hệ số cA và cD
    int upsampled_len = cA_len * 2;
    double *upsampled_cA = (double *)malloc(upsampled_len * sizeof(double));
    double *upsampled_cD = (double *)malloc(upsampled_len * sizeof(double));
    upsample(cA, cA_len, upsampled_cA, 2);
    upsample(cD, cD_len, upsampled_cD, 2);

    // Tích chập với bộ lọc hồi phục
    double *reconstructed_cA = (double *)malloc((upsampled_len + filter_len - 1) * sizeof(double));
    double *reconstructed_cD = (double *)malloc((upsampled_len + filter_len - 1) * sizeof(double));
    convolve(upsampled_cA, upsampled_len, rdb4_low_pass, filter_len, reconstructed_cA);
    convolve(upsampled_cD, upsampled_len, rdb4_high_pass, filter_len, reconstructed_cD);

    // Cộng hai tín hiệu để tái tạo
    for (int i = 0; i < signal_len; i++) {
        reconstructed_signal[i] = 0.0;
        if (i < upsampled_len + filter_len - 1) {
            reconstructed_signal[i] += reconstructed_cA[i];
            reconstructed_signal[i] += reconstructed_cD[i];
        }
    }

    // Giải phóng bộ nhớ tạm
    free(upsampled_cA);
    free(upsampled_cD);
    free(reconstructed_cA);
    free(reconstructed_cD);
}

// Hàm tìm các đỉnh tín hiệu
int find_peaks(const double *signal, int signal_len, int *peaks, int max_peaks, double threshold) {
    int peak_count = 0;
    int i=1;
    while (i<signal_len-1) {
        if (signal[i] > threshold && signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
            if (peak_count < max_peaks) {
                peaks[peak_count++] = i;
                i+=120;
            } else {
                break;
            }
        }
        else i++;
    }
    
    return peak_count;
}

// Hàm tính nhịp tim từ các đỉnh
double calculate_heart_rate(const int *peaks, int peak_count, double sample_rate) {
    if (peak_count < 2) return 0.0;

    double rr_intervals_sum = 0.0;
    for (int i = 1; i < peak_count; i++) {
      //  printf("%f\n", (peaks[i] - peaks[i - 1]) / sample_rate);
        rr_intervals_sum += (peaks[i] - peaks[i - 1]) / sample_rate;
    }
    double rr_mean = rr_intervals_sum / (peak_count - 1);
    return 60.0 / rr_mean; // Nhịp tim tính bằng bpm
}

// Hàm tính SpO2 (giả định tín hiệu Red và IR đã được tách)
double calculate_spo2(const double *red_signal, const double *ir_signal, int signal_len) {
    PROCESS_MEMORY_COUNTERS mem_counters;
    DWORD process_id = GetCurrentProcessId();
    HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    double sr1,sr2,sr3,sr4;
    if (GetProcessMemoryInfo(process_handle, &mem_counters, sizeof(mem_counters))) {
        sr1 = mem_counters.WorkingSetSize / 1024;
    } else {
        perror("Không thể lấy thông tin RAM");
    }
    double ac_red = 0.0, ac_ir = 0.0, dc_red = 0.0, dc_ir = 0.0;
    for (int i = 0; i < signal_len; i++) {
        dc_red += red_signal[i];
        dc_ir += ir_signal[i];
        ac_red += fabs(red_signal[i] - dc_red / signal_len);
        ac_ir += fabs(ir_signal[i] - dc_ir / signal_len);
    }
    double ratio = (ac_red / dc_red) / (ac_ir / dc_ir);
    double spo2 = 110 - 25 * ratio; // Công thức tham khảo, cần điều chỉnh tùy tín hiệu
    if (GetProcessMemoryInfo(process_handle, &mem_counters, sizeof(mem_counters))) {
        sr2 = mem_counters.WorkingSetSize / 1024;
    } else {
        perror("Không thể lấy thông tin RAM");
    }
    //printf("Bo nho su dung: %f KB\n", sr2-sr1);
    return spo2;
}


int main() {
    FILE *input_file = fopen("PPG.csv", "r");
    if (!input_file) {
        perror("Không thể mở file CSV đầu vào");
        return EXIT_FAILURE;
    }

    FILE *output_file = fopen("ucAr_output.csv", "w");
    if (!output_file) {
        perror("Không thể mở file CSV đầu ra");
        fclose(input_file);
        return EXIT_FAILURE;   
    }
    double signal[BLOCK_SIZE];       
    double cA[BLOCK_SIZE], cD[BLOCK_SIZE]; 
    double cA1[BLOCK_SIZE], cD1[BLOCK_SIZE];
    double cA2[BLOCK_SIZE], cD2[BLOCK_SIZE];
    double ucA[BLOCK_SIZE], ucA1[BLOCK_SIZE],ucA2[BLOCK_SIZE];
    double ucA3[BLOCK_SIZE], ucA4[BLOCK_SIZE],ucAr[BLOCK_SIZE];
    int peaks[10]={0};           
    int peak_result[100];
    int kk=0;
    int cA_len = 0, cD_len = 0, peak_count = 0;
    int cA1_len = 0, cD1_len = 0;
    int cA2_len = 0, cD2_len = 0;
    int block_number = 0;
    int t=0;
    int check = 0;
    double spo2=0;
    // Đọc và xử lý từng khối dữ liệu
    // Biến đo thời gian
    clock_t start_time, end_time;
    start_time = clock();

    // Biến đo RAM
    while (1) {
        int block_size = read_signal_block_from_csv(input_file, signal, BLOCK_SIZE,t);
        
        if (block_size == 0) break; // Không còn dữ liệu để đọc

        spo2 += calculate_spo2(signal,signal,block_size);
        block_number++;
        // Xử lý Wavelet Transform cho khối dữ liệu
        wavelet_transform(signal, block_size, cA, &cA_len, cD, &cD_len);
        wavelet_transform(cA, cA_len, cA1, &cA1_len, cD1, &cD1_len);
        wavelet_transform(cA1, cA1_len, cA2, &cA2_len, cD2, &cD2_len);
        upsample(cA2, cA2_len, ucA, 2);
        int k=cA2_len*2;
        convolve(ucA,k, rdb4_low_pass_filter, FILTER_SIZE, ucA1);
        k=k*2;
        upsample(ucA1, k, ucA2, 2);
        convolve(ucA2,k, rdb4_low_pass_filter, FILTER_SIZE, ucA3);
        k=k*2;
        upsample(ucA3, k, ucA4, 2);
        convolve(ucA4,k, rdb4_low_pass_filter, FILTER_SIZE, ucAr);
        for (int i = 0; i < block_size; i++) {
            fprintf(output_file, "%.6lf\n", ucAr[i]);
        }
        peak_count = find_peaks(ucAr, block_size, peaks, BLOCK_SIZE, 100000); // Ngưỡng tùy chỉnh
        //printf("\nCac dinh tim duoc:\n");
        for (int i = 0; i < peak_count; i++) {
            if (kk==0) 
            {
                peak_result[kk]=peaks[i]+block_number*BLOCK_SIZE;
                kk++;
                
                   // printf("Dinh %d: Vi tri %d\n", i + 1, peaks[i]+block_number*BLOCK_SIZE);
            }
            else 
            {
                if (peaks[i]+block_number*BLOCK_SIZE - peak_result[kk-1]>500) 
                {
                    peak_result[kk]=peaks[i]+block_number*BLOCK_SIZE;
                    kk++;
                   // printf("Dinh %d: Vi tri %d\n", i + 1, peaks[i]+block_number*BLOCK_SIZE);
                }
            }
        }
    }
    for (int i=0; i<kk; i++) printf("%d,",peak_result[i]);
    printf("SpO2: %.4f\n", spo2/block_number);
    printf("Nhip tim: %.4fBPM\n", calculate_heart_rate(peak_result,kk,SR));
    fclose(input_file);
    end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Thoi gian thuc hien: %f giay\n", elapsed_time);

    // Lấy thông tin RAM sử dụng
    return 0;
}
