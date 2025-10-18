`timescale 1ns / 1ps

module Convolution (
    input clk,
    input rst,
    input conv_enable,
    input pe_enable,
    input [6:0] pixels,
    input [2:0] patch_size,
    input [48:0] rule,
    input [48:0] neg_rule,
    input Xmatch,
    input Ymatch,
    output reg clause_op
);
reg [6:0] shift_reg,shift_reg2;
reg delayed_conv_en;
reg [6:0] conv_unit[0:6];
reg [6:0] out_3, out_5, out_7;
reg [6:0] neg_out_3, neg_out_5, neg_out_7;
reg row_3, row_5, row_7;
reg neg_row_3, neg_row_5, neg_row_7;
reg out [0:48];
reg neg_out [0:48];
reg conv_en_seen;
integer i,j;
wire x_match_d2,x_match_d4,x_match_d6,y_match_d2,y_match_d4,y_match_d6;
always @(posedge clk)begin
delayed_conv_en <= conv_enable;
end
always @(posedge clk or posedge rst) begin
    if (rst) begin
        conv_en_seen <= 0;
        for (i = 0; i < 7; i = i + 1)
            for (j = 0; j < 7; j = j + 1)
                conv_unit[i][j] <= 7'b0;
    end else if (pe_enable) begin
        for ( i = 0 ; i < patch_size ; i = i + 1)
            begin
                conv_unit [i] [0] <=  pixels[i];
                for(j = 1 ; j < patch_size ; j = j + 1) 
                begin
                    conv_unit[ i ] [j] <= conv_unit[ i ][ j - 1 ];
                end
            end
            end
end

// Compare and reduce
always @(*) begin
    if(!rst && conv_enable)conv_en_seen <= 1;
    if(pe_enable && conv_enable)begin
    for (i = 0; i < 49; i = i + 1) begin
        out[i]     = conv_unit[i/7][i%7] ||(~ rule[i]);
        neg_out[i] = (~conv_unit[i/7][i%7]) ||(~neg_rule[i]);
    end
    for (i = 0; i < 7; i = i + 1) begin
        out_3[i]     = 0;
        neg_out_3[i] = 0;
        out_5[i]     = 0;
        neg_out_5[i] = 0;
        out_7[i]     = 0;
        neg_out_7[i] = 0;
    end

    for (i = 0; i < patch_size; i = i + 1) begin
        if (patch_size >= 3) begin
            out_3[i]     = out[i*7 + 0] & out[i*7 + 1] & out[i*7 + 2];
            neg_out_3[i] = neg_out[i*7 + 0] & neg_out[i*7 + 1] & neg_out[i*7 + 2];
        end

        if (patch_size >= 5) begin
            out_5[i]     = out_3[i] & out[i * 7 + 3] & out[i * 7 + 4];
            neg_out_5[i] = neg_out_3[i] & neg_out[i * 7 + 3] & neg_out[i * 7 + 4];
        end
        if (patch_size == 7) begin
            out_7[i]     = out_5[i] & out[i * 7 + 5] & out[i * 7 + 6];
            neg_out_7[i] = neg_out_5[i] & neg_out[i * 7 + 5] & neg_out[i * 7 + 6];
        end
    end

    row_3 = &out_3[2:0];
    row_5 = &out_5[4:0];
    row_7 = &out_7[6:0];

    neg_row_3 = &neg_out_3[2:0];
    neg_row_5 = &neg_out_5[4:0];
    neg_row_7 = &neg_out_7[6:0];
end
end

    always @(posedge clk) begin
            shift_reg <= {shift_reg[5:0], Xmatch};
            shift_reg2 <= {shift_reg2[5:0], Ymatch};
    end

    assign x_match_d2 = shift_reg[1];
    assign x_match_d4 = shift_reg[3];
    assign x_match_d6 = shift_reg[5];
    assign y_match_d2 = shift_reg2[1];
    assign y_match_d4 = shift_reg2[3];
    assign y_match_d6 = shift_reg2[5];
always @(posedge clk or posedge rst) begin
    if (rst) begin
        clause_op <= 0;
    end else begin
        if(pe_enable && conv_en_seen)
        case (patch_size)
            3: clause_op <= x_match_d2 & y_match_d2 & row_3 & neg_row_3;
            5: clause_op <= x_match_d4 & y_match_d4 & row_5 & neg_row_5;
            7: clause_op <= x_match_d6 & y_match_d6 & row_7 & neg_row_7;
            default: clause_op <= 0;
        endcase
        else clause_op <= 0;
    end
end

endmodule