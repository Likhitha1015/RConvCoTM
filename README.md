# Reconfigurable Convolutional Coalesced Tsetlin Machine

This repository contains the FPGA implementation of a Reconfigurable Convolutional Coalesced Tsetlin Machine as presented in our paper. The system supports MNIST, FMNIST, KMNIST datasets, and custom dataset testing.

## Specifications

Maximum number of Clauses :	140

Number of Classes	: 10

Patch Sizes :	3 / 5 / 7

Stride : 1 - Patch Size  

Note: Weights/clauses are limited to stride = 1. For other strides, get the trained clauses from the TMU library once the stride training update is complete.

Required FPGA :ZYNQ ZCU102 FPGA

Ethernet Cable

## Getting Started
### Required Tools

Xilinx Vitis 2024.1 or later version
 (Platform + Application projects)

Xilinx Vivado 2024.1 or a later version is required for editing or reusing the code.

MATLAB (for preprocessing)

Python (for host code execution)

## Testing Preprocessed Datasets

Preprocessed datasets (10k images each) are included for:

MNIST

FMNIST

KMNIST

**Steps:**

Open the Vitis platform and application projects.

Build and run the projects.

Open the serial monitor at 115200 baud.

Run the Python host code when the serial monitor shows:

Waiting for 10k images

The Python script sends images into DDR memory for testing.

## Custom images Testing

Preprocess images using:

img_to_hex.m

image_padded.m

Send preprocessed images via python.py to the FPGA.

## Custom Clauses and Weights

Obtain clauses and weights from the TMU library.

Preprocess clauses using clause_formatting.m.

Send clauses to python.py to generate .h files.

Preprocess weights using weights_generator and send the resulting 3 files to python.py.

Include generated .h files in the Vitis workspace.

Implement custom BRAM and weights access code in main.c.

Reference implementations for other datasets are provided in main.c.

## Additional Resources

A video tutorial is attached demonstrating the complete testing and deployment process.

1	Build and run Vitis platform + application projects
2	Open serial monitor at 115200 baud
3	Run python.py to send dataset images
4	For custom dataset: preprocess images → send via Python → include .h files in workspace → write custom BRAM/weights code
5	Verify outputs on the serial monitor
