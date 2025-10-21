/*
 * Copyright (C) 2009 - 2022 Xilinx, Inc.
 * Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "limits.h"
#include "memory_data_fmnist_3_weightszero.h"
#include "memory_data_fmnist_3_weightsone.h"
#include "memory_data_fmnist_3_weightstwo.h"
#include "memory_data_fmnist_3_clauses.h"

#include "memory_data_fmnist_5_weightszero.h"
#include "memory_data_fmnist_5_weightsone.h"
#include "memory_data_fmnist_5_weightstwo.h"
#include "memory_data_fmnist_5_clauses.h"

#include "memory_data_fmnist_7_weightszero.h"
#include "memory_data_fmnist_7_weightsone.h"
#include "memory_data_fmnist_7_weightstwo.h"
#include "memory_data_fmnist_7_clauses.h"

#include "memory_data_kmnist_3_weightszero.h"
#include "memory_data_kmnist_3_weightsone.h"
#include "memory_data_kmnist_3_weightstwo.h"
#include "memory_data_kmnist_3_clauses.h"

#include "memory_data_kmnist_5_weightszero.h"
#include "memory_data_kmnist_5_weightsone.h"
#include "memory_data_kmnist_5_weightstwo.h"
#include "memory_data_kmnist_5_clauses.h"

#include "memory_data_kmnist_7_weightszero.h"
#include "memory_data_kmnist_7_weightsone.h"
#include "memory_data_kmnist_7_weightstwo.h"
#include "memory_data_kmnist_7_clauses.h"

#include "memory_data_mnist_3_weightszero.h"
#include "memory_data_mnist_3_weightsone.h"
#include "memory_data_mnist_3_weightstwo.h"
#include "memory_data_mnist_3_clauses.h"

#include "memory_data_mnist_5_weightszero.h"
#include "memory_data_mnist_5_weightsone.h"
#include "memory_data_mnist_5_weightstwo.h"
#include "memory_data_mnist_5_clauses.h"

#include "memory_data_mnist_7_weightszero.h"
#include "memory_data_mnist_7_weightsone.h"
#include "memory_data_mnist_7_weightstwo.h"
#include "memory_data_mnist_7_clauses.h"


#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"
#include "xil_io.h"
#include "xparameters.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "xuartps_hw.h"
#include "xaxidma.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xemacps.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "lwip/tcp.h"
#include "xuartps_hw.h"
#include "xparameters.h"
#include "sleep.h"
#define UART_BASEADDR XPAR_XUARTPS_1_BASEADDR

#include "fpga_power.h"

// extern struct netif *echo_netif;
#define GPIO_DATA_OFFSET 0x0
#define GPIO_TRI_OFFSET  0x4

/* Adjust counts to match arrays below */
#define NUM_OUTPUTS 2
#define NUM_INPUTS 1

#define WORDS_PER_ENTRY  8
#define THRESHOLD_CLAUSES        140
#define THRESHOLD_WEIGHTS        10

/* BRAM base macros — ensure these exist in your xparameters.h */
#define BRAM_BASE        XPAR_AXI_BRAM_0_BASEADDRESS
#define NUM_IMAGES       10000 * 2 
#define BRAM_BASE1       XPAR_AXI_BRAM_1_BASEADDRESS
#define BRAM_BASE2       XPAR_AXI_BRAM_2_BASEADDRESS
#define BRAM_BASE3       XPAR_AXI_BRAM_3_BASEADDRESS
#define BRAM_CTRL_REG    (BRAM_BASE + 0xFFFF0)

#ifndef XPAR_XIICPS_0_DEVICE_ID
#define XPAR_XIICPS_0_DEVICE_ID 0
#endif
#if LWIP_IPV6==1
#include "lwip/ip.h"
#else
#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif
#endif

