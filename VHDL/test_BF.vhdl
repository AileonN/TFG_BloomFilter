-- Test_BF

  LIBRARY ieee;
  USE ieee.std_logic_1164.ALL;
  USE ieee.numeric_std.ALL;
  USE work.bf_defines.ALL;

  ENTITY test_BF IS
  END test_BF;
 -- BFtest
  ARCHITECTURE behavior OF test_BF IS 

	 -- Constants
	constant CLK_period : time := 10 ns; 	  --Clock period definitions

	
	-- Component BF_basic declaration (bloom filter basic)
	-- Bloom filter of NUM_CELLS entries of 1 bit and 2 hashes
	COMPONENT BF_basic is 
		Generic(NUM_CELLS: natural; MAX_CAPACITY: natural); --Bloom filter cells and max 1's in filter
		Port (
			Clk : in std_logic;
			BAddress_din: in std_logic_vector(addr_bits-1 downto 0); --@ of addr_bits bits
			WE : in std_logic;			  				   		   --Write enable	
			RE : in std_logic;			   				           --Read enable		  
			Enable: in std_logic; 			   			           --If enable='1' its possible to write and read
			s: in seeds(hash_number-1 downto 0);				   --hash seeds
			Dout : out std_logic	           	   		           --Output -> 1 its on BF, otherwise 0
	    	 );
	END COMPONENT;
	
	-- Component BF_A2 declaration (bloom filter A2)
	-- Bloom filter A2 of NUM_CELLS entries for both where each entry has 1 bit
	COMPONENT BF_A2 is 
		Generic(NUM_CELLS: natural; MAX_CAPACITY: natural); --Bloom filter cells and max 1's in filter
		Port (
			Clk : in std_logic;
			BAddress_din: in std_logic_vector(addr_bits-1 downto 0); --@ of addr_bits bits
			WE : in std_logic;			   				   		   --Write enable	
			RE : in std_logic;			   				   		   --Read enable		  
			Enable: in std_logic; 			   			   		   --If enable='1' its possible to write and read
			s: in seeds(hash_number-1 downto 0);				   --hash seeds
			Dout : out std_logic		           		   		   --Output -> 1 its on BF, otherwise 0
	     );
	END COMPONENT;
	-- Component BF_SBF declaration (bloom filter sbf)
	-- Bloom filter of NUM_CELLS entries of BITS_PER_CELL bits. Their max value of each cell is 2^BITS_PER_CELLS-1
	COMPONENT BF_SBF is 
		Generic(NUM_CELLS: natural;  BITS_PER_CELL: natural); --Bloom filter cells, number of cells to delete en each access and the cell bits
		Port (
			Clk : in std_logic;
			BAddress_din: in std_logic_vector(addr_bits-1 downto 0):= (others => '0'); --@ of addr_bits bits
			WE : in std_logic;			   											 --Write enable	
			RE : in std_logic;			   											 --Read enable		  
			Enable: in std_logic; 			   										 --If enable='1' its possible to write and read
			s: in seeds(hash_number-1 downto 0);									 --hash seeds
			Dout : out std_logic		           									 --Output -> 1 its on BF, otherwise 0
	    	 );
	END COMPONENT;
	-- Component BF_SR declaration 
	-- Bloom filter of NUM_CELLS entries of 1 bit and hash_number*2 hashes (Half for even @ and the other half for odd)
	COMPONENT BF_SR
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
	END COMPONENT;
	
	-- Signals
	signal Clk: std_logic := '0';
	constant zeros: std_logic_vector(27 downto 0) := (others => '0');					
  	-- Signals for BF
	signal BAddress_din: std_logic_vector(addr_bits-1 downto 0) := (others => '0');		 	 --Input, @block
	signal Dout_bf : std_logic := '0'; 	  				         					 		--Output of bloom filter
	signal Enable_bf: std_logic := '0';
	
	constant s: seeds(hash_number-1 downto 0) := ((X"34",X"ee",X"54",X"b7",X"a5",X"7e",X"8f",X"b5",X"e1",X"95",X"59",X"e1",X"bd",X"db",X"2e",X"32",X"fc",X"9c",X"4f",X"18",X"98",X"cf",X"ad",X"8c",X"77",X"68",X"32",X"a7",X"e4",X"7c",X"a7",X"7d"),
	(X"18",X"ee",X"40",X"c1",X"92",X"71",X"b8",X"3d",X"f9",X"c6",X"33",X"48",X"54",X"a0",X"6e",X"ac",X"f4",X"c7",X"a3",X"00",X"9f",X"8e",X"7b",X"fa",X"31",X"5c",X"5d",X"02",X"6c",X"a6",X"7d",X"0a")); --Values of hash seeds
    constant s_2: seeds(hash_number-1 downto 0) := ((X"44",X"5d",X"0a",X"10",X"4d",X"69",X"4a",X"d0",X"8c",X"99",X"19",X"4b",X"a9",X"05",X"8a",X"a2",X"b8",X"d4",X"e1",X"c8",X"28",X"7a",X"44",X"a0",X"93",X"05",X"7d",X"db",X"0f",X"e3",X"0d",X"2a"),
	(X"72",X"bc",X"9e",X"32",X"5c",X"4f",X"4e",X"5b",X"ed",X"11",X"95",X"ee",X"ed",X"7e",X"8d",X"9f",X"6a",X"90",X"5f",X"35",X"44",X"1c",X"08",X"77",X"23",X"aa",X"78",X"c1",X"3c",X"d6",X"1f",X"94"));
  BEGIN
	-- Bloom filters instantation
	--u_bf_basic: BF_basic GENERIC MAP (num_cells,4) PORT MAP(Clk => Clk,s => s, BAddress_din => Baddress_din, WE => Enable_bf, RE => Enable_bf, Enable => Enable_bf, Dout => Dout_bf);
	--u_bf_A2: BF_A2 GENERIC MAP (num_cells,4) PORT MAP(Clk => Clk, s => s, BAddress_din => Baddress_din, WE => Enable_bf, RE => Enable_bf, Enable => Enable_bf, Dout => Dout_bf);
	--u_bf_SBF: BF_SBF GENERIC MAP (num_cells,2) PORT MAP(Clk => Clk, s => s, BAddress_din => BAddress_din, WE => Enable_bf, RE => Enable_bf, Enable => Enable_bf, Dout => Dout_bf);
	u_bf_SR: BF_SR GENERIC MAP (num_cells) PORT MAP (Clk => Clk,s_even => s, s_odd => s_2, BAddress_din => Baddress_din, WE => Enable_bf, RE => Enable_bf, Enable => Enable_bf, Dout => Dout_bf);

	-- Clock process 
   	CLK_process :process  	 	
	begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   	end process;

 	test_proc: process
   	begin	
		-- En el mismo ciclo: Obtengo la hash, leo el contenido del filtro y escribo en el filtro. 
		-- Habilitamos el filtro -> permitimos escribir y leer 
		Enable_bf <='1';
