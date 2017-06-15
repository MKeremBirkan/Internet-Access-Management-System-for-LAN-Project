
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include "Authentication.h" 
 
#define DEST_MAC0    0x70
#define DEST_MAC1    0x71
#define DEST_MAC2    0xbc
#define DEST_MAC3    0xbd
#define DEST_MAC4    0xb1
#define DEST_MAC5    0x82
 
#define ETHER_TYPE    0x0800

#define DEFAULT_IF    "eth0"
#define BUF_SIZE    1500

int searchMac(uint8_t*);
void addMac(uint8_t *, struct tm *, char*);

char sender[INET6_ADDRSTRLEN];

//linked list for MAC addresses
typedef struct MAC_list {
        char IP[20];
        uint8_t mac[6];
        struct tm * timeinfo;
        struct MAC_list * next;
} MAC_list;

// Initialization of linked list nodes
MAC_list *head = NULL;
MAC_list *last = NULL;
MAC_list *current = NULL;
MAC_list *newMac = NULL;
MAC_list *temp = NULL;

// Yeni node yaratma kısmı, gelen verinin atamalarının yapıldığı yer
MAC_list* createNewMac(uint8_t *mac, struct tm *startTime, char *IP)
{
    int i = 0;
    MAC_list *newMac = NULL;
    newMac = (MAC_list*)malloc(sizeof(MAC_list));
    strcpy(newMac -> IP, IP);
    newMac -> timeinfo = startTime;
    while(i < 6){
    	newMac -> mac[i] = mac[i];
    	i++;
    }
    newMac -> next = NULL;

	//Printing MAC address of client
    i=0;
    while(i < 6){
		printf("%02X ", newMac -> mac[i]);
		i++;
    }
    return newMac;
}

// İnternet giriş - çıkışının kontrol edildiği yer
void ACCEPT_DROP(int arg,char* IP)
{
	int k;
	char code[100];
	puts(IP);
	if(arg==1) // girişe izin veriyor
	{
		sprintf(code,"iptables -I FORWARD -s %s -j ACCEPT",IP);
		system(code);
	}
	else if(arg==0) // izni siliyor
	{
		printf("\n\n\n\n\n\n\nİZİN SİLİNDİ %s\n\n\n\n\n\n", IP);
		sprintf(code,"iptables -D FORWARD -s %s -j ACCEPT",IP);
		for (k = 0; k < 10; k++);
			system(code);
	}
}

			
// MAC listesini spesifik bir MAC adresi için arama
int searchMac(uint8_t* mac)
{
    temp = head;
    int i = 0, k = 0;
    int exit = 0;
    while(temp != NULL)
    {
	k = compareUint8_t(temp->mac, mac, 6);
        if(k)
        {	
		
           return 1; // Buldu, return 1

        }
        temp = temp -> next;
   }
   return 0; // Bulamadı, return 0

}

// Node'lar arasında gezip 
void remainingTime(uint8_t* mac)
{
    temp = head;
    int i = 0, k = 0;
    int exit = 0;
    while(temp != NULL)
    {
	k = compareUint8_t(temp->mac, mac, 6);
        if(k)
        {	
		time_t rawtime;	
		time(&rawtime);

                    int diffrence = difftime(rawtime,mktime(temp->timeinfo));

            if(diffrence == 180) // 3 dakika olduysa internetini kes
		    {	
				ACCEPT_DROP(0,sender);		
		    }
           
           
        }
        temp = temp -> next;
    } 
	
}

// MAC listesini kontrol et, aynı MAC yoksa yeni node yaratma fonksiyonuna gönder
void addMac(uint8_t *mac, struct tm *startTime, char *IP)
{
    if (head == NULL)
    {
        head = createNewMac(mac, startTime, IP);
    	printf("HEADE GİRDİ");
        return;
    }


    temp = head;
    int g=0;
    while (temp != NULL)
    {
        last = temp;
        puts(temp->IP);
        printf("%d\n",g);
        g++;
        temp = temp -> next;
    }
    printf("\n\n\n\n\nHEADE GİRMEDİ\n\n\n\n");
    last -> next = createNewMac(mac, startTime, IP);
       
}


