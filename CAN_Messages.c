#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define table_size 1000 //CHAANGE THIS TO HOW MANY CAN_MESSAGES YOU NEED TO HANDLE!!!!!!

/** struct for CAN messages
 *  Contains a 32-bit CAN id 
 *  Contains a funtion pointer that will point to whatever funciton the CAN id corresponds to
 *  Contains a pointer to next, which is a handle for two CAN id's having the hash index
 *  The next will be a pointer in the hash table at the hash index that is duplicate and will point to the CAN id of the thing with the same hash
*/
typedef struct CAN_Message {
    uint32_t CAN_id; 
    void* function; 
    struct CAN_Message* next;
}CAN_Message; 

/** Hash Table To Store CAN Messages
 *  Creates a hash table of size table_size and type CAN_Message
 *  Initialize all CAN messages in the hash table
*/

CAN_Message CAN_message_table[table_size] = {0};

/** HASH FUNCTION
*   Take a can id and return a "random" hash id
*   The hash id is in range from 0 to table_size
*   The hash id is similar to an array index in its implementation
*/
unsigned int create_hash_id(uint32_t Incoming_CAN_id) {
    unsigned int hash = 0;
    uint32_t id = Incoming_CAN_id; 
   
    for (int i = 0; i < 4; i++) { 
        hash += ((id >> (8 * i)) & 0xFF) * i;                                                      
        hash = (hash * 31);
    }
    return hash % table_size; 
}

/** Function to take CAN id and find its corresponding function
*   Given a CAN id, find it in the hash table and call the function if it exists
*   If it doesn't exist, return 1
*   If it does exist but there are multiple can ids with the same hash
*   follow the next pointer until the right CAN id is found
*   Then call the function
*/
int find_function_from_CAN_id(uint32_t CAN_id, uint8_t* data, uint8_t length) {
    unsigned int index = create_hash_id(CAN_id);                                               
    CAN_Message* current = &CAN_message_table[index];
    
    while (current != NULL) {
        if (current->CAN_id == CAN_id) {
            void (*function_ptr)(uint8_t* data, uint8_t length) = (void (*)(uint8_t*, uint8_t))current->function;
            if (function_ptr != NULL) {
                function_ptr(data, length);  
            }
            return 0;
        }
        current = current->next;                                                                
    }
    return 1;                                                                                  
}

/** Function to insert a message into the hash table
 *  Checks if specific hash id already exists in the hash table
 *  If not, insert the message
 *  If it already exists, malloc a new message and insert it
*/
void insert_message(CAN_Message message) {
    unsigned int index = create_hash_id(message.CAN_id);                                           
    
    if(CAN_message_table[index].CAN_id == message.CAN_id) {  
        CAN_Message* temp = malloc(sizeof(CAN_Message));
        temp->CAN_id = message.CAN_id;
        temp->function = message.function;
        temp->next = CAN_message_table[index].next;
        CAN_message_table[index].next = temp;
        
    }else{
        CAN_message_table[index] = message;                                                         
        CAN_message_table[index].next = NULL;
    }
}


/**  Function to free all malloced memory
*    Index through the hash table and free all the malloced memory at each index
*/
void free_all_memory() {
    for (int i = 0; i < table_size; i++) {
        CAN_Message* current = CAN_message_table[i].next;
        CAN_Message* temp;
        
        while (current != NULL) {
            temp = current;
            current = current->next;
            free(temp);
        }
        
        CAN_message_table[i].next = NULL;
    }
}
int main() {
    
    return 0;
}