----------------------------------------------------------------------------------------------------------------------------------- 
		
		-- BF basico TEST. @'s 32 bits, 256 entradas en el filtro, 2 hashes y un número máximo de 1's igual a 4:
		
			-- Casos de prueba:
			-- (Leyenda de señales: salida del filtro (Dout_bf), filtro (BF), indices hash (Hash_Indexs) 
			--  dirección solicitada (BAddress_din), valor del contador de 1's (counter_Dout))
			-- - Entrada/Salida:
				-- 1. Solicito una dirección que no es recordada por el filtro (valor de una de las celdas indicadas por
				   -- las hash = 0): La salida debe ser 0 y en los indices indicados por la hash se debe introducir el valor 1.
				-- 2. Solicito una dirección que es recordada por el filtro (valor de las celdas indicadas por
				   -- las hash 1): La salida debe ser 1.
			-- - Borrado:
				-- 3. El filtro tiene un número de 1's >= 4: La salida del contador es >=4 y el contenido del filtro se limpia.
				-- 4. El filtro tiene un número de 1's < 4: El estado del filtro se mantiene y el contador de 1's sigue su curso.
		
		-- -- TEST CASOS DE PRUEBA 1 y 4
		-- -- Primera aparición de una dirección (0x80000000) -> Salida 0 y se escribe en el filtro un '1' en los indices (52,24) obtenidos por las hashes.
		-- -- El contador de 1's toma el valor 2. ( +2 al valor anterior del contador porque las hashes apuntan a indices cuyo valor 
		-- -- era 0).
		-- BAddress_din <=  "1000" & zeros; 
		-- wait for CLK_period;
		
		-- -- TEST CASOS DE PRUEBA 2 y 4
		-- -- Segunda aparición de una dirección (0x80000000) -> Salida 1 y se mantiene el estado del filtro.
		-- -- El contador mantiene su valor al no haber nuevos 1's en el filtro porque el valor de los índices obtenidos con las hashes vale 1. 
		-- wait for CLK_period;
		
		-- -- TEST CASOS DE PRUEBA 1 y 4
		-- -- Primera aparición de una dirección (0x40000000) -> Salida 0 y se escribe en el filtro un '1' en los indices (238,238) obtenidos por las hashes
		-- -- El contador de 1's tomará el valor 3 ( +1 al valor anterior del contador porque las hashes apuntan al mismo índice cuyo valor
		-- -- anterior era 0).
		-- BAddress_din <= "0100" & zeros;	
		-- wait for CLK_period;
		
		-- -- TEST CASOS DE PRUEBA 1 y 4
		-- -- Primera aparición de una dirección (0x90000000) -> Salida 0 y se escribe en el filtro un '1' en los indices (131,217) obtenidos por las hashes
		-- -- El contador de 1's tomará el valor 5 (+2 al valor anterior del contador porque las hashes escriben en índices distintos y cuyo
		-- -- valor anterior era 0)
		-- BAddress_din <= "1001" & zeros;	
		-- wait for CLK_period;
		
		-- -- TEST CASOS DE PRUEBA 2 Y 3
		-- -- Segunda aparición de una dirección (0x90000000) -> Salida 1 y como el contador tiene un valor >= 4, se borra el contenido del filtro.
		-- -- El contador retoma su estado incial, es decir, vale 0.
		-- wait for CLK_period;
		
		-- -- TEST CASOS DE PRUEBA 1 y 4 
		-- -- Se solicita una dirección que aparecia en el filtro antes de su borrado pero que ahora no está -> 
		-- -- Salida 0 y se escribe en el filtro un '1' en los indices (54,24) obtenidos por las hashes.
		-- -- El contador de 1's tomará el valor 2. ( +2 al valor anterior del contador porque las hashes apuntan a indices distintos y cuyo valor 
		-- -- era 0).
		-- BAddress_din <=  "1000" & zeros;
		-- wait for CLK_period;
