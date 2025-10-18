`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11.07.2025 18:30:22
// Design Name: 
// Module Name: class_top
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

module class_top#(
    parameter CLAUSEN = 140,
    CLASSN = 10,
    HEIGHT = 28,
    WIDTH = 28,
    CLAUSE_WIDTH = (35 + HEIGHT + WIDTH)*2
)(  
    input clk,
    input [1:0]prst,
    input [18:0] model_params,
    input [511:0]total_img,
    input tvalid,
    input [$clog2(CLAUSEN)-1:0]bram_addr_a,
    input [$clog2(CLASSN)-1:0]bram_addr_a2,
    input [CLAUSE_WIDTH - 1:0]clause_write,
    input [9*CLAUSEN - 1:0]weight_write,
    output reg tready,
    output reg [4:0] output_params
);
    wire img_rst,rst;
    wire [2:0]stride;
    reg [3:0] class_op;
    wire [3:0] class_op_wire;
    wire wea,wea2;
    assign img_rst = prst[1];
    assign rst = prst[0];
    reg [((HEIGHT + 8)*WIDTH)-1:0] total_memory;
    (* keep = "true" *)wire [8:0] clause = model_params[14:6];
    (* keep = "true" *) wire img_done_wire;
    reg img_done;
    integer i = 0,j,k,l,x;
    wire done_rmu,done;
    genvar idx;
    wire clause_act; 
    reg [5:0] cycle_count;
    (* keep = "true" *) reg [3:0]class_no = 0;
    reg shift_enable;
    wire [7:0] pixel_out;
    wire [3:0]classes = model_params[18:15];
    reg [7:0] pixel_in;
    wire [7:2] residues_buf;
    (* keep = "true" *)wire [7:2] residues_rmu;
    wire [$clog2(WIDTH + 2) :0] img_width_count;
    wire [7:0] pe_en;   
    wire reset;
    wire [2:0] patch_size;
    assign patch_size = model_params[2:0];
    assign stride = model_params[5:3];
    assign wea = (!rst && (bram_addr_a < clause)) ? 1'b1 : 1'b0;
    assign wea2 = (!rst && (bram_addr_a2 < classes)) ? 1'b1 : 1'b0;
    reg img_load_done;
    assign reset = wea || rst || !img_load_done || wea2;
    wire[6:0] processor_in1,processor_in2,processor_in3,processor_in4,processor_in5,processor_in6,processor_in7,processor_in8;
    wire [WIDTH - 1:0] p1x1;
    wire [HEIGHT - 1:0] p1y1,p2y1,p3y1,p4y1,p5y1,p6y1,p7y1,p8y1;
    genvar b; 
    wire cycle_change;
    
     buffer #(.BUF_WIDTH(WIDTH+2)) Buf(
        .clk(clk),
        .rst(reset),
        .pixel_in(pixel_in),
        .shift_enable(shift_enable),
        .done(1'b0),
        .img_width(WIDTH),
        .pixel_out(pixel_out),
        .residues(residues_buf),
        .cycle_change(cycle_change),
        .img_width_count(img_width_count)
    );
        
    generate
    for (b = 2; b < 8; b = b + 1) begin: reverse_loop 
    assign residues_rmu [b] = residues_buf[9 - b];
    end 
    endgenerate
    
    remapunit #(
                .IMG_WIDTH(WIDTH), 
                .IMG_HEIGHT(HEIGHT) 
                ) R (
        .clk(clk),
        .rst(reset),
        .patch_size(patch_size),
        .stride(stride),
        .done(done_rmu),
        .img_width(WIDTH),
        .img_height(HEIGHT),
        .xcor1(img_width_count),
        .pixel_in(pixel_out),
        .conv_arch_done(done),
        .residues(residues_rmu),
        .cycle_counts(cycle_count),
        .cycle_detect(cycle_change),
        .processor_in1(processor_in1), .processor_in2(processor_in2),
        .processor_in3(processor_in3), .processor_in4(processor_in4),
        .processor_in5(processor_in5), .processor_in6(processor_in6),
        .processor_in7(processor_in7), .processor_in8(processor_in8),
        .p_en(pe_en),
        .p1y1(p1y1), .p1x1(p1x1), .p2y1(p2y1),.p3y1(p3y1), .p4y1(p4y1), 
        .p5y1(p5y1), .p6y1(p6y1),.p7y1(p7y1), .p8y1(p8y1),
        .clause_act(clause_act)
    );

    // 1. Registers that need async reset (like tready, x, counters)
always @(posedge clk or posedge rst) begin
    if (rst) begin
        tready       <= 0;
        output_params <= 0;
        x            <= 0;
        img_load_done <= 0;
        cycle_count  <= 1;
        k            <= 0;
        j            <= 0;
        shift_enable <= 0;
        pixel_in     <= 0;
    end else begin
        if (img_rst) begin
            shift_enable <= 0;
            pixel_in <= 0;
            j <= 0;
            k <= 0;
            cycle_count  <= 1;
            x            <= 0;
            img_load_done <= 0;
            img_done <= 0;
            output_params = 5'b0;
        end 
        else begin
        class_op <= class_op_wire;
        tready <= tvalid && !img_load_done;
        img_done <= img_done_wire;
        output_params = {img_done,class_op[3:0]};
        if (!img_rst && tready) begin
            for (i = 0; i < 512; i = i + 1) begin
                total_memory[x*512 + i] <= total_img[i];
            end
            x <= x + 1;
        end

        if (x == WIDTH*HEIGHT / 512)
            img_load_done <= 1;
        if (!reset) begin
            shift_enable <= 1;
            pixel_in <= {
                total_memory[((j+7)*WIDTH)+k],
                total_memory[((j+6)*WIDTH)+k],
                total_memory[((j+5)*WIDTH)+k],
                total_memory[((j+4)*WIDTH)+k],
                total_memory[((j+3)*WIDTH)+k],
                total_memory[((j+2)*WIDTH)+k],
                total_memory[((j+1)*WIDTH)+k],
                total_memory[(j*WIDTH)+k]
            };
            k <= k + 1;
        end
        if (cycle_change && !reset) begin
            j <= j + 8;
            k <= 0;
            cycle_count <= cycle_count + 1;
        end
    end
end
end


            top #(
                .WIDTH(WIDTH), 
                .HEIGHT(HEIGHT), 
                .CLAUSEN(CLAUSEN),
                .CLASSN(CLASSN),
                .CLAUSE_WIDTH(CLAUSE_WIDTH)
                ) T (
            .clk(clk),
            .rst(rst),
            .img_rst(img_rst),
            .patch_size(patch_size),
            .stride(stride),
            .wea(wea),
            .bram_addr_a(bram_addr_a),
            .clause_write(clause_write),
            .pe_en(pe_en),
            .clauses(clause),
            .classes(classes),
            .weight_write(weight_write),
            .wea2(wea2),
            .bram_addr_a2(bram_addr_a2),
            .clause_act(clause_act),
            .processor_in1(processor_in1),
            .processor_in2(processor_in2),
            .processor_in3(processor_in3),
            .processor_in4(processor_in4),
            .processor_in5(processor_in5),
            .processor_in6(processor_in6),
            .processor_in7(processor_in7),
            .processor_in8(processor_in8),
            .p1y1(p1y1),
            .p1x1(p1x1),
            .p2y1(p2y1),
            .p3y1(p3y1),
            .p4y1(p4y1),
            .p5y1(p5y1),
            .p6y1(p6y1),
            .p7y1(p7y1),
            .p8y1(p8y1),
            .done(img_done_wire),
            .class_op(class_op_wire),
            .done_rmu(done_rmu)
            );

endmodule