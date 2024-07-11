#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct {
    int synapse_connection[256]; // 256 bits (32 bytes)
    int16_t current_potential; // 9 bits
    int16_t reset_potential; // 9 bits
    int16_t weight[4]; // 4 x 9 bits
    int16_t leak_value; // 9 bits
    int16_t positive_threshold; // 9 bits
    int16_t negative_threshold; // 9 bits
    int8_t reset_mode; // 1 bit
    //spike_destination; 26 bit 
    // dx + dy + axon_destination = 9 + 9 + 8
    int16_t dx; // 9 bit
    int16_t dy; // 9 bit
    int8_t axon_destination; // 8 bit
    int8_t tick_delivery; // 4 bits
} CSRAM;

unsigned int NeuronBlock(CSRAM *csram, int input[], int8_t num_of_input){
    int pre_ff, output;
    int ff=0;
    int negative_check, positive_check;
    if (num_of_input==0)
    {
        return 0;
    }
    
    for(int i = 0; i<num_of_input;i++){
        pre_ff=ff;
        int process = csram->synapse_connection[input[i]];
        //printf("%d ", process);
        if(process == 0){
            continue;
        }else{
            int weight;
            if(input[i]%2==0){
                weight = csram->weight[0];
            }else{
                weight = csram->weight[1];
            }
            ff = weight + pre_ff;
        }
    }
    int value = ff+csram->leak_value;

    if(value > csram->positive_threshold){
        return 1;
        //printf("co xung ne \n");
    }
    else{
        return 0;
    }
    return 0;
}



// Hàm để chuyển đổi chuỗi bit thành giá trị số nguyên
// unsigned int bitsToUnsignedInt(const char *bits, int length) {
//     unsigned int value = 0;
//     for (int i = 0; i < length; i++) {
//         value = (value << 1) | (bits[i] - '0');
//     }
//     return value;
// }

unsigned int bitsToUnsignedInt(const char *bits, int length) {
    signed int value = 0;
    for (int i = 0; i < length; i++) {
        if(bits[i]=='1'){
            value+=pow(2, length-i-1);
        }
    }
    return value;
}

signed int bitsToSignedInt(const char *bits, int length) {
    signed int value = 0;
    for (int i = 1; i < length; i++) {
        if(bits[i]=='1'){
            value+=pow(2, length-i-1);
        }
    }
    if(bits[0]=='1'){
        value=-pow(2, length-1)+value;
    }
    return value;
}

// Hàm để chuyển đổi chuỗi bit thành mảng byte
void bitsToByteArray(const char *bits, char *byteArray, int byteArrayLength) {
    for (int i = 0; i < byteArrayLength; i++) {
        byteArray[i] = bitsToUnsignedInt(bits + (i * 8), 8);
    }
}

void processNumInput(int *num_input){
    FILE* file = fopen("tb_num_inputs.txt", "r");
    if (file == NULL) {
        printf("Không thể mở file.\n");
    }

    char line[256];
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        num_input[i]=atoi(line);
        i++;
    }

    // Đóng file
    fclose(file);
}

void processInput(unsigned int *input[10][4], int tb_num_input[], int *num_input[10]){
    FILE* file = fopen("tb_input.txt", "r");
    // FILE* file1 = fopen("input.txt", "w");
    if (file == NULL) {
        printf("Không thể mở file.\n");
    }
    for(int i=0;i<10;i++){
        char line[33]; // 368 bits + 1 for null terminator
        for(int j = 0;j<tb_num_input[i];j++){
            if (fgets(line, sizeof(line), file) != NULL) {
                int core = bitsToUnsignedInt(line ,9);
                if (input[i][core] == NULL) {
                    input[i][core] = malloc(256 * sizeof(int)); // Cấp phát bộ nhớ cho `input[i][j]`
                    for(int m = 0; m<(sizeof(input[i][core])/sizeof(input[i][core][0]));m++){
                        input[i][core][m]=0;
                    }
                }
                if (num_input[i] == NULL) {
                    num_input[i] = malloc(256 * sizeof(int)); // Cấp phát bộ nhớ cho `input[i][j]`
                    for (int n = 0; n < 4; n++) {
                        num_input[i][n] = 0; // Khởi tạo giá trị cho num_input[i]
                    }
                }
                input[i][core][num_input[i][core]]=bitsToUnsignedInt(line + 18, 8);
                // fprintf(file1, "%d \n", input[i][core][num_input[i][core]]);
                num_input[i][core]++;
            } else {
                break;
            }
        }
    }

    // Đóng file
    fclose(file);
    // fclose(file1);
    //return num_input;
}

