#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint8_t synapse_connection[32]; // 256 bits (32 bytes)
    uint16_t current_potential; // 9 bits
    uint16_t reset_potential; // 9 bits
    uint16_t weight[4]; // 4 x 9 bits
    uint16_t leak_value; // 9 bits
    uint16_t positive_threshold; // 9 bits
    uint16_t negative_threshold; // 9 bits
    uint8_t reset_mode; // 1 bit
    //spike_destination; 26 bit 
    // dx + dy + axon_destination = 9 + 9 + 8
    uint16_t dx; // 9 bit
    uint16_t dy; // 9 bit
    uint8_t axon_destination; // 8 bit
    uint8_t tick_delivery; // 4 bits
} CSRAM;

unsigned int NeuronBlock(CSRAM csram, int *input, int8_t num_of_input){
    int pre_ff, output;
    int ff=0;
    int negative_check, positive_check;
    for(int i = 0; i<num_of_input;i++){
        pre_ff=ff;
        int process = csram.synapse_connection[input[i]];
        if(process == 0){
            continue;
        }else{
            int weight;
            if(input[i]%2==0){
                weight = csram.weight[0];
            }else{
                weight = csram.weight[1];
            }
            ff = weight + pre_ff;
        }
        printf("\n");
    }
    int value = ff+csram.leak_value;

    if(value > csram.positive_threshold){
        output=1;
        //printf("co xung ne \n");
    }
    else{
        output=0;
    }
    return output;
}

// int *Neuron_Net(CSRAM csram[][256], int8_t *input[],int8_t *num_input){
//     static int output[10]={0};
//     unsigned int output_core[5][256];
//     for(int i = 0; i<4; i++){
//         for(int j = 0; j<64;j++){
//             output_core[i][j]=NeuronBlock(csram[i][j], input[i], num_input[i]);
//             if(output_core[i][j]==1){
//                 input[4][num_input[4]]=i*64+j;
//                 num_input[4]++;
//             }
//         }
//     }
//     for(int i = 0;i<250;i++){
//         output_core[4][i]=NeuronBlock(csram[4][i], input[4], num_input[4]);
//         if(output_core[4][i]==1){
//             output[i%10]++;
//         }
//     }
//         //viết file output_core ra file txt;
//     output[0]=2;
//     return output;
// }



// Hàm để chuyển đổi chuỗi bit thành giá trị số nguyên
unsigned int bitsToUnsignedInt(const char *bits, int length) {
    unsigned int value = 0;
    for (int i = 0; i < length; i++) {
        value = (value << 1) | (bits[i] - '0');
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

    char line[256]; // 368 bits + 1 for null terminator
    //CSRAM csram_input[5][256];
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
    if (file == NULL) {
        printf("Không thể mở file.\n");
    }
    for(int i=0;i<10;i++){
        char line[256]; // 368 bits + 1 for null terminator
        //CSRAM csram_input[5][256];
        for(int j = 0;j<tb_num_input[i];j++){
            if (fgets(line, sizeof(line), file) != NULL) {
                int core = bitsToUnsignedInt(line ,9);
                //printf("%d ", core);
                if (input[i][core] == NULL) {
                    input[i][core] = malloc(256 * sizeof(int)); // Cấp phát bộ nhớ cho `input[i][j]`
                }
                if (num_input[i] == NULL) {
                    num_input[i] = malloc(256 * sizeof(int)); // Cấp phát bộ nhớ cho `input[i][j]`
                    for (int n = 0; n < 4; n++) {
                        num_input[i][n] = 0; // Khởi tạo giá trị cho num_input[i]
                    }
                }
                input[i][core][num_input[i][core]]=bitsToUnsignedInt(line + 18, 8);
                //printf("input[%d][%d][%d] = %d ", i, core, num_input[i][core], input[i][core][num_input[i][core]]);
                num_input[i][core]++;
            } else {
                break;
            }
        }
    }

    // Đóng file
    fclose(file);
    //return num_input;
}

void processLine(const char *bitStream, CSRAM *csram) {
    int start = 0;

    // Đọc 256 bit cho synapse_connection
    bitsToByteArray(bitStream + start, csram->synapse_connection, 32);
    start += 256;

    // Đọc 9 bit cho current_potential
    csram->current_potential = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho reset_potential
    csram->reset_potential = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 4 x 9 bit cho weight[0] đến weight[3]
    for (int i = 0; i < 4; i++) {
        csram->weight[i] = bitsToUnsignedInt(bitStream + start, 9);
        start += 9;
    }

    // Đọc 9 bit cho leak_value
    csram->leak_value = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho positive_threshold
    csram->positive_threshold = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho negative_threshold
    csram->negative_threshold = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 1 bit cho reset_mode
    csram->reset_mode = bitsToUnsignedInt(bitStream + start, 1);
    start += 1;

    // Đọc 26 bit cho spike_destination, dx, dy 9 bit each; 8 bit cho axon_destination
    csram->dx = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;
    csram->dy = bitsToUnsignedInt(bitStream + start, 9);
    start += 9;
    csram->axon_destination = bitsToUnsignedInt(bitStream + start, 9);
    start += 8;
    // Đọc 4 bit cho tick_delivery
    csram->tick_delivery = bitsToUnsignedInt(bitStream + start, 4);
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

    FILE *file = fopen("csram_000.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char line[369]; // 368 bits + 1 for null terminator
    CSRAM csram_input[5][256];

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        int k = 0;
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
    printf("\nsos: %d, %d\n", input[0][0][0], input[0][1][0]);
    //CSRAM csram_input[5][256]={0};
    //printf("\n%d\n", csram_input[0][1].dx);
    int output[10][10];
    for(int k = 0; k<10; k++){
        int input_last[256]={0};
        int num_input_last=0;
        //đọc input
        //xử lí
        //int *output=Neuron_Net(csram_input, &input[0][0], num_input);
        //printf("%d\n", num_input[4]);
        int output_core[5][256]={0};
        for(int i = 0; i<4; i++){
            //printf("num_input[%d][%d] = %d\n", k, i, num_input[k][i]);
            for(int j = 0; j<64;j++){
                output_core[i][j]=NeuronBlock(csram_input[i][j], input[k][i], num_input[k][i]);
                if(output_core[i][j]==1){
                    input_last[num_input_last]=i*64+j;
                    num_input_last++;
                }
            }
        }
        for(int i = 0;i<250;i++){
            output_core[4][i]=NeuronBlock(csram_input[4][i], input_last, num_input_last);
            if(output_core[4][i]==1){
                output[k][i%10]++;
            }
        }
        //viết file output_core ra file txt;
        printf("vote voi so %d: ", k);
        for(int j = 0; j<10; j++){
            printf("%d ", output[k][j]);
        }
        printf("\n");
    }
    return 0;
}