/* defined by each RAW mode application */
void print_app_header();
int call_break_func();
int start_application();
int transfer_data();
void tcp_fasttmr(void);
void wait();
void tcp_slowtmr(void);
void print_mac();
/* missing declaration in lwIP */
void lwip_init();
extern int x;
int n_imgs = 10000;
int classes, clause,clauses, stride,patch_size,dataset,customized,classe;
extern int class_output[10000];
int labels[10000];
#if LWIP_IPV6==0
#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);

#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

#if LWIP_IPV6==1
void print_ip6(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf(" %x:%x:%x:%x:%x:%x:%x:%x\n\r",
		   IP6_ADDR_BLOCK1(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK2(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK3(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK4(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK5(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK6(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK7(&ip->u_addr.ip6),
		   IP6_ADDR_BLOCK8(&ip->u_addr.ip6));

}
#else
void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
		   ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}
#endif

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)
int IicPhyReset(void);
#endif
#endif

/* forward declarations */
// void print_app_header();
int start_application();
//int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);
void lwip_init();

/* prototypes for functions defined later or used before definition */
int DMA(void);

#if LWIP_IPV6==0
#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
extern struct netif server_netif;
extern struct netif *echo_netif;


XIicPs Iic;  
/* toggle overall reset using GPIO1 (adapt instance if different on your platform) */
void toggle_reset(void)
{
    /* configure tri as output, assert, deassert */
    Xil_Out32(XPAR_XGPIO_1_BASEADDR + GPIO_TRI_OFFSET, 0x0); // output
    Xil_Out32(XPAR_XGPIO_1_BASEADDR + GPIO_DATA_OFFSET, 0x1); // assert reset
    for (volatile int i = 0; i < 1000; i++);  // small delay
    Xil_Out32(XPAR_XGPIO_1_BASEADDR + GPIO_DATA_OFFSET, 0x0); // deassert reset and assert img_rst
}


/* Load clauses (up to 182 bits per line) into BRAM mapped at BRAM_BASE */

int load_file_to_bram_fmnistclauses_3(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_3_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_3_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_fmnistclauses_7(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_7_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_7_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_fmnistclauses_5(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_5_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_5_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_kmnistclauses_3(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_3_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_3_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_kmnistclauses_7(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_7_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_7_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_kmnistclauses_5(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_5_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_5_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_mnistclauses_3(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_3_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_3_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_mnistclauses_7(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_7_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_7_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}
int load_file_to_bram_mnistclauses_5(int BRAM_WIDTH_BITS) {
    u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_5_clauses_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_5_clauses[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }

    xil_printf("Data loaded into clauses BRAM successfully.\n\r");
    return 0;
}

int load_file_to_bram_fmnist_3_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_3_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_3_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_fmnist_5_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_5_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_5_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_fmnist_7_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_7_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_7_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}

int load_file_to_bram_kmnist_3_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_3_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_3_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_kmnist_5_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_5_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_5_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_kmnist_7_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_7_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_7_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}

int load_file_to_bram_mnist_3_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_3_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_3_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_mnist_5_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_5_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_5_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_mnist_7_weights0(int BRAM_WIDTH_BITS) {
     u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_7_weightszero_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE1 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_7_weightszero[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
    return 0;
}

int load_file_to_bram_fmnist_7_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_7_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_7_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_fmnist_5_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_5_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_5_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_fmnist_3_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_3_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_3_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}

int load_file_to_bram_kmnist_7_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_7_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_7_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_kmnist_5_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_5_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_5_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_kmnist_3_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_3_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_3_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}

int load_file_to_bram_mnist_7_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_7_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_7_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_mnist_5_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_5_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_5_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}
int load_file_to_bram_mnist_3_weights1(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_3_weightsone_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE2 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_3_weightsone[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 0 from internal array.\n\r");
 
     return 0;
}

int load_file_to_bram_mnist_3_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_3_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_3_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_mnist_5_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_5_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_5_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_mnist_7_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_mnist_7_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_mnist_7_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}

int load_file_to_bram_kmnist_3_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_3_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_3_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_kmnist_5_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_5_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_5_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_kmnist_7_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_kmnist_7_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_kmnist_7_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_fmnist_3_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_3_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_3_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_fmnist_5_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_5_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_5_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}
int load_file_to_bram_fmnist_7_weights2(int BRAM_WIDTH_BITS) {
u32 address_offsetcl = 0;

    // Each BRAM line = BRAM_WIDTH_BITS / 32 words
    int WORDS_PER_BRAM_LINE = BRAM_WIDTH_BITS / 32;
    int BYTES_PER_BRAM_LINE = BRAM_WIDTH_BITS / 8;

    xil_printf("Loading data into clauses BRAM (Width = %d bits)...\n\r", BRAM_WIDTH_BITS);

    // Loop through BRAM lines
    for (int line = 0; line < memory_data_fmnist_7_weightstwo_size / WORDS_PER_BRAM_LINE; line++) {
        xil_printf("Writing BRAM line %d at address 0x%08X\n\r",
                   line, BRAM_BASE3 + address_offsetcl);

        // Write each 32-bit word in the line
        for (int j = 0; j < WORDS_PER_BRAM_LINE; j++) {
            int idx = line * WORDS_PER_BRAM_LINE + j;
            Xil_Out32(BRAM_BASE + address_offsetcl + (j * 4), memory_data_fmnist_7_weightstwo[idx]);
        }

        // Move to next BRAM line
        address_offsetcl += BYTES_PER_BRAM_LINE;
    }
    xil_printf("Data loaded into weights BRAM 2 from internal array.\n\r");
    return 0;
}


#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)
int IicPhyReset(void);
#endif
#endif

int main() {
    #if LWIP_IPV6==0
	ip_addr_t ipaddr, netmask, gw;

#endif
	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

	/* Define this board specific macro in order perform PHY reset on ZCU102 */
#ifdef XPS_BOARD_ZCU102
	if (IicPhyReset()) {
		xil_printf("Error performing PHY reset \n\r");
		return -1;
	}
#endif
    //print_mac();

    init_platform();

        // Initialize I2C controller for INA226 sensors
    if (power_monitor_init(&Iic, XPAR_XIICPS_0_DEVICE_ID) != XST_SUCCESS) {
        xil_printf("Power monitor init failed!\r\n");
        // You can choose to continue without power-monitoring rather than exit
    } else {
        xil_printf("Power monitor initialized.\r\n");
    }


    #if LWIP_IPV6==0
#if LWIP_DHCP==1
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#else
	/* initialize IP addresses to be used */
	IP4_ADDR(&ipaddr,  192, 168,   1, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   1,  1);
#endif
#endif

        // Idle baseline
    xil_printf("===== Baseline (idle) power =====\r\n");
    float pl_idle = measure_pl_power(&Iic);
    float ps_idle = measure_ps_ddr_power(&Iic);
    xil_printf("PL Idle Power:  %.3f W  |  PS/DDR Idle Power: %.3f W\r\n",
               pl_idle, ps_idle);

	print_app_header();

    lwip_init();

    #if (LWIP_IPV6 == 0)
	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
		       &gw, mac_ethernet_address,
		       XPAR_XEMACPS_0_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
#else
	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, NULL, NULL, NULL, mac_ethernet_address,
		       XPAR_XEMACPS_0_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	echo_netif->ip6_autoconfig_enabled = 1;

	netif_create_ip6_linklocal_address(echo_netif, 1);
	netif_ip6_addr_set_state(echo_netif, 0, IP6_ADDR_VALID);

	print_ip6("\n\rBoard IPv6 address ", &echo_netif->ip6_addr[0].u_addr.ip6);

#endif
	netif_set_default(echo_netif);

#ifndef SDT
	/* now enable interrupts */
	platform_enable_interrupts();
#endif

	/* specify that the network if is up */
	netif_set_up(echo_netif);

#if (LWIP_IPV6 == 0)
#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(echo_netif);
	dhcp_timoutcntr = 240;

	while (((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0)) {
		xemacif_input(echo_netif);
	}

	if (dhcp_timoutcntr <= 0) {
		if ((echo_netif->ip_addr.addr) == 0) {
			xil_printf("DHCP Timeout\r\n");
			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(echo_netif->ip_addr),  192, 168,   1, 10);
			IP4_ADDR(&(echo_netif->netmask), 255, 255, 255,  0);
			IP4_ADDR(&(echo_netif->gw),      192, 168,   1,  1);
		}
	}

	ipaddr.addr = echo_netif->ip_addr.addr;
	gw.addr = echo_netif->gw.addr;
	netmask.addr = echo_netif->netmask.addr;
#endif

	print_ip_settings(&ipaddr, &netmask, &gw);

#endif

    xil_printf("=== BUILD CHECK: %s %s ===\r\n", __DATE__, __TIME__);
    printf("\n");
    printf("// ------------------ Xilinx ZCU102 FPGA Development Board--------------- // \n");
    printf("// ------------------------------ READY --------------------------------- // \n");
    printf("//////////////////////////////////////////////////////////////////////////// \n");
    printf("\n");

    // Initialize I2C controller for INA226 sensors
    // if (power_monitor_init(&Iic, XPAR_XIICPS_0_DEVICE_ID) != XST_SUCCESS) {
    //     xil_printf("I2C init failed!\r\n");
    //     return -1;
    // }
    /* Ensure these macros exist in your xparameters.h */
    unsigned int output_bases[NUM_OUTPUTS] = {
        XPAR_XGPIO_0_BASEADDR,   /* model params = {classes,clauses,stride,patch_size} */
        XPAR_XGPIO_1_BASEADDR    /* resets = {rst} */
    };
    unsigned int input_bases[NUM_INPUTS] = {
        XPAR_XGPIO_2_BASEADDR    /* {final_done,img_done,class_op} */
    };

    /* Configure directions: outputs -> drive, inputs -> read */
    for (int i = 0; i < NUM_OUTPUTS; i++) {
        Xil_Out32(output_bases[i] + GPIO_TRI_OFFSET, 0x0); /* outputs */
    }
    for (int i = 0; i < NUM_INPUTS; i++) {
        Xil_Out32(input_bases[i] + GPIO_TRI_OFFSET, 0xFFFFFFFF); /* inputs */
    }

    while (1) {
        char acceptance;
        u32 val = 0;
            xil_printf("Enter model parameters:\n\r");  
            xil_printf("Stride (0-7): ");
            scanf("%d", &stride);
            xil_printf("%d\n\r",stride);

            xil_printf("Patch size (3 or  5 or 7): ");
            scanf("%d", &patch_size);
            xil_printf("%d\n\r",patch_size);

            xil_printf("Dataset (1-4): \n\r 1.MNIST \n\r 2.FMNIST \n\r 3.KMNIST \n\r 4.Custom\n\r");            
            scanf("%d", &dataset);
            if(dataset == 1)xil_printf("MNIST Dataset selected\n\r");
            else if(dataset == 2)xil_printf("FMNIST Dataset selected\n\r");
            else if(dataset == 3)xil_printf("KMNIST Dataset selected\n\r");
            else if(dataset == 4)xil_printf("Custom Dataset selected, please change the clauses accordingly to the trained values\n\r");                                
            xil_printf("Classes (0-15): ");
            scanf("%d", &classe);
            xil_printf("%d\n\r",classe%100);
            xil_printf("Clauses (0-140): ");
            scanf("%d", &clause);
            xil_printf("%d\n\r",clause%1000);
        //labels
            clauses = clause % 1000;
            classes = classe % 100;
        for(int k = 0;k < n_imgs ;k++){
            if(k < n_imgs/10) labels[k] = 0;
            else if(k < n_imgs/5) labels[k] = 1;   
            else if(k < 3*n_imgs/10)labels[k] = 2; 
            else if(k < 2*n_imgs/5) labels[k] = 3;   
            else if(k < 5*n_imgs/10)labels[k] = 4; 
            else if(k < 3*n_imgs/5) labels[k] = 5;   
            else if(k < 7*n_imgs/10)labels[k] = 6; 
            else if(k < 4*n_imgs/5) labels[k] = 7;   
            else if(k < 9*n_imgs/10)labels[k] = 8; 
            else if(k < n_imgs) labels[k] = 9;   
        }
                 
            customized = (clauses != 140 || classes !=10 || stride != 1 || !(patch_size == 3 || patch_size == 5 || patch_size == 7));
            if(customized)printf("customized parameters, please be vary of files attached in code and train clauses accordingly using TMU library\n\r");
            xil_printf("continue?[y/n]\n\r");
            scanf("%c",&acceptance);
            if(acceptance == 'n')break;
            else {
            val = ((classes & 0xF) << 15) |
              ((clauses & 0x1FF) << 6) |
              ((stride & 0x7) << 3) |
              ((patch_size & 0x7));
        xil_printf("GPIO i/p : %d\n\r",val);
        Xil_Out32(XPAR_XGPIO_1_BASEADDR + GPIO_DATA_OFFSET, val);
        xil_printf("Toggling overall reset...\n\r");
        toggle_reset();

        xil_printf("Loading weights' BRAM...\n\r");
        if(patch_size == 3 && dataset == 1){
        load_file_to_bram_mnist_3_weights0(512);
        load_file_to_bram_mnist_3_weights1(512);
        load_file_to_bram_mnist_3_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_mnistclauses_3(256);
        }

        else if(patch_size == 5 && dataset == 1){
        load_file_to_bram_mnist_5_weights0(512);
        load_file_to_bram_mnist_5_weights1(512);
        load_file_to_bram_mnist_5_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_mnistclauses_5(256);
        }

        else if(patch_size == 7 && dataset == 1){
        load_file_to_bram_mnist_7_weights0(512);
        load_file_to_bram_mnist_7_weights1(512);
        load_file_to_bram_mnist_7_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_mnistclauses_7(256);
        }

        if(patch_size == 3 && dataset == 2){
        load_file_to_bram_fmnist_3_weights0(512);
        load_file_to_bram_fmnist_3_weights1(512);
        load_file_to_bram_fmnist_3_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_fmnistclauses_3(256);
        }

        else if(patch_size == 5 && dataset == 2){
        load_file_to_bram_fmnist_5_weights0(512);
        load_file_to_bram_fmnist_5_weights1(512);
        load_file_to_bram_fmnist_5_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_fmnistclauses_5(256);
        }

        else if(patch_size == 7 && dataset == 2){
        load_file_to_bram_fmnist_7_weights0(512);
        load_file_to_bram_fmnist_7_weights1(512);
        load_file_to_bram_fmnist_7_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_fmnistclauses_7(256);
        }

        if(patch_size == 3 && dataset == 3){
        load_file_to_bram_kmnist_3_weights0(512);
        load_file_to_bram_kmnist_3_weights1(512);
        load_file_to_bram_kmnist_3_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_kmnistclauses_3(256);
        }

        else if(patch_size == 5 && dataset == 3){
        load_file_to_bram_kmnist_5_weights0(512);
        load_file_to_bram_kmnist_5_weights1(512);
        load_file_to_bram_kmnist_5_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_kmnistclauses_5(256);
        }

        else if(patch_size == 7 && dataset == 3){
        load_file_to_bram_kmnist_7_weights0(512);
        load_file_to_bram_kmnist_7_weights1(512);
        load_file_to_bram_kmnist_7_weights2(256);

        xil_printf("Loading clauses' BRAM...\n\r");
        load_file_to_bram_kmnistclauses_7(256);
        }

        xil_printf("BRAM Loading done...\n\r");
        // Power measuring after loading BRAM 
        xil_printf("===== After BRAM load =====\r\n");
        float pl_after_bram = measure_pl_power(&Iic);
        float ps_after_bram = measure_ps_ddr_power(&Iic);
        xil_printf("PL after BRAM load: %.3f W  |  PS/DDR: %.3f W\r\n",
                pl_after_bram, ps_after_bram);


        xil_printf("Starting TCP (Port 6001)...\n\r");
        // Measure idle PL power before starting
        float pl_idle = measure_pl_power(&Iic);
        xil_printf("PL power before starting : %d", pl_idle);
        start_application(); // from the TCP+DMA receiver file

        xil_printf("Waiting for 10000 images...\n\r");
        while (1) {
        if(call_break_func())break;
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
	    }
        wait();
        int correct = 0,wrong = 0;
        float accuracy;
        xil_printf("All %d images processed via DMA and GPIO config.\n\r", n_imgs);
        for(int j = 0;j < NUM_IMAGES/2 ; j++){
                xil_printf("predicted class = %d, label = %d, for image : %d\n\r",class_output[j],labels[j],j);
                if(class_output[j] == labels[j])correct++;
                else wrong++;                                            
            }       
            accuracy = (100.0) * correct/(correct + wrong);
        printf("accuracy :%f",accuracy); 
        int reply;
        xil_printf("Multi-image inference complete.\n\r");  
        xil_printf("test for another dataset? [1/0]:");
        scanf("%d",&reply); 
        if(reply)continue;
        else break;
    }
    }

    cleanup_platform();
    return 0;
}


#define REG_CONFIG        0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE   0x02
#define REG_POWER         0x03
// Initialize I2C controller
int power_monitor_init(XIicPs *Iic, u16 device_id)
{
    XIicPs_Config *Config;
    int status;

    Config = XIicPs_LookupConfig(device_id);
    if (!Config) {
        xil_printf("IIC config not found!\r\n");
        return XST_FAILURE;
    }

    status = XIicPs_CfgInitialize(Iic, Config, Config->BaseAddress);
    if (status != XST_SUCCESS) {
        xil_printf("IIC init failed!\r\n");
        return XST_FAILURE;
    }

    XIicPs_SetSClk(Iic, 100000); // 100 kHz
    return XST_SUCCESS;
}

// Read 16-bit register from INA226
u16 read_ina226_register(XIicPs *Iic, u8 addr, u8 reg)
{
    u8 write_buf[1];
    u8 read_buf[2];
    int status;

    write_buf[0] = reg;

    status = XIicPs_MasterSendPolled(Iic, write_buf, 1, addr);
    if (status != XST_SUCCESS) {
        xil_printf("Failed to send reg 0x%x to 0x%02X\r\n", reg, addr);
        return 0xFFFF;
    }
    usleep(1000);

    status = XIicPs_MasterRecvPolled(Iic, read_buf, 2, addr);
    if (status != XST_SUCCESS) {
        xil_printf("Failed to read reg 0x%x from 0x%02X\r\n", reg, addr);
        return 0xFFFF;
    }

    return ((read_buf[0] << 8) | read_buf[1]);
}

// Convert raw power to Watts
float raw_to_watts(u16 raw_power)
{
    if (raw_power == 0xFFFF) return 0.0f;
    return (float)raw_power / 1000000.0f; // µW → W
}

// Measure total PL power (VCCINT + VCCBRAM + VCCAUX)
float measure_pl_power(XIicPs *Iic)
{
    float total = 0.0f;
    u16 raw;

    raw = read_ina226_register(Iic, 0x40, REG_POWER); // VCCINT
    total += raw_to_watts(raw);

    raw = read_ina226_register(Iic, 0x41, REG_POWER); // VCCBRAM
    total += raw_to_watts(raw);

    raw = read_ina226_register(Iic, 0x42, REG_POWER); // VCCAUX
    total += raw_to_watts(raw);

    return total;
}

// Measure total PS/DDR power (VCCPSINTFP + VCCPSDDR)
float measure_ps_ddr_power(XIicPs *Iic)
{
    float total = 0.0f;
    u16 raw;

    raw = read_ina226_register(Iic, 0x44, REG_POWER); // PS internal
    total += raw_to_watts(raw);

    raw = read_ina226_register(Iic, 0x45, REG_POWER); // DDR
    total += raw_to_watts(raw);

    return total;
}