void processLine(const char *bitStream, CSRAM *csram) {
    int start = 0;
    //printf("%c ", bitStream[1]);
    // Đọc 256 bit cho synapse_connection
    //bitsToByteArray(bitStream + start, csram->synapse_connection, 32);
    for(int i = 0;i<256;i++){
        // printf("%d ", bitStream[i]);
        if (bitStream[i] == 49)
        {
            csram->synapse_connection[256-i-1]=1;
        } else if (bitStream[i]==48){
            csram->synapse_connection[256-i-1]=0;
        }
        
        //csram->synapse_connection[i] = bitsToUnsignedInt(bitStream+start, 1);
        //printf("%d ", csram->synapse_connection[i]);
        //start+=8;
    }
    start += 256;

    // Đọc 9 bit cho current_potential
    csram->current_potential = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho reset_potential
    csram->reset_potential = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 4 x 9 bit cho weight[0] đến weight[3]
    for (int i = 0; i < 4; i++) {
        csram->weight[i] = bitsToSignedInt(bitStream + start, 9);
        start += 9;
        //if(i==1) printf("weight: %d ", csram->weight[i]);
    }

    // Đọc 9 bit cho leak_value
    csram->leak_value = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho positive_threshold
    csram->positive_threshold = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho negative_threshold
    csram->negative_threshold = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 1 bit cho reset_mode
    csram->reset_mode = bitsToSignedInt(bitStream + start, 1);
    start += 1;

    // Đọc 26 bit cho spike_destination, dx, dy 9 bit each; 8 bit cho axon_destination
    csram->dx = bitsToSignedInt(bitStream + start, 9);
    start += 9;
    csram->dy = bitsToSignedInt(bitStream + start, 9);
    start += 9;
    csram->axon_destination = bitsToSignedInt(bitStream + start, 8);
    start += 8;
    //printf("%d ", csram->axon_destination);
    //Đọc 4 bit cho tick_delivery
    csram->tick_delivery = bitsToSignedInt(bitStream + start, 4);
    start += 4;
}

