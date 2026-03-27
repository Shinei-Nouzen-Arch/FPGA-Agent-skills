`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 01.11.2018 09:19:14
// Design Name: 
// Module Name: single_sdp_ram
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


module single_sdp_ram #(
     parameter C_DATA_WIDTH = 16,
     parameter C_ADDR_WIDTH = 10,
     parameter RAM_PERFORMANCE = "HIGH_PERFORMANCE") (
     input CLKA,
     input WEA,
     input [C_ADDR_WIDTH-1:0] ADDRA,
     input [C_DATA_WIDTH-1:0] DINA,
     input ENB,
     input RSTB,
     input [C_ADDR_WIDTH-1:0] ADDRB,
     output [C_DATA_WIDTH-1:0] DOUTB,
     input REGCEB,
     input CLKB
     );
    
  reg [C_DATA_WIDTH-1:0] ram_i [2**C_ADDR_WIDTH-1:0];
  reg [C_DATA_WIDTH-1:0] ram_data = {C_DATA_WIDTH{1'b0}};

  always @(posedge CLKA)
    if (WEA)
      ram_i[ADDRA] <= DINA;

  always @(posedge CLKB)
    if (ENB)
      ram_data <= ram_i[ADDRB];

  //  The following code generates HIGH_PERFORMANCE (use output register) or LOW_LATENCY (no output register)
  generate
    if (RAM_PERFORMANCE == "LOW_LATENCY") begin: no_output_register
      // The following is a 1 clock cycle read latency at the cost of a longer clock-to-out timing
       assign DOUTB = ram_data;

    end else begin: output_register
      // The following is a 2 clock cycle read latency with improve clock-to-out timing
      reg [C_DATA_WIDTH-1:0] doutb_reg = {C_DATA_WIDTH{1'b0}};
      always @(posedge CLKB)
        if (RSTB)
          doutb_reg <= {C_DATA_WIDTH{1'b0}};
        else if (REGCEB)
          doutb_reg <= ram_data;

      assign DOUTB = doutb_reg;

    end
  endgenerate
    
endmodule
