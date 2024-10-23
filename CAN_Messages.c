#include <stdio.h>
#include <stdint.h>

#define table_size 100 //CHAANGE THIS TO HOW MANY CAN_MESSAGES YOU NEED TO HANDLE

/** Strut for CAN messages
 * Contains a 32-bit CAN id 
 * Contains a function pointer that will point to whatever funciton the CAN id corresponds to
 * Contains a pointer to next, which is a handle for two CAN id's having the hash index
 * The next will be a pointer in the hash table at the hash index that is duplicate and will point to the CAN id of the thing with the same hash
*/
typedef struct CAN_Message {
    uint32_t CAN_id; 
    void* function; //------------------------------------------------------> CHANGE THIS TO TAKE A UINT8_T ARGUMENT AND ADJUST THE REST OF THE CODE TO MATCH IT*************
    struct CAN_Message* next;
}CAN_Message; 

//Create the hash table (array of CAN messages)
CAN_Message CAN_message_table[table_size] = {0};

//HASH FUNCTION (Take a can id and return a "random" hash id)
unsigned int create_hash_id(uint32_t Incoming_CAN_id) {
    unsigned int hash = 0;
    uint32_t id = Incoming_CAN_id; 
   
    for (int i = 0; i < 4; i++) { 
        hash += ((id >> (8 * i)) & 0xFF) * i;                                       //Scramble the id
        hash = (hash * 31);
    }
    return hash % table_size; 
}

//Function to take CAN id and find its corresponding function
int find_function_from_CAN_id(uint32_t CAN_id){
    unsigned int index = create_hash_id(CAN_id);                                    //Get the index in the hash table based on the CAN id
     
    if (CAN_message_table[index].CAN_id == CAN_id){ 
        void (*function_ptr)() = (void (*)())CAN_message_table[index].function;     //Cast to a void function pointer that takes no arguments? 
        if (function_ptr != NULL) {
            function_ptr();                                                         // Call the function if it exisits
        }
        return 0;
    } else {
        return 1;
    }
}

//Function to insert a message into the hash table
void insert_message(CAN_Message message) {
    unsigned int index = create_hash_id(message.CAN_id);                            //Get the index in the hash table based on the CAN id
    CAN_message_table[index] = message;                                         // Insert message at hashed index
}

int main(){    

    return 0;
}
