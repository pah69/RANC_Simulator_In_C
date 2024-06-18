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

typedef struct {

}NeuronBlock;





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
    FILE *file = fopen("csram_000.mem", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char line[369]; // 368 bits + 1 for null terminator
    CSRAM csram;

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        processLine(line, &csram);

        // In các giá trị trong cấu trúc để kiểm tra
       // printf("Synapse Connection: ");
        for (int i = 0; i < 32; i++) {
            //printf("%02X", (unsigned char)csram.synapse_connection[i]);
        }
       // printf("\n");

        //printf("Current Potential: %u\n", csram.current_potential);
       // printf("Reset Potential: %u\n", csram.reset_potential);
        for (int i = 0; i < 4; i++) {
           // printf("Weight[%d]: %u\n", i, csram.weight[i]);
        }
        //printf("Leak value: %u\n", csram.leak_value);
        //printf("Positive Threshold: %u\n", csram.positive_threshold);
        //printf("Negative Threshold: %u\n", csram.negative_threshold);
        //printf("Reset Mode: %u\n", csram.reset_mode);
        /*printf("dx: %u\n", csram.dx);
        printf("dy: %u\n", csram.dy);
        printf("Axons destination: %u\n", csram.axon_destination);
        printf("Tick Delivery: %u\n", csram.tick_delivery);
        printf("\n");
        */
    }

    fclose(file);

    return 0;
}