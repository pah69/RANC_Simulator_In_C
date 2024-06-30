#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// khai bao cac const:
#define NUM_tb_num_inputs 10  
#define TEST 0

// khai bao cac bien global:
int tb_num_inputs_index = 0;
int tb_inputs_index = 0;
int arr_tb_num_inputs[NUM_tb_num_inputs] = {0};
typedef struct _tb_input_data
{
    // example; 000000001 000000000 00011011 0000
    unsigned dx;                // 9 bit
    unsigned dy;                // 9 bit
    unsigned axon_destination;  // 8 bit
    unsigned tick_delivery;     // 4 bits
} tb_input_data;
typedef struct _CSRAM{
    uint8_t synapse_connection[32]; // 256 bits (32 bytes)
    uint16_t current_potential;     // 9 bits
    uint16_t reset_potential;       // 9 bits
    uint16_t weight[4];             // 4 x 9 bits
    uint16_t leak_value;            // 9 bits
    uint16_t positive_threshold;    // 9 bits
    uint16_t negative_threshold;    // 9 bits
    uint8_t reset_mode;             // 1 bit
    uint16_t dx;                    // 9 bit
    uint16_t dy;                    // 9 bit
    uint8_t axon_destination;       // 8 bit
    uint8_t tick_delivery;          // 4 bits
} CSRAM;


// function definition:

// Neuron related function:
unsigned int NeuronBlock(CSRAM csram, int *input);

// CSRAM related function:
// Hàm để chuyển đổi chuỗi bit thành giá trị số nguyên
unsigned int bitsToUnsignedInt(const char *bits, int length);
// Hàm để chuyển đổi chuỗi bit thành mảng byte
void bitsToByteArray(const char *bits, char *byteArray, int byteArrayLength);
// Ham xu ly line trong file csram
void processLine(const char *bitStream, CSRAM *csram);
/*
    Ham xu ly file csram va tra ve array
        filename: ten cua file can truyen vao, vd: csram_000.mem
        csram_input: mang muon truyen gia tri vao
        csram_num: index cho csram_input
*/ 
void csram_to_array(char *filename, CSRAM csram_input[5][256], unsigned csram_num);

// tb_num_inputs tb_input related funciton:
/*
    ham de doc gia tri trong file tb_num_input.txt vao 1 mang
    function to read data from tb_num_input.txt into an array 
*/  
void tb_num_inputs_to_array(int *arr_tb_num_inputs);
/*
    ham de xu ly 1 dong trong tb_input.txt
    function to process a line in tb_input.txt
*/
void tb_input_line_process(const char *bitStream, tb_input_data *tb_input_data);
/*
    fucncion to read data from tb_input.txt into arr[i][j][k][l]
    ham de doc gia tri trong file tb_input.txt vao mang arr[i][j][k][l]
        trong do (in this arr):
            i,j: dx and dy to locate the core data need to go to
            k: pics number
            l: the index for data of pic k
*/
void tb_input_to_array(int arr_tb_input[4][2][10][200]); 


