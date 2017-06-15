#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void addAuth(uint8_t*);
int searchAuth(uint8_t*, uint8_t*);

uint8_t tempMac[6]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t zero[1]={0x00};

//linked list for authentication server
typedef struct Authentication_STR {
	uint8_t mac[6];
	uint8_t token[10];
	struct Authentication_STR *next;
} Authentication_STR;

// Program çalışınca keyleri listeye koy
void createkey(){

	uint8_t  token11[10]={0x31, 0x32,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41};////123456789A
	addAuth(token11);
	uint8_t  token22[10]={0x31, 0x32,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x42};////123456789B
	addAuth(token22);
	uint8_t  token33[10]={0x31, 0x32,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x43};////123456789C
	addAuth(token33);

}

//initialization for authentication nodes
Authentication_STR *authHead = NULL;
Authentication_STR *authLast = NULL;
Authentication_STR *authCurrent = NULL;
Authentication_STR *authNewAuth = NULL;
Authentication_STR *authTemp = NULL;

//create new authentication node
Authentication_STR* createNewAuth(uint8_t *token)
{
    int i = 0, k = 0;
    Authentication_STR *authNewAuth = NULL;
    authNewAuth = (Authentication_STR*)malloc(sizeof(Authentication_STR));
    //mac ataması  
    while(i < 6){
        authNewAuth -> mac[k] = zero[0];
    	i++;
    }
    //token ataması
    while(k < 10){
        authNewAuth -> token[k] = token[k];
    	k++;
    }
    authNewAuth -> next = NULL;
    return authNewAuth;
}

//add new authentication node to linked list
void addAuth(uint8_t *token)
{
    if (authHead == NULL)
    {
        authHead = createNewAuth(token);
        return;
    }
   

    authTemp = authHead;
    
    while (authTemp != NULL)
    {
        authLast = authTemp;
       
        authTemp = authTemp -> next;
    }
    
    authLast -> next = createNewAuth(token);
       
}

//compare two uint8_t arrays
int compareUint8_t(uint8_t* linkedList, uint8_t* client, int size)
{
	int lenght = 0;
	while(lenght < size)
	{
		if(linkedList[lenght] != client[lenght])
			return 0; // iki array birbirinden farklı, return 0
		
		lenght++;
	}
	
	return 1; // arrayler aynı, return 1

}			

//search for authentication node in linked list
int searchAuth(uint8_t* mac, uint8_t* token)
{
    authTemp = authHead;
    int i = 0, k = 0,z=0;
    
    while(authTemp != NULL)
    {
   	k = compareUint8_t(authTemp->mac, tempMac, 6);
	z = compareUint8_t(authTemp->token, token, 10);

	if(z == 1 && k == 1)
        {
		int w=0;
		while(w<6)
		{
			authTemp->mac[w]=mac[w];
			w++;
		}
		
	        return 1; // Found, return 1
            
        }
        authTemp = authTemp -> next;
    }

   return 0;    //Not found, return 0
}



