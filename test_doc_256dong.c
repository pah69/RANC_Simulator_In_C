/*
    TEST number meaning:
    2x: test related to tb_input
        20: test value of tb_input array in main
    5x: test related to csram_input
        50: test value of csram_input array in main
        51: test value of csram_input array in csram_to_array funciton
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// khai bao cac const:
#define NUM_tb_num_inputs 10  
#define TEST 20

// khai bao cac bien global:
int tb_num_inputs_index = 0;
int tb_inputs_index = 0;
typedef struct _CSRAM{
    int synapse_connection[256];    // 256 bits (32 bytes)
    int16_t current_potential;      // 9 bits
    int16_t reset_potential;        // 9 bits
    int16_t weight[4];              // 4 x 9 bits
    int16_t leak_value;             // 9 bits
    int16_t positive_threshold;     // 9 bits
    int16_t negative_threshold;     // 9 bits
    int8_t reset_mode;              // 1 bit
    int16_t dx;                     // 9 bit
    int16_t dy;                     // 9 bit
    int8_t axon_destination;        // 8 bit
    int8_t tick_delivery;           // 4 bits
} CSRAM;


// function definition:

// Neuron related function:
unsigned int NeuronBlock(CSRAM *csram, int input[], int8_t num_of_input);

// CSRAM related function:
// Các hàm để chuyển đổi chuỗi bit thành giá trị số hệ thập phân
unsigned int bitsToUnsignedInt(const char *bits, int length);
signed int bitsToSignedInt(const char *bits, int length);
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
    fucncion to read data from tb_input.txt into arr[i][j][k][l]
    ham de doc gia tri trong file tb_input.txt vao mang arr[i][j][k][l]
        trong do (in this arr):
            i,j: dx and dy to locate the core data need to go to
            k: pics number
            l: the index for data of pic k
*/
void tb_input_to_array(unsigned int *input[10][4], int tb_num_input[], int *num_input_per_core[10]);


