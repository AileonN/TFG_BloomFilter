LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE ieee.math_real.ALL;
use IEEE.std_logic_arith.all;
use work.bf_defines.ALL;

-- Hash function H3 for addr_bits bits @ and num_cells 1 bit entries of filter
ENTITY H3 IS

Port(	
	DoHash: in std_logic :='0'; --DoHash=1 -> Dout = Index
	Seeds_Values: in indexs(addr_bits-1 downto 0); --Seed values
	Din:  in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@'s of addr_bits bits
	Dout: out std_logic_vector(indexs_bits-1 downto 0):= (others => '0') --@ for index bloom filter
); 
END H3;

ARCHITECTURE behavior OF H3 IS
signal f_xor: indexs(addr_bits-1 downto 0) := (others => (others => '0'));
BEGIN
	--Hashing(H3), return the index in Dout
	f_xor(0) <= (0 to indexs_bits-1 => Din(0)) and Seeds_Values(0);
	g_GENERATE_FOR: for i in 1 to addr_bits-1 generate
		f_xor(i) <= (((0 to indexs_bits-1 => Din(i)) and Seeds_Values(i))) xor f_xor(i-1);
    	end generate g_GENERATE_FOR;
	Dout<= f_xor(addr_bits-1) when dohash='1' else conv_std_logic_vector(0,indexs_bits);		
END;

