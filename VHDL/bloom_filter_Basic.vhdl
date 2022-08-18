library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use IEEE.std_logic_arith.all;
use work.bf_defines.ALL;

-- Bloom filter of NUM_CELLS entries of 1 bit and 2 hashes
entity BF_basic is 
	Generic(NUM_CELLS: natural; MAX_CAPACITY: natural); --Bloom filter cells and max 1's in filter
	Port (
			Clk : in std_logic := '0';
			BAddress_din: in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@ of addr_bits bits
			WE : in std_logic := '0';			   	     		 --Write enable	
			RE : in std_logic := '0';		   		     		 --Read enable		  
			Enable: in std_logic := '0';		   	             --If enable='1' its possible to write and read
			s: in seeds(hash_number-1 downto 0);
			Dout : out std_logic	           		    	     --Output -> 1 its on BF, otherwise 0
	     );
end BF_basic;

architecture Behavioral of BF_basic is
	--Counter declaration
	COMPONENT reg is
		Generic(BITS: natural); --Number of bits the register keeps
		Port ( 
			Din : in  STD_LOGIC_VECTOR (BITS-1 downto 0);
           	clk : in  STD_LOGIC;
	   		reset : in  STD_LOGIC;
           	load : in  STD_LOGIC;
           	Dout : out  STD_LOGIC_VECTOR 
     	);
	END COMPONENT;
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
signal BF : BFType := (others => '0');			--Bloom filter 			
signal Clean: std_logic := '0';				--If clean = 1 then BF={0,0...}
signal Hash_Indexs: indexs (hash_number-1 downto 0) := (others => (others => '0')); --Indexs 
--Counter signals
signal counter_Din : std_logic_vector (indexs_bits-1 downto 0) := (others => '0');
signal counter_Dout : std_logic_vector (indexs_bits-1 downto 0) := (others => '0');
signal Load: std_logic := '0';
signal BF_data_index0: std_logic_vector (indexs_bits-1 downto 0) := (others => '0');
signal BF_data_index1: std_logic_vector (indexs_bits-1 downto 0) := (others => '0');

begin
 --Save the number of 1's in BF
 counter : reg generic map (indexs_bits) port map (Din => counter_Din , clk => Clk, reset => Clean,load => Load, Dout => counter_Dout);
-- Hash functions instantiation
 GEN_HASH: for i in 0 to hash_number-1 generate
	v_hash: H3 PORT MAP(DoHash => Enable,Seeds_Values => s(i),Din => BAddress_din,Dout =>Hash_Indexs(i));
 end generate GEN_HASH;

 process (CLK)
    begin
        if (CLK'event and CLK = '1') then	
	  if (clean='1') then 
		BF <= (others => '0');
	  else	 
            if (WE = '1') and (enable = '1') then 
			-- Set to 1 hash indexs
			BF(conv_integer(Hash_Indexs(0))) <='1';
			BF(conv_integer(Hash_Indexs(1))) <='1';	
		end if;
	  end if;
        end if;
    end process;

Load <= WE and Enable;									       			-- Load counter with new values if the BF is rewrited					      
Clean <= '1' when conv_integer(counter_Dout) >= MAX_CAPACITY else '0';  -- Clean the counter if reset or the numb of 1's is MAX_CAPACITY
--Add the number of new 1's in BF to counter
BF_data_index0 <= conv_std_logic_vector(not(BF(conv_integer(Hash_Indexs(0)))),indexs_bits);
BF_data_index1 <= conv_std_logic_vector(not(BF(conv_integer(Hash_Indexs(1)))),indexs_bits);
counter_Din <= counter_Dout + BF_data_index0 + BF_data_index1 when Hash_Indexs(0) /= Hash_Indexs(1) else counter_Dout + BF_data_index0; 

Dout <= '0' when ((RE='0') or (Enable='0')) else BF(conv_integer(Hash_Indexs(0))) and BF(conv_integer(Hash_Indexs(1))); -- 1 only if the 2 indexs has 1's

end Behavioral;