int main(int argc, char *argv[]) {

	//Bütün listeyi sıfırla
	system("iptables -F");
	
	//Bütün gelen verileri reddet
	int l;
	for(l = 0; l < 5; l++);
		system("iptables -I FORWARD --in-interface eth0 -j REJECT");

	printf("ALL COMPUTERS REJECTED\n");

	// Key üretme fonksiyonunu çağır
	createkey();

    int sockfd, ret, i;
    int sockopt;
    ssize_t numbytes;
    struct ifreq ifopts;
    struct sockaddr_storage their_addr;
    uint8_t buf[BUF_SIZE];
    uint8_t mac[6];

 
    char ifName[IFNAMSIZ];
   
    double diffrence;
    time_t rawtime;
    struct tm * timeinfo;

   
    MAC_list *head = malloc(sizeof(MAC_list));
    MAC_list *last = malloc(sizeof(MAC_list));
    MAC_list *current = malloc(sizeof(MAC_list));
    MAC_list *newMac = malloc(sizeof(MAC_list));
    MAC_list *temp = malloc(sizeof(MAC_list));

 
    /* Header Structures */
    struct ether_header *eh = (struct ether_header *) buf;
    struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
    struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));
 
    /* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
    if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
        perror("listener: socket");
        return -1;
    }
     
    /* Get interface name */
    strcpy(ifName, DEFAULT_IF);
     
    /* Set interface to promiscuous mode */
    strncpy(ifopts.ifr_name, ifName, IFNAMSIZ - 1);
    ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
    ifopts.ifr_flags |= IFF_PROMISC;
    ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
 
    /* Allow the socket to be reused */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
 
    /* Bind to device */
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ - 1) == -1) {
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    head=NULL;
    while (1) {
        numbytes = recvfrom(sockfd, buf, BUF_SIZE, 0, NULL, NULL);
        /* Check the packet is for me */
        if (eh->ether_dhost[0] == DEST_MAC0 && eh->ether_dhost[1] == DEST_MAC1
                && eh->ether_dhost[2] == DEST_MAC2
                && eh->ether_dhost[3] == DEST_MAC3
                && eh->ether_dhost[4] == DEST_MAC4
                && eh->ether_dhost[5] == DEST_MAC5) {
            /* Correct destination MAC address */
 
            /* Get source IP */
            ((struct sockaddr_in *) &their_addr)->sin_addr.s_addr = iph->saddr;
            inet_ntop(AF_INET, &((struct sockaddr_in*) &their_addr)->sin_addr, sender, sizeof sender);
             
             
            //printf("client IP");
            printf("Source IP: %s\n", sender);
            /* UDP payload length */
            ret = ntohs(udph->len) - sizeof(struct udphdr);
 
            printf("UDP Payload Length: %d\n", ret);
             
		int z=0;
		//UDP Packet search for MAC address and get MAC address
		for (i = 0; i < numbytes; i++)
		{
			
			if(i>=6 && i<12)
			{
			    mac[z]=buf[i];
			    printf("%02X ",mac[z]);
			    z++;
			   
			}
		}

		//connection to authentication server
		int v=searchMac(mac);
		
		// MAC linked listte kayıtlı ise zamanı kontrol et
		if(v==1)
		{
			time(&rawtime);
			   
			remainingTime(mac);				    
		}
		// MAC linked listte kayıtlı değil ise MAC'i kaydetmeye gir
		else
		{
		
			uint8_t token_variable[5]={0x54,0x4F,0x4B,0x45,0x4E};//"TOKEN" in hexadecimal
			uint8_t token_real[10];
			  
				i=42;
		
				if(i>41 && buf[i]==token_variable[0] && buf[i+1]==token_variable[1] && buf[i+2]==token_variable[2]&& buf[i+3]==token_variable[3]&& buf[i+4]==token_variable[4]){
			
			int go=0;			
			while(go<10)
			{
				token_real[go]=buf[go+47];
				
				go++;
			}

			//Authentication keyini kontrol et
			int y=searchAuth(mac,token_real);

			// Paketten gelen authentication keyi doğru ise, MAC listesine ekleyip, internet girişine izin ver
			if(y)
			{ 
				printf("AUTHENTICATION OK"); 
				time ( &rawtime );
                timeinfo = localtime ( &rawtime );
				addMac(mac,timeinfo,sender);
				ACCEPT_DROP(1,sender);
			}
			// Paketten gelen authentication keyi yanlış ise, hiçbir şey yapma
			else
			{
				 printf("AUTHENTICATION NOT");
			}			

		}
		}            
            printf("\n");
                   
        }
 
         
    }
 
    close(sockfd);
    return 0;
}

