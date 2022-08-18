library ieee;
use ieee.std_logic_1164.all;
use work.bf_defines.ALL;
 
entity LFSR is
	GENERIC (
		Num_Bits : integer;					--Size of random number 
		number: integer						--Number of the LFSR component (for assign a unique value)
    );
	PORT (
		Clk    : in std_logic;
		Enable : in std_logic; 
		-- Parameters to set a new seed for LFSR
		i_Seed_DV   : in std_logic;
		i_Seed_Data : in std_logic_vector(indexs_bits-1 downto 0);
		
		o_LFSR_Data : out std_logic_vector(indexs_bits-1 downto 0); --Random number
		o_LFSR_Done : out std_logic									--If Done='1'-> repeat the cycle
    );
end entity LFSR;
 
architecture RTL of LFSR is
 
  signal r_LFSR : std_logic_vector(Num_Bits downto 1) := Seed_LFSR(number);  --Value of registers
  signal w_XNOR : std_logic;												 --XNOR gate Value   
begin
 
  -- Purpose: Load up LFSR with Seed if Data Valid (DV) pulse is detected.
  -- Othewise just run LFSR when enabled.
  p_LFSR : process (Clk) is
  begin
    if rising_edge(Clk) then
      if Enable = '1' then
        if i_Seed_DV = '1' then
          r_LFSR <= i_Seed_Data;
        else
          r_LFSR <= r_LFSR(r_LFSR'left-1 downto 1) & w_XNOR;
        end if;
      end if;
    end if;
  end process p_LFSR; 

  
  g_LFSR_3 : if Num_Bits = 3 generate
    w_XNOR <= r_LFSR(3) xnor r_LFSR(2);
  end generate g_LFSR_3;
 
  g_LFSR_4 : if Num_Bits = 4 generate
    w_XNOR <= r_LFSR(4) xnor r_LFSR(3);
  end generate g_LFSR_4;
 
  g_LFSR_5 : if Num_Bits = 5 generate
    w_XNOR <= r_LFSR(5) xnor r_LFSR(3);
  end generate g_LFSR_5;
 
  g_LFSR_6 : if Num_Bits = 6 generate
    w_XNOR <= r_LFSR(6) xnor r_LFSR(5);
  end generate g_LFSR_6;
 
  g_LFSR_7 : if Num_Bits = 7 generate
    w_XNOR <= r_LFSR(7) xnor r_LFSR(6);
  end generate g_LFSR_7;
 
  g_LFSR_8 : if Num_Bits = 8 generate
    w_XNOR <= r_LFSR(8) xnor r_LFSR(6) xnor r_LFSR(5) xnor r_LFSR(4);
  end generate g_LFSR_8;
 
  g_LFSR_9 : if Num_Bits = 9 generate
    w_XNOR <= r_LFSR(9) xnor r_LFSR(5);
  end generate g_LFSR_9;
 
  g_LFSR_10 : if Num_Bits = 10 generate
    w_XNOR <= r_LFSR(10) xnor r_LFSR(7);
  end generate g_LFSR_10;
 
  g_LFSR_11 : if Num_Bits = 11 generate
    w_XNOR <= r_LFSR(11) xnor r_LFSR(9);
  end generate g_LFSR_11;
 
  g_LFSR_12 : if Num_Bits = 12 generate
    w_XNOR <= r_LFSR(12) xnor r_LFSR(6) xnor r_LFSR(4) xnor r_LFSR(1);
  end generate g_LFSR_12;
 
  g_LFSR_13 : if Num_Bits = 13 generate
    w_XNOR <= r_LFSR(13) xnor r_LFSR(4) xnor r_LFSR(3) xnor r_LFSR(1);
  end generate g_LFSR_13;
 
  g_LFSR_14 : if Num_Bits = 14 generate
    w_XNOR <= r_LFSR(14) xnor r_LFSR(5) xnor r_LFSR(3) xnor r_LFSR(1);
  end generate g_LFSR_14;
 
  g_LFSR_15 : if Num_Bits = 15 generate
    w_XNOR <= r_LFSR(15) xnor r_LFSR(14);
  end generate g_LFSR_15;
 
  g_LFSR_16 : if Num_Bits = 16 generate
    w_XNOR <= r_LFSR(16) xnor r_LFSR(15) xnor r_LFSR(13) xnor r_LFSR(4);
  end generate g_LFSR_16;
 
  g_LFSR_17 : if Num_Bits = 17 generate
    w_XNOR <= r_LFSR(17) xnor r_LFSR(14);
  end generate g_LFSR_17;
 
  g_LFSR_18 : if Num_Bits = 18 generate
    w_XNOR <= r_LFSR(18) xnor r_LFSR(11);
  end generate g_LFSR_18;
 
  g_LFSR_19 : if Num_Bits = 19 generate
    w_XNOR <= r_LFSR(19) xnor r_LFSR(6) xnor r_LFSR(2) xnor r_LFSR(1);
  end generate g_LFSR_19;
 
  g_LFSR_20 : if Num_Bits = 20 generate
    w_XNOR <= r_LFSR(20) xnor r_LFSR(17);
  end generate g_LFSR_20;
 
  g_LFSR_21 : if Num_Bits = 21 generate
    w_XNOR <= r_LFSR(21) xnor r_LFSR(19);
  end generate g_LFSR_21;
 
  g_LFSR_22 : if Num_Bits = 22 generate
    w_XNOR <= r_LFSR(22) xnor r_LFSR(21);
  end generate g_LFSR_22;
 
  g_LFSR_23 : if Num_Bits = 23 generate
    w_XNOR <= r_LFSR(23) xnor r_LFSR(18);
  end generate g_LFSR_23;
 
  g_LFSR_24 : if Num_Bits = 24 generate
    w_XNOR <= r_LFSR(24) xnor r_LFSR(23) xnor r_LFSR(22) xnor r_LFSR(17);
  end generate g_LFSR_24;
 
  g_LFSR_25 : if Num_Bits = 25 generate
    w_XNOR <= r_LFSR(25) xnor r_LFSR(22);
  end generate g_LFSR_25;
 
  g_LFSR_26 : if Num_Bits = 26 generate
    w_XNOR <= r_LFSR(26) xnor r_LFSR(6) xnor r_LFSR(2) xnor r_LFSR(1);
  end generate g_LFSR_26;
 
  g_LFSR_27 : if Num_Bits = 27 generate
    w_XNOR <= r_LFSR(27) xnor r_LFSR(5) xnor r_LFSR(2) xnor r_LFSR(1);
  end generate g_LFSR_27;
 
  g_LFSR_28 : if Num_Bits = 28 generate
    w_XNOR <= r_LFSR(28) xnor r_LFSR(25);
  end generate g_LFSR_28;
 
  g_LFSR_29 : if Num_Bits = 29 generate
    w_XNOR <= r_LFSR(29) xnor r_LFSR(27);
  end generate g_LFSR_29;
 
  g_LFSR_30 : if Num_Bits = 30 generate
    w_XNOR <= r_LFSR(30) xnor r_LFSR(6) xnor r_LFSR(4) xnor r_LFSR(1);
  end generate g_LFSR_30;
 
  g_LFSR_31 : if Num_Bits = 31 generate
    w_XNOR <= r_LFSR(31) xnor r_LFSR(28);
  end generate g_LFSR_31;
 
  g_LFSR_32 : if Num_Bits = 32 generate
    w_XNOR <= r_LFSR(32) xnor r_LFSR(22) xnor r_LFSR(2) xnor r_LFSR(1);
  end generate g_LFSR_32;
   
   
  o_LFSR_Data <= r_LFSR(r_LFSR'left downto 1);
  o_LFSR_Done <= '1' when r_LFSR(r_LFSR'left downto 1) = i_Seed_Data else '0';
   
end architecture RTL;
