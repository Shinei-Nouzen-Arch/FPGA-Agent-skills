// ROMs Using LUT Resources.
//
module sp_rom (clk, en, addr, dout);
input clk;
input en;
input [5:0] addr;
output  [83:0] dout;

reg [83:0] data;

always_comb 
begin 
  data <= 84'h11223344;
  if (en)
    case(addr)
      6'b000000: data <= 84'hFF200A; 6'b100000: data <= 84'hFF2222;
      6'b000001: data <= 84'hFF0300; 6'b100001: data <= 84'hFF4001;
      6'b000010: data <= 84'hFF8101; 6'b100010: data <= 84'hFF0342;
      6'b000011: data <= 84'hFF4000; 6'b100011: data <= 84'hFF232B;
      6'b000100: data <= 84'hFF8601; 6'b100100: data <= 84'hFF0900;
      6'b000101: data <= 84'hFF233A; 6'b100101: data <= 84'hFF0302;
      6'b000110: data <= 84'hFF0300; 6'b100110: data <= 84'hFF0102;
      6'b000111: data <= 84'hFF8602; 6'b100111: data <= 84'hFF4002;
      6'b001000: data <= 84'hFF2310; 6'b101000: data <= 84'hFF0900;
      6'b001001: data <= 84'hFF203B; 6'b101001: data <= 84'hFF8201;
      6'b001010: data <= 84'hFF8300; 6'b101010: data <= 84'hFF2023;
      6'b001011: data <= 84'hFF4002; 6'b101011: data <= 84'hFF0303;
      6'b001100: data <= 84'hFF8201; 6'b101100: data <= 84'hFF2433;
      6'b001101: data <= 84'hFF0500; 6'b101101: data <= 84'hFF0301;
      6'b001110: data <= 84'hFF4001; 6'b101110: data <= 84'hFF4004;
      6'b001111: data <= 84'hFF2500; 6'b101111: data <= 84'hFF0301;
      6'b010000: data <= 84'hFF0340; 6'b110000: data <= 84'hFF0102;
      6'b010001: data <= 84'hFF0241; 6'b110001: data <= 84'hFF2137;
      6'b010010: data <= 84'hFF4002; 6'b110010: data <= 84'hFF2036;
      6'b010011: data <= 84'hFF8300; 6'b110011: data <= 84'hFF0301;
      6'b010100: data <= 84'hFF8201; 6'b110100: data <= 84'hFF0102;
      6'b010101: data <= 84'hFF0500; 6'b110101: data <= 84'hFF2237;
      6'b010110: data <= 84'hFF8101; 6'b110110: data <= 84'hFF4004;
      6'b010111: data <= 84'hFF0602; 6'b110111: data <= 84'hFF0304;
      6'b011000: data <= 84'hFF4003; 6'b111000: data <= 84'hFF4040;
      6'b011001: data <= 84'hFF241E; 6'b111001: data <= 84'hFF2500;
      6'b011010: data <= 84'hFF0301; 6'b111010: data <= 84'hFF2500;
      6'b011011: data <= 84'hFF0102; 6'b111011: data <= 84'hFF2500;
      6'b011100: data <= 84'hFF2122; 6'b111100: data <= 84'hFF030D;
      6'b011101: data <= 84'hFF2021; 6'b111101: data <= 84'hFF2341;
      6'b011110: data <= 84'hFF0301; 6'b111110: data <= 84'hFF8201;
      6'b011111: data <= 84'hFF0102; 6'b111111: data <= 84'hFF400D;
      default: data <= 84'hFF00111;
    endcase
end 

assign dout = data;

endmodule