int main() 
{
    int tb_num_inputs[NUM_tb_num_inputs] = {0};  
    tb_num_inputs_to_array(tb_num_inputs);
    for(int j = 0; j < 10; j++)
    {
        printf("%d ", tb_num_inputs[j]);
    }
    printf("\n");

    int *num_input_per_core[10]={NULL};
    unsigned int *input[10][4]={NULL};
    tb_input_to_array(input, tb_num_inputs, num_input_per_core);

    #if (TEST == 20) 
        // testing the correctness of csram_input array 
        FILE *fptr;
        fptr = fopen("test20.txt", "w");
        
        for (int pic; pic < 10; pic++)
        {        
            for (int core_index; core_index < 4; core_index++)
            {
                for (int pic_index = 0; pic_index < num_input_per_core[pic][core_index]; pic_index++)
                    fprintf(fptr, "input[%d][%d][%d] = %d\n", 
                    pic, core_index, pic_index, input[pic][core_index][pic_index]);
            }
        }

        fprintf(fptr, "\ninput[pic][core_index][pic_index]\n");        
        fclose(fptr);        
    #endif

    CSRAM csram_input[5][256];
    
    csram_to_array("csram_000.mem", csram_input, 0);
    csram_to_array("csram_001.mem", csram_input, 1);
    csram_to_array("csram_002.mem", csram_input, 2);
    csram_to_array("csram_003.mem", csram_input, 3);
    csram_to_array("csram_004.mem", csram_input, 4);

    #if (TEST == 50) 
        // testing the correctness of csram_input array 
        FILE *fptr;
        fptr = fopen("test50.txt", "w");
        
        for (int csram_num = 0; csram_num <5; csram_num++)
        {
            for (int neuron_num = 0; neuron_num <256; neuron_num++)
            {
                fprintf(fptr, "csram_input[%u][%u] = %u\n", 
                        csram_num, neuron_num, csram_input[csram_num][neuron_num]);
            }
        }                 
        fclose(fptr);        
    #endif

    int output[10][10];
    for(int k = 0; k<10; k++){
        int input_last[256]={0};
        int num_input_last=0;
        int output_core[5][256]={0};
        for(int i = 0; i<4; i++){
            
            for(int j = 0; j<64;j++){
                output_core[i][j]=NeuronBlock(&csram_input[i][j], input[k][i], num_input_per_core[k][i]);
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


// function definition:
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
unsigned int bitsToUnsignedInt(const char *bits, int length) 
{
    signed int value = 0;
    for (int i = 0; i < length; i++) {
        if(bits[i]=='1'){
            value+=pow(2, length-i-1);
        }
    }
    return value;
}

signed int bitsToSignedInt(const char *bits, int length) 
{
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
void bitsToByteArray(const char *bits, char *byteArray, int byteArrayLength) 
{
    for (int i = 0; i < byteArrayLength; i++) {
        byteArray[i] = bitsToUnsignedInt(bits + (i * 8), 8);
    }
}

void processLine(const char *bitStream, CSRAM *csram) 
{
    int start = 0;

    // Đọc 256 bit cho synapse_connection
    // bitsToByteArray(bitStream + start, csram->synapse_connection, 32);
    for(int i = 0;i<256;i++)
    {
        // printf("%d ", bitStream[i]);
        if (bitStream[i] == 49)
        {
            csram->synapse_connection[256-i-1]=1;
        } else if (bitStream[i]==48){
            csram->synapse_connection[256-i-1]=0;
        }
    }    
    start += 256;

    // Đọc 9 bit cho current_potential
    csram->current_potential = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 9 bit cho reset_potential
    csram->reset_potential = bitsToSignedInt(bitStream + start, 9);
    start += 9;

    // Đọc 4 x 9 bit cho weight[0] đến weight[3]
    for (int i = 0; i < 4; i++) 
    {
        csram->weight[i] = bitsToSignedInt(bitStream + start, 9);
        start += 9;
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
    csram->axon_destination = bitsToSignedInt(bitStream + start, 9);
    start += 8;
    // Đọc 4 bit cho tick_delivery
    csram->tick_delivery = bitsToSignedInt(bitStream + start, 4);
    start += 4;
}

void csram_to_array(char *filename, CSRAM csram_input[5][256], unsigned csram_num)
{
    char line[371]; // 368 bits + '\n' + '\0' + một bit đâu cho vui

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    int k = 0;
    while (fgets(line, sizeof(line), file) != NULL) 
    {        
        // if (line[0] == 10) {
        //     // Nếu ký tự đầu dòng là newline, di chuyển con trỏ chuỗi
        //     for(int i = 0; i < 368; i++){
        //         line[i] = line[i+2];
        //     }
        // }
        // //printf("%d ", line[0]);
        // line[strcspn(line, "\n")]='\0';
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

// ham chuyen tb_input thanh array
void tb_input_to_array(unsigned int *input[10][4], int tb_num_input[], int *num_input_per_core[10])
{
    FILE *tb_input_p;
    tb_input_p = fopen("tb_input.txt", "r");
    if (tb_input_p == NULL)
    {
        perror("Failed to open tb_input.txt\n");
        return;
    } 

    for (int pic; pic < 10; pic++)
    {
        /*
            This loop ensure we get the correct amount of input per picture follow in the
            tb_num_input.txt. 
            It's also track number of input per picture per core with the 
            num_input_per_core array.
        */
        for (int pic_input_index; pic_input_index < tb_num_input[pic]; pic_input_index++)
        {
            /*
                In this application, the input only go into dy = 0 so whe can totally just
                mind dx or in this case it's called core and have range of [0;3] 
            */
            char line[32];   
            while (fgets(line, sizeof(line), tb_input_p) != NULL)
            {
                int core = bitsToUnsignedInt(line, 9);
                if (input[pic][core] == NULL)
                {
                    input[pic][core] = malloc(256 * sizeof(int));
                    for (int pic_input_index = 0; pic_input_index < 256; pic_input_index ++)                    
                        input[pic][core][pic_input_index] = 0;                    
                }

                if (num_input_per_core[pic] == NULL)
                {
                    num_input_per_core[pic]  = malloc(4 * sizeof(int));
                    for (int core_init = 0; core_init < 4; core_init++)
                    {
                        num_input_per_core[pic][core_init] = 0;
                    }
                }
                int pic_index = num_input_per_core[pic][core];
                input[pic][core][pic_index] = bitsToUnsignedInt(line + 18, 8);

                num_input_per_core[pic][core] += 1;
            }
        }
        
    }  

    fclose(tb_input_p);    
}