------------------------------------------------------------------------------------------------------------------------------------	
		
		-- BF A2 TEST: 2 filtros de 256 entradas, @'s de 32 bits, 2 hashes y un número máximo de 1's igual a 4
			--Estado inicial:
			--Filtro actual -> BF_0 
			--Filtro antiguo -> BF_1
			
		-- Casos de prueba:
		-- (Leyenda de señales: salida del contador (counter_Dout), salida del filtro (Dout_bf), filtro (BF_0 y BF_1), 
		-- filtro actual (Actual_Filter), indices hash (Hash_Indexs), dirección solicitada (BAddress_din))
		-- - Entrada/Salida:
			-- 1. Solicito una dirección que no es recordada por ambos filtros (valor 0 en uno de los indices obtenidos por
			-- las hashes de ambos filtros): La salida debe ser 0 y en los indices del filtro actual obtenidos por las hashes 
			-- se debe introducir el valor 1.
			-- 2. Solicito una dirección que es recordada por ambos filtros (valor 1 en los indices obtenidos por
			-- las hashes de ambos filtros): La salida debe ser 1.
			
			-- 3. Solicito una dirección que no es recordada por el filtro actual pero si por el antiguo 
			-- (valor 0 en uno de los indices obtenidos por las hashes del filtro actual pero, valor 1 en todos los indices
			-- del filtro antiguo): La salida debe ser 1 y en los indices del filtro actual se debe introducir el valor 1.
			-- 4. Solicito una dirección que es recordada por el filtro actual pero no por el antiguo 
			-- (valor 1 en todos los indices obtenidos por las hashes del filtro actual pero, valor 0 en uno de los indices
			-- del filtro antiguo): La salida debe ser 1.
			
		-- - Borrado:
			-- 5. El filtro tiene un número de 1's >= 4: La salida del contador es >=4 y se genera el intercambio de filtros (filtro 
			-- antiguo <-> filtro actual). En este intercambio, el filtro que pasa a ser el actual se limpia y se escribe en ambos
			-- filtros el valor 1 en los indices marcados por las hashes. El contador pasa a contabilizar los 1's del nuevo filtro actual.
			-- 6. El filtro tiene un número de 1's < 4: El estado del filtro se mantiene y el contador de 1's sigue su curso.
			
			
		
		
		-- -- TEST CASO DE PRUEBA 1 y 6 
		-- -- Primera aparición de una dirección para ambos filtros (0x80000000) -> 
		-- -- Salida 0 y se escribe en el filtro actual un '1' en los indices (52,24) obtenidos por las hashes.
		-- -- El contador de 1's del filtro actual toma el valor 2 (+2 al valor anterior porque las hashes apuntan a índices
		-- -- diferentes cuyo valor era 0)
		-- BAddress_din <=  "1000" & zeros; 
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 4 y 6
		-- -- Segunda aparición de una dirección para el filtro actual (0x80000000) -> Salida 1 y se mantiene el estado del filtro.
		-- -- El contador mantiene su valor al no haber nuevos 1's en el filtro actual.
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 1 Y 6
		-- -- Primera aparición de una dirección para ambos fitros (0x40000000) -> 
		-- -- Salida 0 y se escribe en el filtro actual un '1' en los indices (238,238) obtenidos por las hashes
		-- -- El contador de 1's del filtro actual tomará el valor 3 ( +1 al valor anterior del contador porque las hashes apuntan 
		-- -- al mismo índice cuyo valor era 0).
		-- BAddress_din <= "0100" & zeros;	
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 1 Y 6
		-- -- Primera aparición de una dirección para ambos filtros (0x90000000) -> 
		-- -- Salida 0 y se escribe en el filtro actual un '1' en los indices (131,217) obtenidos por las hashes
		-- -- El contador de 1's del filtro actual tomará el valor 5 (+2 al valor anterior del contador porque las hashes escriben en 
		-- -- índices distintos y cuyo valor era 0)
		-- BAddress_din <= "1001" & zeros;	
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 4 y 5
		-- -- Segunda aparición de una dirección para el filtro actual (0x90000000) -> 
		-- -- Salida 1 y como el contador tiene un valor >= 4, se intercambian los filtros y se escribe en el nuevo filtro actual 
		-- -- un '1' en los indices (131,217).
		-- -- El contador de 1's del filtro actual tomará el valor 2 (porque las hashes dan índices distintos cuyo valor
		-- -- es 0 porque se borra el contenido del filtro al convertirse en el actual) 
		-- -- Estado actual: BF_0 -> Filtro antiguo y BF_1 -> Filtro actual
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 3 Y 6
		-- -- Segunda aparición de una dirección para el filtro antiguo (0x80000000) -> 
		-- -- Salida 1 y se escribe en el filtro actual un '1' en los indices (54,24) obtenidos por las hashes.
		-- -- El contador de 1's del filtro actual tomará el valor 4. (+2 al valor anterior porque las hashes apuntan a índices
		-- -- diferentes cuyo valor era 0)
		-- BAddress_din <=  "1000" & zeros;
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 2 y 6
		-- -- Segunda aparición de una dirección para ambos filtros (0x80000000) -> 
		-- -- Salida 1 y como el contador tiene un valor >= 4, se intercambian los filtros (se puede ver como se borra el contenido 
		-- -- del que pasa a ser el filtro actual.) y se escribe un '1' en los indices (52,24) del nuevo filtro actual, que ya no
		-- -- recuerda dicha direccion.
		-- -- El contador de 1's del filtro actual tomará el valor 2 (porque las hashes dan índices distintos cuyo valor
		-- -- es 0 porque se borra el contenido del filtro al convertirse en el actual) 
		-- wait for CLK_period;

