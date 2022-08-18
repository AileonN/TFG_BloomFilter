library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use IEEE.std_logic_arith.all;
use work.bf_defines.ALL;

-- Bloom filter of NUM_CELLS entries of BITS_PER_CELL bits. Their max value of each cell is 2^BITS_PER_CELLS-1
entity BF_SBF is 
	Generic(NUM_CELLS: natural; BITS_PER_CELL: natural); --Bloom filter cells and the bits per cell
	Port (
		Clk : in std_logic := '0';
		BAddress_din: in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@ of addr_bits bits
		WE : in std_logic := '0';			   			 						 --Write enable	
		RE : in std_logic := '0';			   			 						 --Read enable		  
		Enable: in std_logic := '0'; 			   			 					 --If enable='1' its possible to write and read
		s: in seeds(hash_number-1 downto 0);									 --hash seeds
		Dout : out std_logic		           				 					 --Output -> 1 its on BF, otherwise 0
	     );
end BF_SBF;

architecture Behavioral of BF_SBF is
-- Component H3 declaration (hash used for index the filter)
	COMPONENT H3 is
		Port(
			DoHash: in std_logic :='0'; --DoHash=1 -> Dout = Index
			Seeds_Values: in indexs(addr_bits-1 downto 0); --Seed values
			Din:  in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@'s of addr_bits bits
			Dout: out std_logic_vector(indexs_bits-1 downto 0):= (others => '0') --@ for index bloom filter
		);
	END COMPONENT;
	
	COMPONENT LFSR is
	GENERIC (
		Num_Bits : integer;					    --Size of random number 
		number: integer							--Number of the LFSR component (for assign a unique value)
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
  END COMPONENT;
  
  
-- BF signals	
type BFType is array(0 to NUM_CELLS-1) of std_logic_vector(BITS_PER_CELL-1 downto 0);   			 --The BF has NUM_CELLS cells of BITS_PER_CELLS bits each one
constant MAX: std_logic_vector :=(0 to BITS_PER_CELL -1 => '1');									 --MAX value for each cells
signal BF : BFType := (others => (0 to BITS_PER_CELL-1 => '0'));
--signal BF : BFType := (others => (0 to BITS_PER_CELL-1 => '1'));							 		 --FOR DEBUG and see how the BF delete cell value correctly
signal Hash_Indexs: indexs (hash_number-1 downto 0) := (others => (others => '0')); 				 --Indexs 
-- LFSR signals  
signal random_number : indexs(num_delete_cells_sbf-1 downto 0); 									 --Random cells to delete (output from LFSRs)
signal LFSR_Done : std_logic;

begin

-- Hash functions instantiation
 GEN_HASH: for i in 0 to hash_number-1 generate
	v_hash: H3 PORT MAP(DoHash => Enable,Seeds_Values => s(i),Din => BAddress_din,Dout =>Hash_Indexs(i));
 end generate GEN_HASH;
-- LFSR instantiation
 GEN_LSFR: for i in 0 to num_delete_cells_sbf-1 generate
	u_LFSR: LFSR GENERIC MAP (Num_Bits => indexs_bits, number => i) PORT MAP(Clk => Clk,Enable => Enable, i_Seed_DV => '0', i_Seed_Data => Seed_LFSR(i),o_LFSR_Data => random_number(i), o_LFSR_Done => open);
 end generate GEN_LSFR;
 
 process (CLK)
    begin
        if (CLK'event and CLK = '1') then	 
            if (WE = '1') and (enable = '1') then 
				-- Substract 1 to BF random cells value
				for i in 0 to num_delete_cells_sbf-1 loop
					if (BF(conv_integer(random_number(i))) > conv_std_logic_vector(0,BITS_PER_CELL)) then
						BF(conv_integer(random_number(i))) <= BF(conv_integer(random_number(i))) - conv_std_logic_vector(1,BITS_PER_CELL);
					end if;
				end loop;
				-- Set to MAX value the cells accessed
				BF(conv_integer(Hash_Indexs(0))) <= MAX;
				BF(conv_integer(Hash_Indexs(1))) <= MAX;
			end if;
        end if;
    end process;

-- Return '1' if the value is >= 1
Dout <= '0' when ((RE='0') or (Enable='0')) else 
	'1' when BF(conv_integer(Hash_Indexs(0))) >= conv_std_logic_vector(1,BITS_PER_CELL) and BF(conv_integer(Hash_Indexs(1))) >= conv_std_logic_vector(1,BITS_PER_CELL) else '0';
end Behavioral;