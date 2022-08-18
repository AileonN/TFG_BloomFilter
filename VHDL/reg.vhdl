-- Register of BITS capacity
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_arith.all;

entity reg is
    Generic(BITS: natural);
    Port ( 
	   Din : in  STD_LOGIC_VECTOR (BITS-1 downto 0) := (others => '0');
           clk : in  STD_LOGIC := '0';
	   reset : in  STD_LOGIC := '0';
           load : in  STD_LOGIC := '0';
           Dout : out  STD_LOGIC_VECTOR (BITS-1 downto 0) := (others => '0')
     );
end reg;

architecture Behavioral of reg is

begin
SYNC_PROC: process (clk)
   begin
      if (clk'event and clk = '1') then
         if (reset = '1') then
            Dout <= conv_std_logic_vector(0,BITS);
         else
            if (load='1') then 
		Dout <= Din;
	    end if;	
         end if;        
      end if;
   end process;

end Behavioral;