-----------------------------------------------------------------------------------------------------------------------------------
		-- BF SBF TEST: filtro de 256 entradas de 2 bits, @'s de 32 bits, 2 hashes y se borran 4 celdas por cada acceso
		
		-- Casos de prueba:
		-- (Leyenda de señales: salida del filtro (Dout_bf), filtro (BF), indices de borrado (random_number), 
		-- indices hash (Hash_Indexs), dirección solicitada (BAddress_din))
		-- - Entrada/Salida
			-- 1. Solicito una dirección que no es recordada por el filtro (valor de una de las celdas indicadas por
			-- las hash = 0): La salida debe ser 0 y en los indices indicados por la hash se debe introducir el valor 3.
			-- 2. Solicito una dirección que es recordada por el filtro (valor de las celdas indicadas por
			-- las hash >0): La salida debe ser 1 y en los indices indicados por la hash se debe introducir el valor 3. 
		-- - Borrado: 
			-- 3. Un indice de borrado apunta a una celda con valor 0: Esa celda debe seguir en 0 después del borrado.
			-- 4. Un indice de borrado apunta a una celda con valor >0: Esa celda debe disminuir su valor en 1.
			-- 5. Un indice de borrado apunta a la misma celda que los indices de entrada (hashes): Esa celda debe tener valor 3.
		
		-- -- TEST CASO DE PRUEBA 1 y 3
		-- -- Primera aparición de una dirección (0x80000000) -> Salida 0 y se escribe en el filtro un '3' en los indices (52,24) obtenidos por las hashes.
		-- -- Los 4 indices a borrar son (16,48,33,245) pero como todos valen 0, su valor se queda en 0
		-- BAddress_din <=  "1000" & zeros; 
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 1 y 3
		-- -- Primera aparición de una dirección (0x20000000) -> Salida 0 y se escribe en el filtro un '3' en los indices (84,64) obtenidos por las hashes.
		-- -- Los 4 indices a borrar son (32,97,66,234) pero como todos valen 0, su valor se queda en 0
		-- BAddress_din <= "0010" & zeros;
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 2 y 4
		-- -- Segunda aparición de una dirección (0x80000000) -> Salida 1 y se escribe en el filtro un '3' en los indices (52,24) obtenidos por las hashes.
		-- -- Los 4 indices a borrar son (64,194,133,212). En este caso el indice 64 vale 3 por lo tanto, pasa a valer 2. ("borrado")
		-- BAddress_din <=  "1000" & zeros;
		-- wait for CLK_period;
		
		-- -- TEST CASO DE PRUEBA 1 y 5
		-- -- Primera aparición de una dirección (0x1) -> Salida 0 y se escribe en el filtro un '3' en los indices (125,10) obtenidos por las hashes
		-- -- Los 4 indices a borrar son (129,132,10,169). En este caso el indice 10 coincide con uno de los indices escritos por tanto,
		-- -- no se produce un borrado del mismo.
		-- BAddress_din <= zeros & "0001";
		-- wait for CLK_period;
