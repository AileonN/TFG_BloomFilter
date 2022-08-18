
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use IEEE.std_logic_arith.all;
use work.bf_defines.ALL;

-- Bloom filter of NUM_CELLS entries of 1 bit and hash_number*2 hashes (Half for even @ and the other half for odd)
entity BF_SR is 
	Generic(NUM_CELLS: natural); --Bloom filter cells and max 1's in filter
	Port (
			Clk : in std_logic := '0';
			BAddress_din: in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@ of addr_bits bits
			WE : in std_logic := '0';			   	     		 --Write enable	
			RE : in std_logic := '0';		   		     		 --Read enable		  
			Enable: in std_logic := '0';		   	             --If enable='1' its possible to write and read
			s_odd: in seeds(hash_number-1 downto 0);			 --Seeds for odd @ hashes
			s_even: in seeds(hash_number-1 downto 0);			 --Seeds for even @ hashes
			Dout : out std_logic	           		    	     --Output -> 1 its on BF, otherwise 0
	     );
end BF_SR;

architecture Behavioral of BF_SR is

	-- Component H3 declaration (hash used for index the filter)
	COMPONENT H3 is
		Port(
			DoHash: in std_logic :='0'; --DoHash=1 -> Dout = Index
			Seeds_Values: in indexs(addr_bits-1 downto 0); --Seed values
			Din:  in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@'s of addr_bits bits
			Dout: out std_logic_vector(indexs_bits-1 downto 0):= (others => '0') --@ for index bloom filter
		);
	END COMPONENT;
	
-- BF signals
type BFType is array(0 to NUM_CELLS-1) of std_logic; 
signal BF : BFType := ('0','1','1','0','0','1','1','0','0','1','1','0','0','1','0','1','1','0','1','1','1','0','1',
'1','1','1','1','0','1','1','1','0','0','1','1','1','0','0','1','0','1','0','1','1','1','1','1','0','0','0','1','0',
'0','0','1','0','0','1','1','1','1','0','0','1','0','0','0','0','0','0','0','0','0','0','0','1','1','1','1','1','0','0',
'1','1','1','0','0','1','0','1','0','0','1','1','1','1','1','0','0','1','1','0','1','0','0','0','0','0','0','1','1','1',
'1','0','1','0','0','0','0','1','1','0','1','1','0','1','1','1','1','1','0','1','1','1','0','0','0','0','1','1','0','0',
'1','1','0','1','0','0','0','1','0','1','0','1','0','1','0','1','1','1','1','0','0','1','1','0','1','0','0','1','0','0','1',
'1','0','0','1','1','0','0','1','0','0','0','0','0','1','0','0','0','0','1','1','1','0','0','0','0','1','0','1','0','1','1','0',
'1','0','0','0','1','1','0','1','0','0','0','1','1','1','0','0','1','0','1','1','0','0','1','1','0','1','0','0','1','1','0','1','1',
'1','0','0','0','1','1','0','0','1','1','1','1','0','0','1','0','0','0');			
signal Clean: std_logic := '0';					--If clean = 1 then BF={0,0...}
signal Hash_Indexs_odd: indexs (hash_number-1 downto 0) := (others => (others => '0')); --Indexs odd
signal Hash_Indexs_even: indexs (hash_number-1 downto 0) := (others => (others => '0')); --Indexs even
signal even_odd: std_logic := '0';

begin

-- Hash functions instantiation
-- Odd hashes 
 GEN_HASH_ODD: for i in 0 to hash_number-1 generate
	v_hash_odd: H3 PORT MAP(DoHash => Enable,Seeds_Values => s_odd(i),Din => BAddress_din,Dout =>Hash_Indexs_odd(i));
 end generate GEN_HASH_ODD;
 -- Even hashes
 GEN_HASH_EVEN: for i in 0 to hash_number-1 generate
	v_hash_even: H3 PORT MAP(DoHash => Enable,Seeds_Values => s_even(i),Din => BAddress_din,Dout =>Hash_Indexs_even(i));
 end generate GEN_HASH_EVEN;

 process (CLK)
    begin
        if (CLK'event and CLK = '1') then	
            if (WE = '1') and (enable = '1') then 
				if (even_odd = '0') then -- Even
					-- Set to 1 hash indexs
					BF(conv_integer(Hash_Indexs_even(0))) <='1';
					BF(conv_integer(Hash_Indexs_even(1))) <='1';	
				else -- Odd (even_odd = '1')
					-- Set to 0 hash indexs
					BF(conv_integer(Hash_Indexs_odd(0))) <='0';
					BF(conv_integer(Hash_Indexs_odd(1))) <='0';
				end if;
			end if;
	    end if;
    end process;
	

-- Even = '0', odd = '1'
even_odd <= BAddress_din(0);

-- Dout = '1' only if the 2 indexs has 1's (even) or the 2 indexs has 0's (odd)
Dout <= '0' when ((RE='0') or (Enable='0')) 
		else BF(conv_integer(Hash_Indexs_even(0))) and BF(conv_integer(Hash_Indexs_even(1))) when even_odd = '0'
		else  not (BF(conv_integer(Hash_Indexs_odd(0))) or BF(conv_integer(Hash_Indexs_odd(1))));

end Behavioral;