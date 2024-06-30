/*
return number explain:
101: tb_num_input.txt failed to open
102: tb_input.txt failed to open
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


// khai bao cac const:
#define NUM_tb_num_inputs 10  


// khai bao cac bien global:
int tb_num_inputs_index = 0;
int tb_inputs_index = 0;
typedef struct _tb_input_data
{
    // example; 000000001 000000000 00011011 0000
    uint16_t dx;                // 9 bit
    uint16_t dy;                // 9 bit
    uint8_t axon_destination;   // 8 bit
    uint8_t tick_delivery;      // 4 bits
} tb_input_data;


// functions declaration: 
/*
    thanks to PA.Hohohohoho    
*/
unsigned int bitsToUnsignedInt(const char *bits, int length);
void bitsToByteArray(const char *bits, char *byteArray, int byteArrayLength);
/*
    ham de doc gia tri trong file tb_num_input.txt vao 1 mang
    function to read data from tb_num_input.txt into an array 
*/  
void tb_num_inputs_to_array(int *arr_tb_num_inputs);
/*
    ham de xu ly 1 dong trong tb_input.txt
    function to process a line in tb_input.txt
*/
void tb_num_inputs_line_process(const char *bitStream, tb_input_data *tb_input_data);
/*
    fucncion to read data from tb_input.txt into arr[i][j][k][l]
    ham de doc gia tri trong file tb_input.txt vao mang arr[i][j][k][l]
        trong do (in this arr):
            i,j: dx and dy to locate the core data need to go to
            k: pics number
            l: the index for data of pic k
*/
void tb_inputs_to_array(int *arr_tb_inputs); 

int main()
{
    int arr_tb_num_inputs[NUM_tb_num_inputs] = {0};
    tb_num_inputs_to_array(arr_tb_num_inputs);

    // printf("testing\n");
    // for (int i = 0; i < NUM_tb_num_inputs; i++)
    // {
    //     printf("%d: %d\n", i, arr_tb_num_inputs[i]);
    // }

    return 0;
}


// functions definition:
void tb_num_inputs_to_array(int *arr_tb_num_inputs)
{
    FILE *tb_num_inputs_p;
    tb_num_inputs_p = fopen("tb_num_inputs.txt","r"); 
    if (tb_num_inputs_p == NULL) {
        perror("Failed to open tb_num_inputs.txt");
        return 101;
    }
    int index = 0;
    while ( (!feof(tb_num_inputs_p)) && // If it is EOF stop reading.
            (index < NUM_tb_num_inputs))
    {
        int i = 0;
        fscanf (tb_num_inputs_p, "%d", &i); 
        arr_tb_num_inputs[index] = i;
        // printf ("%d: %d\n", index, arr_tb_num_inputs[index]);
        index += 1;       
    } ;

    fclose(tb_num_inputs_p);
}

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

// Ham xu ly 1 dong trong tb_input:
void tb_num_inputs_line_process(const char *bitStream, tb_input_data *tb_input_data)
{
    int offset = 0;
    tb_input_data->dx = bitsToUnsignedInt(bitStream + offset, 9);
    offset += 9;
    tb_input_data->dy = bitsToUnsignedInt(bitStream + offset, 9);
    offset += 9;
    tb_input_data->axon_destination = bitsToUnsignedInt(bitStream + offset, 8);
    offset += 8;
    tb_input_data->tick_delivery = bitsToUnsignedInt(bitStream + offset, 4); 
}

// 
void tb_inputs_to_array(int *arr_tb_inputs)
{
    
}