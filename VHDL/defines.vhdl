library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

package bf_defines is
-- Constants/Defines
constant hash_number: natural := 2;	  --Number of hash functions
constant indexs_bits: natural := 8;	  --Number of bits to index BF Ex: Need 8 bits to index 256 entries
constant num_cells: natural:= 256;	  --Number of BF cells
constant addr_bits: natural :=32;	  --Size of @
	-- SBF filter
	constant num_delete_cells_sbf: natural := 4; --Number of cells to delete
	-- PI filter
	constant max_bits_xorPI: natural := 10;		--Number of bits for the xor function (PI filter)
-- Types
type indexs is array (natural range <>) of std_logic_vector(indexs_bits-1 downto 0); 
type seeds is array (natural range <>) of indexs(addr_bits-1 downto 0); 	  --Number of seeds * hash == Number of @'s bits
type neigth_bits is array (natural range <>) of std_logic_vector(7 downto 0); --uint8_t
--Constants Variables
	--Seeds value for LFSR, SBF filter
	constant Seed_LFSR : indexs(num_delete_cells_sbf-1 downto 0):= (X"10",X"30",X"21",X"F5");	--One seed for LFSR or for each delete cell
	--Bits for the xor function, PI filter
	constant fxorPI_bits : neigth_bits(max_bits_xorPI-1 downto 0) := (X"01",X"03",X"05",X"07",X"09",X"0B",X"0D",X"0F",X"11",X"13");
end package bf_defines;