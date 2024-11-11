#ifndef NRF24L01_H
#define NRF24L01_H


// Global variables
extern int ch_width_1;
extern int ch_width_2;
extern int ch_width_3;
extern int ch_width_4;
extern int ch_width_5;
extern int ch_width_6;
extern int ch_width_7;
extern int ch_width_8;


extern const uint64_t pipeIn;      // Communication pipe address

extern unsigned long lastRecvTime; // Time of the last data reception

// Function Prototypes
void ResetData();
void rxsetup();
void recvData();
void rxloop();

#endif // RECEIVER_H
