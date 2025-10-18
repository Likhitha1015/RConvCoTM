`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06.10.2025 15:27:29
// Design Name: 
// Module Name: weight_adder
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module weight_adder#(
    parameter CLAUSEN = 10,
              CLASSN = 10
)(  input clk,
    input rst,
    input wea2,
    input [$clog2(CLASSN)-1:0]bram_addr_a2,
    input [$clog2(CLASSN)-1:0]bram_addr_2,
    input [(9*CLAUSEN) - 1:0]weight_write,
    input [$clog2(CLAUSEN)-1:0]clauses,
    input [$clog2(CLAUSEN)-1:0]clause_no,
    output reg [8:0]weight
    );
    wire [(9*CLAUSEN) - 1:0] dout; 
    reg [8:0] w;
    reg [8:0] tcomp;
    reg signed [17:0] ext_w;
    reg signed [17:0] max_sum = -10000;
    blk_mem_gen_1 weights_inp (
        .clka(clk),            
        .ena(1'b1),               
        .wea(wea2),            
        .addra(bram_addr_a2),   
        .dina(weight_write),
                   
        .clkb(clk),           
        .enb(1'b1),               
        .addrb(bram_addr_2),   
        .doutb(dout)        
    );
    always @(posedge clk)begin
        w = dout[(clauses - clause_no - 1)*9 +: 9];
            if (w[8]) begin
                tcomp = ~w + 1'b1;
                weight = -tcomp;
            end
            else begin
                weight = w;
            end
    end
    
endmodule
