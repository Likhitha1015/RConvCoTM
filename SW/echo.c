#include <stdio.h>
#include <string.h>
#include "sleep.h"
#include <stdint.h>
#include <sys/types.h>
#include <xaxidma.h>
#include "lwip/err.h"
#include "lwip/tcp.h"
#if defined (__arm__) || defined (__aarch64__)
#include "xil_printf.h"
#endif
#include "xaxidma.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xaxidma_hw.h"

#define DMA_DEVICE_ID       XPAR_AXIDMA_0_DEVICE_ID
#define GPIO2_BASEADDR 0xA0060000
#define GPIO2_DATA_OFFSET 0x0
//#define DMA_BEAT_SIZE XPAR_AXI_DMA_BEAT_SIZE
#define GPIO2_TRI_OFFSET  0x4
#define XPAR_AXI_DMA_0_BASEADDR 0xa0010000
#define IMAGE_SIZE_BYTES    64   // Example image size
#define IMAGE_SIZE    32*32
#define image_size_byte 0
#define PL_BEAT_BYTES      64           // 512 bits per PL beat
#define PL_BEATS_PER_SEND  2
#define BURST_BYTES        (PL_BEAT_BYTES * PL_BEATS_PER_SEND) // 128 bytes
#define DMA_BUSY_TIMEOUT    10000000U
#define DDR_BEAT_SIZE XPAR_AXI_DMA_BEAT_SIZE
#define NUM_IMAGES    (10000 * 2) //this is number of image chunks, basically num of images * 2
#define DDR_BASE_ADDR  ((volatile uint8_t *)0x01000000)

extern int n_imgs;
XAxiDma AxiDma;
void wait();
int img = 0;
int x;
int class_output[10000];
volatile int image_count = 0;  
struct tcp_pcb *global_tpcb = NULL;  // Global TCP connection

int call_break_func(){
    if(img >= NUM_IMAGES - 1){
        x = 1;
    }
    else x = 0;
    wait();
    return x;
}
void reverse_beats(u8* buffer, int length, int beat_size) {
    int i, j;
    u8 temp;
    for (i = 0; i < length; i += beat_size) {
        for (j = 0; j < beat_size / 2; j++) {
            temp = buffer[i + j];
            buffer[i + j] = buffer[i + beat_size - 1 - j];
            buffer[i + beat_size - 1 - j] = temp;
        }
    }
}

static int wait_dma_idle() {
    uint32_t timeout = DMA_BUSY_TIMEOUT;
    while (XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE)) {
        if (--timeout == 0) {
            xil_printf("ERROR: DMA busy timeout\n\r");
            return XST_FAILURE;
        }
    }
    return XST_SUCCESS;
}
void wait(){
    for(int e = 0;e<1000;e++);
}

/* Transfer an image in 2 PL-beat bursts (128 bytes each) */
int dma_transfer(uint32_t image_size_bytes) {
    int status;
    for (img = 0; img < NUM_IMAGES; img++) {
        volatile uint8_t *ddr_base_addr = (volatile uint8_t *)(0x01000000 + img * DDR_BEAT_SIZE);
        xil_printf("\n\rStarting DMA transfer for image chunk %d, size %u bytes\n\r", img, image_size_bytes);

            uint32_t bytes_to_send = 64;
            uintptr_t cur_addr = (uintptr_t)(ddr_base_addr);
            Xil_DCacheFlushRange(cur_addr, bytes_to_send);
            status = XAxiDma_SimpleTransfer(&AxiDma, cur_addr, bytes_to_send, XAXIDMA_DMA_TO_DEVICE);
            if (status != XST_SUCCESS) {
                xil_printf("DMA transfer failed at image chunk %d\n\r", img);
                return XST_FAILURE;
            }        
            status = wait_dma_idle();
            if (status != XST_SUCCESS) {
                xil_printf("DMA busy timeout at image %d\n\r", img);
                return XST_FAILURE;
            }    

        xil_printf("Image chunk %d DMA transfer complete.\n\r", img);
        if(img%2 == 1)Xil_Out32(GPIO2_BASEADDR + GPIO2_TRI_OFFSET, 0x3F);
        class_output[img/2] = Xil_In32(GPIO2_BASEADDR + GPIO2_DATA_OFFSET);
    }

    xil_printf("\n\rAll %d images chunks transferred via DMA.\n\r", NUM_IMAGES);
    return XST_SUCCESS;
}


err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    static int receivedBytes = 0;
    static char *buffPntr = (char *)(DDR_BASE_ADDR); 
    if (!p) {  
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Prevent overflow
    if (image_count >= NUM_IMAGES) {
        xil_printf("Image limit exceeded! Ignoring further data.\n\r");
        pbuf_free(p);
        return ERR_OK;
    }    

    int remaining_space = IMAGE_SIZE_BYTES - receivedBytes;
    int bytes_to_copy = (p->len > remaining_space) ? remaining_space : p->len;

    memcpy((void *)buffPntr, (void *)(p->payload), bytes_to_copy);   
    receivedBytes += bytes_to_copy;
    buffPntr += bytes_to_copy;

    if (receivedBytes == IMAGE_SIZE_BYTES) {
        xil_printf("Image chunk %d received\n\r", image_count);
        image_count++;
        receivedBytes = 0;

        tcp_write(tpcb, "ACK", 3, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);

        if (image_count < NUM_IMAGES)
            buffPntr = (char *)(DDR_BASE_ADDR + image_count * IMAGE_SIZE_BYTES);
        else
            buffPntr =  (char *)DDR_BASE_ADDR;

        if (image_count == NUM_IMAGES) {
            xil_printf("All %d images chunks received.\n\r", NUM_IMAGES);
            dma_transfer(IMAGE_SIZE_BYTES);
        }
    }

    tcp_recved(tpcb, p->len);
    pbuf_free(p);
    return ERR_OK;
}    

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    static int connection = 1;
    global_tpcb = newpcb;  // Store the TCP connection globally
    tcp_recv(newpcb, recv_callback);
    tcp_arg(newpcb, (void *)(UINTPTR)connection);
    connection++;
    return ERR_OK;
}

void print_app_header() {
    xil_printf("\n\r-----lwIP TCP Server------\n\r");
    xil_printf("TCP packets sent to port 6001 will be processed.\n\r");
}

int start_application() {
    int status;
    
    XAxiDma_Config *AxiDmaConfig = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_BASEADDR);
    if (!AxiDmaConfig) {
        xil_printf("Error: DMA config not found\n\r");
        return -1;
    }

    status = XAxiDma_CfgInitialize(&AxiDma, AxiDmaConfig);
    if (status != XST_SUCCESS) {
        xil_printf("DMA initialization failed...\n\r");
        return -1;
    }
    xil_printf("DMA initialization success...\n\r");

    struct tcp_pcb *pcb;
    err_t err;
    unsigned port = 6001;

    pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        xil_printf("Error creating PCB. Out of Memory\n\r");
        return -2;
    }
    xil_printf("PCB assignment successful.\n\r");

    err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (err != ERR_OK) {
        xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
        tcp_close(pcb);
        return -3;
    }
    xil_printf("bound to port %d: err = %d\n\r", port, err);
    pcb = tcp_listen(pcb);
    if (!pcb) {
        xil_printf("Out of memory while tcp_listen\n\r");
        return -4;
    }

    tcp_accept(pcb, accept_callback);
    xil_printf("TCP image processing server started @ port %d\n\r", port);
    return 0;
}