int main() 
{
    tb_num_inputs_to_array(arr_tb_num_inputs);      
    // declair the arr[i][j][k][l] mention above 
    int arr_tb_input[4][2][10][200] = {{{{0}}}};

    tb_input_to_array(arr_tb_input);

    CSRAM csram_input[5][256];
    
    csram_to_array("csram_000.mem", csram_input, 0);
    csram_to_array("csram_001.mem", csram_input, 1);
    csram_to_array("csram_002.mem", csram_input, 2);
    csram_to_array("csram_003.mem", csram_input, 3);
    csram_to_array("csram_004.mem", csram_input, 4);

    printf("\nsos: %d, %d\n", arr_tb_input[0][0][0][0], arr_tb_input[0][0][1][0]);

    // CSRAM csram_input[5][256]={0};
    // printf("\n%d\n", csram_input[0][1].dx);
    int output[10][10];
    for(int k = 0; k<10; k++)
    {
        int input_last[256]={0};
        int num_input_last=0;
        // đọc input
        // xử lí
        // int *output=Neuron_Net(csram_input, &input[0][0], num_input);
        // printf("%d\n", num_input[4]);
        int output_core[5][256]={0};
        for(int i = 0; i<4; i++){
            // printf("num_input[%d][%d] = %d\n", k, i, num_input[k][i]);
            for(int j = 0; j<64;j++){
                output_core[i][j]=NeuronBlock(csram_input[i][j], arr_tb_input[i][0][k]);
                if(output_core[i][j]==1){
                    input_last[num_input_last]=i*64+j;
                    num_input_last++;
                }
            }
        }
        for(int i = 0;i<250;i++){
            output_core[4][i]=NeuronBlock(csram_input[4][i], input_last);
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


// function definition:
unsigned int NeuronBlock(CSRAM csram, int *input)
{
    int pre_ff, output;
    int ff=0;
    int negative_check, positive_check;
    for(int i = 0; ((input[i] != '\0') && (i != 0)) ; i++)
    {
        pre_ff = ff;
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

void csram_to_array(char *filename, CSRAM csram_input[5][256], unsigned csram_num)
{
    char line[369]; // 368 bits + 1 for null terminator

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        // Xử lý từng dòng
        int k = 0;
        processLine(line, &csram_input[csram_num][k]);
        k++;
    }
    fclose(file);
}

void tb_num_inputs_to_array(int *arr_tb_num_inputs)
{
    FILE *tb_num_inputs_p;
    tb_num_inputs_p = fopen("tb_num_inputs.txt","r"); 
    if (tb_num_inputs_p == NULL) {
        perror("Failed to open tb_num_inputs.txt");
        return;
    }
    int index = 0;
    while ( (!feof(tb_num_inputs_p)) && // If it is EOF stop reading.
            (index < NUM_tb_num_inputs))
    {
        int i = 0;
        fscanf (tb_num_inputs_p, "%d", &i); 
        arr_tb_num_inputs[index] = i;
        index += 1;       
    } ;

    fclose(tb_num_inputs_p);
}

// Ham xu ly 1 dong trong tb_input:
void tb_input_line_process(const char *bitStream, tb_input_data *tb_input_data)
{
    int offset = 0;
    tb_input_data->dx = bitsToUnsignedInt(bitStream + offset, 9);
    offset += 9;
    tb_input_data->dy = bitsToUnsignedInt(bitStream + offset, 9);
    offset += 9;
    tb_input_data->axon_destination = bitsToUnsignedInt(bitStream + offset, 8);
    offset += 8;
    tb_input_data->tick_delivery = bitsToUnsignedInt(bitStream + offset, 4); 
    offset += 4;
}

// ham chuyen tb_input thanh array
void tb_input_to_array(int arr_tb_input[4][2][10][200])
{
    FILE *tb_input_p;
    tb_input_p = fopen("tb_input.txt", "r");
    if (tb_input_p == NULL)
    {
        perror("Failed to open tb_input.txt");
        return;
    } 

    char line [31];
    tb_input_data tb_in_data;
    unsigned pic_num = 0;
    unsigned pic_index = 0;
    unsigned sum = 0;
    unsigned _dx = 0;

    #if (TEST == 2)
        FILE *fptr;
        fptr = fopen("test2.txt", "w");
    #endif

    #if (TEST == 22)
        FILE *fptr;
        fptr = fopen("test22.txt", "w");
    #endif

    while (fgets(line, 31, tb_input_p) != NULL)
    {
        tb_input_line_process(line, &tb_in_data);

        unsigned dx, dy, ad;
        dx = tb_in_data.dx;     
        dy = tb_in_data.dy;
        ad = tb_in_data.axon_destination;  

        if (dx >= 4 || dy >= 2 || pic_num >= 10 || pic_index >= 200) {
            continue;
        }
        
        if ((_dx == 3) && (dx == 0) && (sum == arr_tb_num_inputs[pic_num]))
        {
            pic_num += 1;
            pic_index = 0;
            sum = 0;
        }
        else if (_dx != dx)
        {
            pic_index = 0;
        }

        #if (TEST == 22)
            // fprintf(fptr, "Read line: %s\n", line);
            fprintf(fptr, "Processed line: dx = %u, dy = %u, ad = %u, pic = %u, pic_index = %u\n", dx, dy, ad, pic_num, pic_index);
        #endif

        arr_tb_input[dx][dy][pic_num][pic_index] = ad;

        #if (TEST == 2)
            fprintf(fptr, "arr_tb_input[%d][%d][%d][%d] = %d\n", dx, dy, pic_num, pic_index, arr_tb_input[dx][dy][pic_num][pic_index]);            
        #endif

        _dx = dx;
        pic_index += 1;
        sum += 1;
    }     

    fclose(tb_input_p);
    #if (TEST == 22)
        fprintf(fptr, "\nCLOSED FILE\n");
        fclose(fptr);
    #endif
    #if (TEST == 2)
        fprintf(fptr, "\nCLOSED FILE\n");
        fclose(fptr);
    #endif
}