int main() {
    int tb_num_input[10]={0};
    int *num_input[10]={NULL};
    processNumInput(&tb_num_input[0]);
    for(int j = 0; j<10;j++){
        printf("%d ", tb_num_input[j]);
    }
    printf("\n");
    unsigned int *input[10][4]={NULL};
    processInput(input, tb_num_input, num_input);

    // FILE* file = fopen("tb_input.txt", "r");
    // char line1[34];
    // fgets(line1, sizeof(line1), file);
    // fgets(line1, sizeof(line1), file);
    // for(int i = 0; i < 30;i++){
    //     printf("%d ", line1[i]);
    // }

    // for(int i = 0; i<num_input[1][0];i++){
    //     printf("%d ", input[3][0][i]);
    // }
    int i = 0;
    do{
        printf("%d ", input[3][0][i]);
        i++;
    }while(input[3][0][i]!='\0');

    CSRAM csram_input[5][256];
    char line[371]; // 368 bits + 1 for null terminator
    {
    FILE *file = fopen("csram_000.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    int k = 0;   

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        if (line[0] == 10) {
            // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
            for(int i = 0; i<368;i++){
                line[i]=line[i+2];
            }
        }
        //printf("%d ", line[0]);
        line[strcspn(line, "\n")]='\0';
        
        processLine(line, &csram_input[0][k]);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram_input[0][k].synapse_connection[i]);
        }
       //printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           //printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        //printf("dx: %u", csram_input[0][k].dy);
        // printf("dy: %u\n", csram.dy);
        // printf("Axons destination: %u\n", csram.axon_destination);
        // printf("Tick Delivery: %u\n", csram.tick_delivery);
        // printf("\n");
        
        k++;
    }
    fclose(file);
    }

    {
    FILE *file = fopen("csram_001.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    int k = 0;   

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        if (line[0] == 10) {
            // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
            for(int i = 0; i<368;i++){
                line[i]=line[i+2];
            }
        }
        //printf("%d ", line[0]);
        line[strcspn(line, "\n")]='\0';
        
        processLine(line, &csram_input[1][k]);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram_input[0][k].synapse_connection[i]);
        }
       //printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           //printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        //printf("dx: %u", csram_input[0][k].dy);
        // printf("dy: %u\n", csram.dy);
        // printf("Axons destination: %u\n", csram.axon_destination);
        // printf("Tick Delivery: %u\n", csram.tick_delivery);
        // printf("\n");
        
        k++;
    }
    fclose(file);
    }

    {
    FILE *file = fopen("csram_002.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    int k = 0;   

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        if (line[0] == 10) {
            // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
            for(int i = 0; i<368;i++){
                line[i]=line[i+2];
            }
        }
        //printf("%d ", line[0]);
        line[strcspn(line, "\n")]='\0';
        
        processLine(line, &csram_input[2][k]);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram_input[0][k].synapse_connection[i]);
        }
       //printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           //printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        //printf("dx: %u", csram_input[0][k].dy);
        // printf("dy: %u\n", csram.dy);
        // printf("Axons destination: %u\n", csram.axon_destination);
        // printf("Tick Delivery: %u\n", csram.tick_delivery);
        // printf("\n");
        
        k++;
    }
    fclose(file);
    }

    {
    FILE *file = fopen("csram_003.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    int k = 0;   

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        if (line[0] == 10) {
            // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
            for(int i = 0; i<368;i++){
                line[i]=line[i+2];
            }
        }
        //printf("%d ", line[0]);
        line[strcspn(line, "\n")]='\0';
        
        processLine(line, &csram_input[3][k]);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram_input[0][k].synapse_connection[i]);
        }
       //printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           //printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        //printf("dx: %u", csram_input[0][k].dy);
        // printf("dy: %u\n", csram.dy);
        // printf("Axons destination: %u\n", csram.axon_destination);
        // printf("Tick Delivery: %u\n", csram.tick_delivery);
        // printf("\n");
        
        k++;
    }
    fclose(file);
    }

    {
    FILE *file = fopen("csram_004.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    int k = 0;   

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        if (line[0] == 10) {
            // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
            for(int i = 0; i<368;i++){
                line[i]=line[i+2];
            }
        }
        //printf("%d ", line[0]);
        line[strcspn(line, "\n")]='\0';
        
        processLine(line, &csram_input[4][k]);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram_input[0][k].synapse_connection[i]);
        }
       //printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           //printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        //printf("dx: %u", csram_input[0][k].dy);
        // printf("dy: %u\n", csram.dy);
        // printf("Axons destination: %u\n", csram.axon_destination);
        // printf("Tick Delivery: %u\n", csram.tick_delivery);
        // printf("\n");
        
        k++;
    }
    fclose(file);
    }

    int output[10][10];
    for(int k = 0; k<10; k++){
        int input_last[256]={0};
        int num_input_last=0;
        int output_core[5][256]={0};
        for(int i = 0; i<4; i++){
            
            for(int j = 0; j<64;j++){
                output_core[i][j]=NeuronBlock(&csram_input[i][j], input[k][i], num_input[k][i]);
                if(output_core[i][j]==1){
                    input_last[num_input_last]=i*64+j;
                    num_input_last++;
                }
            }
        }
        for(int i = 0;i<250;i++){
            output_core[4][i]=NeuronBlock(&csram_input[4][i], input_last, num_input_last);
            if(output_core[4][i]==1){
                output[k][i%10]++;
            }
        }
        printf("vote voi so %d: ", k);
        for(int j = 0; j<10; j++){
            printf("%d ", output[k][j]);
        }
        printf("\n");
    }
    return 0;
}