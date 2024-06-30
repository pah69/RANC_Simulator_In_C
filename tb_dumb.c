#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


// khai bao cac const:
#define NUM_tb_num_inputs 10  
#define TEST 22

// khai bao cac bien global:
int tb_num_inputs_index = 0;
int tb_inputs_index = 0;
int arr_tb_num_inputs[NUM_tb_num_inputs] = {0};
typedef struct _tb_input_data
{
    // example; 000000001 000000000 00011011 0000
    unsigned dx;                // 9 bit
    unsigned dy;                // 9 bit
    unsigned axon_destination;   // 8 bit
    unsigned tick_delivery;      // 4 bits
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
void tb_input_line_process(const char *bitStream, tb_input_data *tb_input_data);
/*
    fucncion to read data from tb_input.txt into arr[i][j][k][l]
    ham de doc gia tri trong file tb_input.txt vao mang arr[i][j][k][l]
        trong do (in this arr):
            i,j: dx and dy to locate the core data need to go to
            k: pics number
            l: the index for data of pic k
*/
void tb_input_to_array(int (*arr_tb_input)[2][4][100]); 

int main()
{
    // int arr_tb_num_inputs[NUM_tb_num_inputs] = {0};
    tb_num_inputs_to_array(arr_tb_num_inputs);

    #if (TEST == 1)
        printf("testing tb_num_inputs to array:\n");
        for (int i = 0; i < NUM_tb_num_inputs; i++)
        {
            printf("%d: %d\n", i, arr_tb_num_inputs[i]);
        }
    #endif 
        
    // declair the arr[i][j][k][l] mention above 
    int arr_tb_input[4][2][4][100] = {{{{0}}}};

    tb_input_to_array(arr_tb_input);

    #if (TEST == 3)
        printf("\n********\nTest tb_input array value\n********\n");
        for (int k = 0; k < 10; k++)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int l = 0; l < 100; l++)
                {
                    if (arr_tb_input[i][0][k][l] == '\0')
                        break;
                    printf("arr_tb_input[%d][0][%d][%d] = %d\n", i, k, l, arr_tb_input[i][0][k][l]);        
                }
            }
        }
        
        printf("\n********\nEnd of test tb_input array value\n********\n");
    #endif 

    return 0;
}


// functions definition:
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
}

// 
void tb_input_to_array(int (*arr_tb_input)[2][4][100])
/* https://stackoverflow.com/questions/65649438/how-to-create-a-pointer-to-a-multi-dimensional-array-in-c */
{
    FILE *tb_input_p;
    tb_input_p = fopen("tb_input.txt", "r");
    if (tb_input_p == NULL)
    {
        perror("Failed to open tb_input.txt");
        return;
    } 

    char line [30];
    tb_input_data tb_in_data;
    unsigned pic_num = 0;
    unsigned pic_index = 0;
    unsigned sum = 0;
    unsigned _dx = 0;

    #if (TEST == 2)
            printf("\n********\nTest tb_input reading\n********\n");
    #endif

    while (fgets(line, sizeof(line), tb_input_p) != NULL)
    {
        #if (TEST == 2)
            int pic_change = 0;    
        #endif
        
        tb_input_line_process(line, &tb_in_data);

        #if (TEST == 22)
            printf("Read line: %s\n", line);
        #endif

        unsigned dx, dy, ad;
        dx = tb_in_data.dx;     
        dy = tb_in_data.dy;
        ad = tb_in_data.axon_destination;  

        #if (TEST == 22)
            printf("Processed line: dx = %u, dy = %u, ad = %u\n", dx, dy, ad);
        #endif

        if ((_dx == 3) && (dx == 0) && (sum == arr_tb_num_inputs[pic_num]))
        {
            pic_num += 1;
            pic_index = 0;
            sum = 0;
            #if (TEST == 2)
                int pic_change = 1;    
            #endif
        }
        else if (_dx != dx)
        {
            pic_index = 0;
        }

        arr_tb_input[dx][dy][pic_num][pic_index] = ad;

        #if (TEST == 2)
            printf("arr_tb_input[%d][%d][%d][%d] = %d\n", dx, dy, pic_num, pic_index, arr_tb_input[dx][dy][pic_num][pic_index]);
            if (pic_change)
                printf("\n");
        #endif

        _dx = dx;
        pic_index += 1;
        sum += 1;
    }     

    fclose(tb_input_p);
    #if (TEST == 22)
            printf("CLOSED FILE\n");
        #endif
    #if (TEST == 2)
        printf("\n********\nEnd of test tb_input\n********\n");
    #endif
}