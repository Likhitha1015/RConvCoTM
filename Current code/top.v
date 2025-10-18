`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Module Name: top
// Description: Top module connecting buffer, remapunit, convolution units, etc.
//////////////////////////////////////////////////////////////////////////////////

module top#(
    parameter WIDTH = 32, 
              HEIGHT = 32, 				
              CLAUSEN = 10,
              CLASSN = 10,
              CLAUSE_WIDTH = (35 + HEIGHT + WIDTH)*2
)
(
    input clk,
    input rst,img_rst,
    input [2:0] patch_size,
    input [2:0] stride,
    input [8:0]clauses,
    input [$clog2(CLASSN)-1:0]classes,
    input [CLAUSE_WIDTH - 1:0]clause_write,
    input [7:0] pe_en,
    input [$clog2(CLASSN)-1:0]bram_addr_a2,
    input [(9*CLAUSEN) - 1:0]weight_write,
    input done_rmu,
    input [6:0] processor_in1,processor_in2,processor_in3,processor_in4,processor_in5,processor_in6,processor_in7,processor_in8,
    input [HEIGHT - 1:0]p1y1,p2y1,p3y1,p4y1,p5y1,p6y1,p7y1,p8y1,
    input [WIDTH - 1:0] p1x1,
    input wea,wea2,
    input [$clog2(CLAUSEN)-1:0]bram_addr_a,
    input clause_act,
    output reg [$clog2(CLASSN)-1:0] class_op,
    output reg done
);
    wire [CLAUSEN-1:0] clause_output;
    wire [CLAUSEN - 1:0] clause_op;
    reg signed [$clog2(128*CLAUSEN):0] temp_sum[CLASSN-1:0]; 
    reg [$clog2(CLASSN)-1:0]class_no = 0;
    wire reset,done_conv;
    reg done_seen;
    assign reset = wea || rst || wea2 || img_rst;
    wire [0:CLAUSEN-1] done_conv_arch;
    wire [6:0] processor_out [1: CLAUSEN][0:7];  // stage, processor
    wire [WIDTH-1:0] po_x1 [1:CLAUSEN];
    wire [HEIGHT-1:0] po_y1 [1:CLAUSEN];
    wire [HEIGHT-1:0] po_y2 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y3 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y4 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y5 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y6 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y7 [1:CLAUSEN];           
    wire [HEIGHT-1:0] po_y8 [1:CLAUSEN];        
                    // stage, processor
    wire clause_done [1:CLAUSEN];
    wire [8:0] weight[CLASSN - 1:0];
    wire [$clog2(CLASSN)-1:0]bram_addr2;
    integer j,jdx,kdx;
    reg [$clog2(CLAUSEN):0]clause_no;
    reg ip_done_reg;

    reg signed [17:0] max_sum = -10000;


    always @(posedge clk)begin
        if(reset)begin
        for(jdx = 0;jdx < CLASSN;jdx = jdx+1)begin
            temp_sum[jdx] <= 0;
        end
        end
        else if(ip_done_reg)begin    
            for(jdx = 0;jdx < CLASSN;jdx = jdx+1)begin
            if(done_conv_arch[clause_no] && clause_op[clause_no])temp_sum[jdx] <= temp_sum[jdx] + $signed(weight[jdx]);
            else temp_sum[jdx] <= temp_sum[jdx];
        end
        end
    end
    always @(*)begin
    if(reset)begin
    class_op = 0;
    max_sum = -1000;
    end
    else if(done_conv)begin
        for(kdx = 0;kdx < CLASSN; kdx = kdx + 1)begin
            if(temp_sum[kdx] > max_sum)begin
            max_sum = temp_sum[kdx];
            class_op = kdx;
        end
        end
        end
        done = done_conv;
    end
    
    always @(posedge clk)begin
    if(reset)begin
        clause_no = 1;
        ip_done_reg = 0;
    end
    if(done_rmu)ip_done_reg = 1;
    else if(ip_done_reg)clause_no = clause_no + 1;
    end
    
    
assign done_conv = done_conv_arch[clauses-1];    
    genvar id,idx;
    
    assign clause_output = rst ? 0 :clause_op;
    generate
    for (idx = 0; idx < CLASSN; idx = idx + 1) begin : wt_chain_pos
        weight_adder #(.CLAUSEN(CLAUSEN),
                .CLASSN(CLASSN))W(
                .clk(clk),
                .rst(rst),
                .wea2(wea2),
                .bram_addr_a2(bram_addr_a2),
                .bram_addr_2(idx),
                .weight_write(weight_write),
                .clauses(clauses),
                .clause_no(clause_no),
                .weight(weight[idx])
                );
                
    end
    endgenerate
    
    
    
generate
    for (id = 0; id < CLAUSEN; id = id + 1) begin : conv_chain_pos
    if(id == 0 || id==1)begin
                conv_arch  #(
                .IMG_WIDTH(WIDTH), 
                .IMG_HEIGHT(HEIGHT), 
                .CLAUSEN(CLAUSEN),
                .CLASSN(CLASSN),
                .CLAUSE_WIDTH(CLAUSE_WIDTH)
                ) C (
            .clk(clk),
            .rst(rst),
            .img_rst(img_rst),
            .clause_no(id),
            .ipdone(done_rmu),
            .opdone_reg(done_conv_arch[id]),
            .stride(stride),
            .pe_en(pe_en),
            .class_no(class_no),
            .done_final(done_conv),
            .wea(wea),
            .patch_size(patch_size),
            .bram_addr_a(bram_addr_a),
            .clause_op(clause_op[id]),
            .clause_act(clause_act),
            .clauses(clauses),
            .clause_write(clause_write),
            .prev_clause_op(clause_output[id]),
            .clause_done(clause_done[id+1]),
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

            .processor_out1(processor_out[id+1][0]),
            .processor_out2(processor_out[id+1][1]),
            .processor_out3(processor_out[id+1][2]),
            .processor_out4(processor_out[id+1][3]),
            .processor_out5(processor_out[id+1][4]),
            .processor_out6(processor_out[id+1][5]),
            .processor_out7(processor_out[id+1][6]),
            .processor_out8(processor_out[id+1][7]),

            .po1x(po_x1[id+1]),
            .po1y(po_y1[id+1]),
            .po2y(po_y2[id+1]),
            .po3y(po_y3[id+1]),
            .po4y(po_y4[id+1]),
            .po5y(po_y5[id+1]),
            .po6y(po_y6[id+1]),
            .po7y(po_y7[id+1]),
            .po8y(po_y8[id+1])
        );
    end
        else begin
        conv_arch #(
                .IMG_WIDTH(WIDTH), 
                .IMG_HEIGHT(HEIGHT), 
                .CLAUSEN(CLAUSEN),
                .CLASSN(CLASSN),
                .CLAUSE_WIDTH(CLAUSE_WIDTH)
                ) C (
            .clk(clk),
            .rst(rst || !(id < clauses)),
            .img_rst(img_rst),
            .wea(wea),
            .stride(stride),
            .pe_en(pe_en),
            .clause_no(id),
            .ipdone(done_conv_arch[id-1]),
            .opdone_reg(done_conv_arch[id]),
            .done_final(done_conv),
            .bram_addr_a(bram_addr_a),
            .class_no(class_no),
            .patch_size(patch_size),
            .clauses(clauses),
//            .clause_result(clause_result[id]),
            .clause_write(clause_write),
            .clause_op(clause_op[id]),
            .clause_act(clause_done[id]),
            .clause_done(clause_done[id+1]),
            .prev_clause_op(clause_output[id]),
            // Processor inputs: use stage i signals
            .processor_in1(processor_out[id][0]),
            .processor_in2(processor_out[id][1]),
            .processor_in3(processor_out[id][2]),
            .processor_in4(processor_out[id][3]),
            .processor_in5(processor_out[id][4]),
            .processor_in6(processor_out[id][5]),
            .processor_in7(processor_out[id][6]),
            .processor_in8(processor_out[id][7]),

            .p1y1(po_y1[id]),
            .p1x1(po_x1[id]),
            .p2y1(po_y2[id]),
            .p3y1(po_y3[id]),
            .p4y1(po_y4[id]),
            .p5y1(po_y5[id]),
            .p6y1(po_y6[id]),
            .p7y1(po_y7[id]),
            .p8y1(po_y8[id]),

            .processor_out1(processor_out[id+1][0]),
            .processor_out2(processor_out[id+1][1]),
            .processor_out3(processor_out[id+1][2]),
            .processor_out4(processor_out[id+1][3]),
            .processor_out5(processor_out[id+1][4]),
            .processor_out6(processor_out[id+1][5]),
            .processor_out7(processor_out[id+1][6]),
            .processor_out8(processor_out[id+1][7]),

            .po1x(po_x1[id+1]),
            .po1y(po_y1[id+1]),
            .po2y(po_y2[id+1]),
            .po3y(po_y3[id+1]),
            .po4y(po_y4[id+1]),
            .po5y(po_y5[id+1]),
            .po6y(po_y6[id+1]),
            .po7y(po_y7[id+1]),
            .po8y(po_y8[id+1])
        );
        end
    end
endgenerate
endmodule