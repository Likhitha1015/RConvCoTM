`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09.07.2025 17:01:03
// Design Name: 
// Module Name: conv_arch
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

//////////////////////////////////////////////////////////////////////////////////


module conv_arch(
    clk,rst,stride,pe_en,class_no,patch_size,clause_no,clause_op,prev_clause_op,bram_addr_a,clause_act,clause_done,clauses,clause_write,wea,img_rst,
    processor_in1,processor_in2,processor_in3,processor_in4,processor_in5,processor_in6,processor_in7,processor_in8,
    p1y1,p1x1,p2y1,p3y1,p4y1,p5y1,p6y1,p7y1,p8y1,ipdone,opdone_reg,done_final,
    processor_out1,processor_out2,processor_out3,processor_out4,processor_out5,processor_out6,processor_out7,processor_out8,
    po1x,po1y,po2y,po3y,po4y,po5y,po6y,po7y,po8y
    );
    parameter  IMG_WIDTH = 32, IMG_HEIGHT = 32,CLAUSEN = 10,CLASSN = 5,
               CLAUSE_WIDTH = (35 + IMG_HEIGHT + IMG_WIDTH)*2;
    input clk,rst,img_rst,ipdone,done_final;
    input [$clog2(CLAUSEN):0] clause_no;
    input [$clog2(CLASSN)-1:0] class_no;
    input [2:0] stride;
    input [$clog2(CLAUSEN):0]clauses;
    wire conv_enable;
    input [7:0] pe_en;
    input [6:0] processor_in1,processor_in2,processor_in3,processor_in4,processor_in5,processor_in6,processor_in7,processor_in8;
    input wire [IMG_WIDTH - 1:0] p1x1;
    input wire [IMG_HEIGHT - 1:0] p1y1,p2y1,p3y1,p4y1,p5y1,p6y1,p7y1,p8y1;
    input [2:0] patch_size;
    input prev_clause_op;
    output reg clause_op;
    output reg [6:0] processor_out1,processor_out2,processor_out3,processor_out4,processor_out5,processor_out6,processor_out7,processor_out8;
    output reg [IMG_WIDTH - 1:0] po1x;
    output reg [IMG_HEIGHT - 1:0] po1y,po2y,po3y,po4y,po5y,po6y,po7y,po8y;
    wire [7:0] bclause_op;
    input clause_act;
    input [CLAUSE_WIDTH - 1:0]clause_write;
    output reg clause_done;
    output reg opdone_reg;
    input wea;//start the whole design once clauses are taken in
    input [$clog2(CLAUSEN)-1:0]bram_addr_a;
        reg rst_n;
        reg [IMG_HEIGHT - 1:0] nypos[0:7];
        reg [IMG_WIDTH - 1:0] nxpos;
        wire [$clog2(CLAUSEN)-1:0]bram_addr;
        reg [(CLAUSE_WIDTH)-1:0]clause;
        wire [(CLAUSE_WIDTH)-1:0] clause_in;
        reg Xmatch;
        reg Ymatch[7:0];
        reg [IMG_HEIGHT - 1:0] Ypos_mask,nYpos_mask;
        reg [IMG_WIDTH - 1:0] Xpos_mask,nXpos_mask;
        (* rom_style = "block" *) reg [48:0]patch_rule_1;
        (* rom_style = "block" *) reg [48:0]patch_neg_rule_1;
        reg [27:0] force_ones;
        integer jdx,i;
        initial opdone_reg = 0;
        always @(*) begin
         if(patch_size == 3)begin
         for (i = 0; i < 118; i = i + 1) begin
            clause[i] = clause_in[117 - i];
        end
        end
        else if(patch_size == 5)begin
         for (i = 0; i < 142; i = i + 1) begin
            clause[i] = clause_in[141 - i];
        end
        end
         else if(patch_size == 7)begin
         for (i = 0; i < 182; i = i + 1) begin
            clause[i] = clause_in[181 - i];
        end
        end
         else clause = 0;
        end
        
        always @* begin
        force_ones = 28'b0;
        for (jdx = 27; jdx >= 28 - patch_size; jdx = jdx - 1) begin
            force_ones[jdx] = 1'b1;   // set MSB `patch_size` bits to 1
        end
        end
    assign bram_addr = (class_no * clauses + clause_no);
        genvar idx;
        
    blk_mem_gen_0 bram_inst (
    .clka(clk),
    .ena(1'b1),              // always enabled
    .wea(wea),               // write when wea=1
    .addra(bram_addr_a),
    .dina(clause_write),

    .clkb(clk),
    .enb(1'b1),              // always enabled
    .addrb(bram_addr),
    .doutb(clause_in)
    );
    
            conv_enable_generation CE(
                .clk(clk),
                .rst(rst_n || (wea || rst) || img_rst),
                .stride(stride),
                .patch_size(patch_size),
                .conv_enable(conv_enable)
            );
    
always @(posedge clk)begin
    if(rst || !clause_act || wea || img_rst)begin
        Xmatch        <= 0;
        Ymatch[1]     <= 1'b0;
        Ymatch[2]     <= 1'b0;
        Ymatch[3]     <= 1'b0;
        Ymatch[4]     <= 1'b0;
        Ymatch[5]     <= 1'b0;
        Ymatch[6]     <= 1'b0;
        Ymatch[7]     <= 1'b0;
        Ymatch[0]     <= 1'b0;
        Xpos_mask     <= 0;
        Ypos_mask     <= 0;
        nYpos_mask    <= 0;
        nXpos_mask    <= 0;
        nypos[0]      <= 0;
        nypos[1]      <= 0;
        nypos[2]      <= 0;
        nypos[3]      <= 0;
        nypos[4]      <= 0;
        nypos[5]      <= 0;
        nypos[6]      <= 0;
        nypos[7]      <= 0;
        
        end
    else begin
     nypos[0] <= ~p1y1;
     nypos[1] <= ~p2y1;
     nypos[2] <= ~p3y1;
     nypos[3] <= ~p4y1;
     nypos[4] <= ~p5y1;
     nypos[5] <= ~p6y1;
     nypos[6] <= ~p7y1;
     nypos[7] <= ~p8y1;
     nxpos <= ~p1x1;
     
    patch_rule_1 = 49'd0;          // default full assignment
    patch_neg_rule_1 = 49'd0;
    
    if (patch_size == 3) begin
    patch_rule_1[2:0]   = clause[52:50];//conv unit and rules vector matching
    patch_rule_1[9:7]   = clause[55:53];
    patch_rule_1[16:14] = clause[58:56]; // neg patch and patch pixels extraction from clause

    patch_neg_rule_1[2:0]   = clause[111:109];
    patch_neg_rule_1[9:7]   = clause[114:112];
    patch_neg_rule_1[16:14] = clause[117:115];

//    patch_rule_1[2:0]   = clause[61:59];//conv unit and rules vector matching
//    patch_rule_1[9:7]   = clause[64:62];
//    patch_rule_1[16:14] = clause[67:65]; // neg patch and patch pixels extraction from clause

//    patch_neg_rule_1[2:0]   = clause[2:0];
//    patch_neg_rule_1[9:7]   = clause[5:3];
//    patch_neg_rule_1[16:14] = clause[8:6];
    end
    
    else if (patch_size == 5) begin
    patch_rule_1[4:0]    = clause[50:46];
    patch_rule_1[11:8]   = clause[55:51];
    patch_rule_1[18:14]  = clause[60:56];
    patch_rule_1[25:21]  = clause[65:61];
    patch_rule_1[32:28]  = clause[70:66];

    patch_neg_rule_1[4:0]    = clause[121:117];
    patch_neg_rule_1[11:7]   = clause[126:122];
    patch_neg_rule_1[18:14]  = clause[131:127];
    patch_neg_rule_1[25:21]  = clause[136:132];
    patch_neg_rule_1[32:28]  = clause[141:137];

//    patch_rule_1[4:0]    = clause[75:71];
//    patch_rule_1[11:8]   = clause[80:76];
//    patch_rule_1[18:14]  = clause[85:81];
//    patch_rule_1[25:21]  = clause[90:86];
//    patch_rule_1[32:28]  = clause[95:91];

//    patch_neg_rule_1[4:0]    = clause[4:0];
//    patch_neg_rule_1[11:7]   = clause[9:5];
//    patch_neg_rule_1[18:14]  = clause[14:10];
//    patch_neg_rule_1[25:21]  = clause[19:15];
//    patch_neg_rule_1[32:28]  = clause[24:20];
    end

    else begin
                patch_rule_1[48:0] = clause[90:42];
                patch_neg_rule_1[48:0] = clause[181:133];
                
//                 patch_rule_1[48:0] = clause[139:91];
//                patch_neg_rule_1[48:0] = clause[48:0];
    end
    
if (patch_size == 3'd3) begin
        Ypos_mask  <= clause[24:0];    //if 32 [28:0]    64 [60:0]    128 [124:0]   256 [252:0]
        Xpos_mask  <= clause[49:25];   //if 32 [57:29]   64 [121:61]  128 [249:125] 256 [505:253]
        nYpos_mask <= clause[83:59];   //if 32 [95:67]   64 [191:131] 128 [383:259] 256 [767:515]
        nXpos_mask <= clause[108:84];  //if 32 [124:96]  64 [252:192] 128 [508:384] 256 [1020:768]
//        nYpos_mask  <= clause[58:34];    //if 32 [26:0]    64 [58:0]    128 [122:0]   256 [250:0]
//        nXpos_mask  <= clause[33:9];   //if 32 [53:27]   64 [117:59]  128 [245:123] 256 [501:251]
//        Ypos_mask <= clause[117:93];  //if 32 [105:79]  64 [201:143] 128 [393:271] 256 [777:527]
//        Xpos_mask <= clause[92:68];
end else if (patch_size == 3'd5) begin
        Ypos_mask  <= clause[22:0];    //if 32 [26:0]    64 [58:0]    128 [122:0]   256 [250:0]
        Xpos_mask  <= clause[45:23];   //if 32 [53:27]   64 [117:59]  128 [245:123] 256 [501:251]
        nYpos_mask <= clause[93:71];  //if 32 [105:79]  64 [201:143] 128 [393:271] 256 [777:527]
        nXpos_mask <= clause[116:94];//if 32 [132:106] 64 [260:202] 128 [516:394] 256 [1028:778]
//        Ypos_mask  <= clause[141:119];    
//        Xpos_mask  <= clause[118:96];   //if 32 [53:27]   64 [117:59]  128 [245:123] 256 [501:251]
//        nYpos_mask <= clause[70:48];  //if 32 [105:79]  64 [201:143] 128 [393:271] 256 [777:527]
//        nXpos_mask <= clause[47:25]; 
end else if (patch_size == 3'd7) begin
        Ypos_mask <= clause[20:0];     //if 32 [24:0]    64 [56:0]    128 [120:0]   256 [248:0]
        Xpos_mask <= clause[41:21];    //if 32 [49:25]   64 [113:57]  128 [241:121] 256 [497:249]
        nYpos_mask <= clause[111:91];  //if 32 [123:99]  64 [219:163] 128 [411:291] 256 [795:547]
        nXpos_mask <= clause[132:112]; //if 32 [148:124] 64 [276:220] 128 [532:412] 256 [1044:796]
          
//        Ypos_mask <= clause[181:161];     //if 32 [24:0]    64 [56:0]    128 [120:0]   256 [248:0]
//        Xpos_mask <= clause[160:140];    //if 32 [49:25]   64 [113:57]  128 [241:121] 256 [497:249]
//        nYpos_mask <= clause[90:70];  //if 32 [123:99]  64 [219:163] 128 [411:291] 256 [795:547]
//        nXpos_mask <= clause[69:49]; 

end else begin
    Ypos_mask  <= 1'bx;
    Xpos_mask  <= 1'bx;
    nYpos_mask <= 1'bx;
    nXpos_mask <= 1'bx;
end

    Xmatch    <= &(p1x1 | ~Xpos_mask | force_ones) && (&(nxpos    | ~nXpos_mask | force_ones));
    Ymatch[0] <= &(p1y1 | ~Ypos_mask | force_ones) && (&(nypos[0] | ~nYpos_mask | force_ones));
    Ymatch[1] <= &(p2y1 | ~Ypos_mask | force_ones) && (&(nypos[1] | ~nYpos_mask | force_ones));
    Ymatch[2] <= &(p3y1 | ~Ypos_mask | force_ones) && (&(nypos[2] | ~nYpos_mask | force_ones));
    Ymatch[3] <= &(p4y1 | ~Ypos_mask | force_ones) && (&(nypos[3] | ~nYpos_mask | force_ones));
    Ymatch[4] <= &(p5y1 | ~Ypos_mask | force_ones) && (&(nypos[4] | ~nYpos_mask | force_ones));
    Ymatch[5] <= &(p6y1 | ~Ypos_mask | force_ones) && (&(nypos[5] | ~nYpos_mask | force_ones));
    Ymatch[6] <= &(p7y1 | ~Ypos_mask | force_ones) && (&(nypos[6] | ~nYpos_mask | force_ones));
    Ymatch[7] <= &(p8y1 | ~Ypos_mask | force_ones) && (&(nypos[7] | ~nYpos_mask | force_ones));
    
    end 
    if(!(wea || rst ||img_rst )) clause_done <= clause_act;
    else clause_done <= 1'b0;
    rst_n <= rst ||!(clause_act) || wea || img_rst;
    if(rst || wea || img_rst)clause_op <= 0;
    else
    clause_op <= (prev_clause_op | (|bclause_op) );
    processor_out1 <= processor_in1;
    processor_out2 <= processor_in2;
    processor_out3 <= processor_in3;
    processor_out4 <= processor_in4;
    processor_out5 <= processor_in5;
    processor_out6 <= processor_in6;
    processor_out7 <= processor_in7;
    processor_out8 <= processor_in8;
    po1x <= p1x1;
    po1y <= p1y1;
    po2y <= p2y1;
    po3y <= p3y1;
    po4y <= p4y1;
    po5y <= p5y1;
    po6y <= p6y1;
    po7y <= p7y1;
    po8y <= p8y1;
    end
    always@(posedge clk)begin
    if(rst_n)opdone_reg <= 0;
    else opdone_reg <= ipdone;
    end
    Convolution PE1 (clk, (wea || rst || img_rst), conv_enable, pe_en[0], processor_in1, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[0],bclause_op[0]);
    Convolution PE2 (clk, (wea || rst || img_rst), conv_enable, pe_en[1], processor_in2, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[1],bclause_op[1]);
    Convolution PE3 (clk, (wea || rst || img_rst), conv_enable, pe_en[2], processor_in3, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[2],bclause_op[2]);
    Convolution PE4 (clk, (wea || rst || img_rst), conv_enable, pe_en[3], processor_in4, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[3],bclause_op[3]);
    Convolution PE5 (clk, (wea || rst || img_rst), conv_enable, pe_en[4], processor_in5, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[4],bclause_op[4]);
    Convolution PE6 (clk, (wea || rst || img_rst), conv_enable, pe_en[5], processor_in6, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[5],bclause_op[5]);
    Convolution PE7 (clk, (wea || rst || img_rst), conv_enable, pe_en[6], processor_in7, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[6],bclause_op[6]);
    Convolution PE8 (clk, (wea || rst || img_rst), conv_enable, pe_en[7], processor_in8, patch_size, patch_rule_1,patch_neg_rule_1,Xmatch,Ymatch[7],bclause_op[7]);
    
endmodule