-----------------------------------------------------------------------------------------------------------------------------------

		-- BF SR TEST. @'s 32 bits, 256 entradas en el filtro, 4 hashes (2 para pares y 2 para impares):
		-- "PAR": Si el últ bit de la direccion de BLOQUE es 0
		-- "IMPAR": Si el últ bit de la direccion de BLOQUE es 1
		
			-- Casos de prueba:
			-- (Leyenda de señales: salida del filtro (Dout_bf), filtro (BF), indices hash pares (Hash_Indexs_even) 
			-- indices hash impares (Hash_Indexs_odd), dirección solicitada (BAddress_din))
			-- - Entrada/Salida:
				-- 1. Solicito una dirección "PAR" que no es recordada por el filtro (valor de una de las celdas indicadas por
				   -- las hash = 0): La salida debe ser 0 y en los indices indicados por la hash se debe introducir el valor 1.
				-- 2. Solicito una dirección "PAR" que es recordada por el filtro (valor de las celdas indicadas por
				   -- las hash 1): La salida debe ser 1.
				-- 3. Solicito una dirección "IMPAR" que no es recordada por el filtro (valor de una de las celdas indicadas por
				   -- las hash = 1): La salida debe ser 0 y en los indices indicados por la hash se debe introducir el valor 0.
				-- 4. Solicito una dirección "IMPAR" que es recordada por el filtro (valor de las celdas indicadas por
				   -- las hash 0): La salida debe ser 1.
				
		-- TEST CASOS DE PRUEBA 1 
		-- Primera aparición de una dirección PAR (0xA) -> Salida 0 y se escribe en el filtro un '1' en los indices (67,17) obtenidos por las hashes pares.
		BAddress_din <= zeros & "1010"; 
		wait for CLK_period;
		
		-- TEST CASOS DE PRUEBA 2
		-- Segunda aparición de una dirección (0xA) -> Salida 1 y se mantiene el estado del filtro.
		wait for CLK_period;
		
		-- TEST CASOS DE PRUEBA 3
		-- Primera aparición de una dirección IMPAR (0x1) -> Salida 0 y se escribe en el filtro un '0' en los indices (42,148) obtenidos por las hashes impares
		BAddress_din <= zeros & "0001";	
		wait for CLK_period;
		
		-- TEST CASOS DE PRUEBA 4
		-- Segunda aparición de una dirección (0x1) -> Salida 1 y se mantiene el estado del filtro.
		wait for CLK_period;
------------------------------------------------------------------------------------------------------------------------------------	
		-- Deshabilitamos el filtro -> no podemos ni leer su contenido ni escribir en él 
		Enable_bf <='0';
		wait;


   	end process;
	

  END;
