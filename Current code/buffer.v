`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06/29/2024 05:44:40 PM
// Design Name: 
// Module Name: buffer
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

module buffer (clk, rst, pixel_in, shift_enable, done,
                img_width, pixel_out, residues, cycle_change,img_width_count);

parameter  BUF_HEIGHT = 8;
parameter  BUF_WIDTH = 34;
parameter  MAX_KERNEL_SIZE = 7;
input clk, rst;
input  done, shift_enable;
input [BUF_HEIGHT-1:0] pixel_in;

input [$clog2(BUF_WIDTH)-1:0] img_width;
output [BUF_HEIGHT-1:0]pixel_out;
output [BUF_HEIGHT-1:2] residues;
output reg cycle_change;

output reg [$clog2(BUF_WIDTH) :0] img_width_count;
//intializing buffer register
(* keep = "true" *) reg  buffer [BUF_HEIGHT-1:0][BUF_WIDTH-1:0];


integer row, col;

assign pixel_out = pixel_in;


always@(posedge clk or posedge rst)
begin
    if(rst)
        begin
                for(row = 0 ; row < BUF_HEIGHT ; row = row + 1)
                begin
                    for(col = 0 ; col < BUF_WIDTH ; col = col + 1)
                    begin
                        buffer [row] [col] <= 0;
                    end
                end
        end

    else
        begin
            if(shift_enable && !done)
            begin            
                for(col = 0 ; col < BUF_WIDTH ; col = col + 1)
                begin
                    if (col==0)
                        begin
                            for ( row = 0 ; row < BUF_HEIGHT ; row = row + 1)
                            begin
                                buffer [row] [col] <=  pixel_in[row];      
                            end
                        end
                    else
                        begin
                            for ( row = 0 ; row < BUF_HEIGHT ; row = row + 1)
                            begin
                                buffer [row] [col] <= buffer[row][col-1];    
                            end
                        end    
                end                          
            end
        end
end    

//control signal for multiplexer
integer m, n;
(* keep = "true" *) reg mux_sel [0:BUF_HEIGHT-1][0:BUF_WIDTH-1];
always @(img_width)
begin
    for (n=3; n < BUF_WIDTH-1; n=n+1)    
        for (m=2; m < BUF_HEIGHT; m=m+1)
        begin
            if (n == img_width)
                mux_sel[m][n] = 1'b1;
            else
                mux_sel[m][n] = 1'b0;    
        end    
end    

//connecting multiplexers
(* keep = "true" *) wire mux_out [0:BUF_HEIGHT-1][0:BUF_WIDTH-1];
genvar p, q;
generate
for (p=2; p<BUF_HEIGHT; p = p+1) begin
    for (q=3; q<BUF_WIDTH-1; q = q+1)
        begin
        assign mux_out [p] [q] = mux_sel[p][q] ? buffer[p][q] : mux_out[p][q+1] ; 
        assign residues[p] =  shift_enable ? mux_out[p][img_width-1]: 1'b0;   
        end
   
end     
endgenerate

always@(posedge clk or posedge rst)
begin
    if(rst) begin
        cycle_change <= 0;
        img_width_count <= 0;
    end
    else begin
        if(shift_enable) 
        begin
            img_width_count <= img_width_count + 1;
            cycle_change <=0 ;
            if(img_width_count == img_width-1) 
                begin
                    cycle_change <= 1;
                    img_width_count <= -1;                
                end
        end
    end


end

endmodule