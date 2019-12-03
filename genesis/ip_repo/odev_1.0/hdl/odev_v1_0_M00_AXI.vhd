library ieee;
library unisim;
use unisim.vcomponents.all;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity odev_v1_0_M00_AXI is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Base address of targeted slave
		C_M_TARGET_SLAVE_BASE_ADDR	: std_logic_vector	:= x"30000000";
		-- Burst Length. Supports 1, 2, 4, 8, 16, 32, 64, 128, 256 burst lengths
		C_M_AXI_BURST_LEN	: integer	:= 16;
		-- Thread ID Width
		C_M_AXI_ID_WIDTH	: integer	:= 1;
		-- Width of Address Bus
		C_M_AXI_ADDR_WIDTH	: integer	:= 32;
		-- Width of Data Bus
		C_M_AXI_DATA_WIDTH	: integer	:= 32
	);
	port (
		-- Users to add ports here
		-- M_TRIG_MEM_CPY : in std_logic;
		-- Initiate AXI transactions
--		M_G_START	: in std_logic;
--		M_G_PULSE : in std_logic;
		M_STREAM_START: in std_logic;
        M_SRC_ADDR : in std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
        M_SRC_LEN : in std_logic_vector(15 downto 0);
		M_ITAB_OUT_VALID: in std_logic;
        M_ITAB_OUT_TRANS_REQ : out std_logic;
        M_ITAB_EMPTY : in std_logic;
        M_RDATA: out std_logic_vector(31 downto 0);
        M_RDATA_VALID: out std_logic;
        M_RDBUFF_FULL: in std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line
		-- Asserts when transaction is complete
		-- Global Clock Signal.
		M_AXI_ACLK	: in std_logic;
		-- Global Reset Singal. This Signal is Active Low
		M_AXI_ARESETN	: in std_logic;
		-- Master Interface Write Address ID
		M_AXI_AWID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Master Interface Write Address
		M_AXI_AWADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		-- Burst length. The burst length gives the exact number of transfers in a burst
		M_AXI_AWLEN	: out std_logic_vector(7 downto 0);
		-- Burst size. This signal indicates the size of each transfer in the burst
		M_AXI_AWSIZE	: out std_logic_vector(2 downto 0);
		-- Burst type. The burst type and the size information, 
    -- determine how the address for each transfer within the burst is calculated.
		M_AXI_AWBURST	: out std_logic_vector(1 downto 0);
		-- Lock type. Provides additional information about the
    -- atomic characteristics of the transfer.
		M_AXI_AWLOCK	: out std_logic;
		-- Memory type. This signal indicates how transactions
    -- are required to progress through a system.
		M_AXI_AWCACHE	: out std_logic_vector(3 downto 0);
		-- Protection type. This signal indicates the privilege
    -- and security level of the transaction, and whether
    -- the transaction is a data access or an instruction access.
		M_AXI_AWPROT	: out std_logic_vector(2 downto 0);
		-- Quality of Service, QoS identifier sent for each write transaction.
		M_AXI_AWQOS	: out std_logic_vector(3 downto 0);
		-- Write address valid. This signal indicates that
    -- the channel is signaling valid write address and control information.
		M_AXI_AWVALID	: out std_logic;
		-- Write address ready. This signal indicates that
    -- the slave is ready to accept an address and associated control signals
		M_AXI_AWREADY	: in std_logic;
		-- Master Interface Write Data.
		M_AXI_WDATA	: out std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte
    -- lanes hold valid data. There is one write strobe
    -- bit for each eight bits of the write data bus.
		M_AXI_WSTRB	: out std_logic_vector(C_M_AXI_DATA_WIDTH/8-1 downto 0);
		-- Write last. This signal indicates the last transfer in a write burst.
		M_AXI_WLAST	: out std_logic;
		-- Write valid. This signal indicates that valid write
    -- data and strobes are available
		M_AXI_WVALID	: out std_logic;
		-- Write ready. This signal indicates that the slave
    -- can accept the write data.
		M_AXI_WREADY	: in std_logic;
		-- Master Interface Write Response.
		M_AXI_BID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Write response. This signal indicates the status of the write transaction.
		M_AXI_BRESP	: in std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the
    -- channel is signaling a valid write response.
		M_AXI_BVALID	: in std_logic;
		-- Response ready. This signal indicates that the master
    -- can accept a write response.
		M_AXI_BREADY	: out std_logic;
		-- Master Interface Read Address.
		M_AXI_ARID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Read address. This signal indicates the initial
    -- address of a read burst transaction.
		M_AXI_ARADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		-- Burst length. The burst length gives the exact number of transfers in a burst
		M_AXI_ARLEN	: out std_logic_vector(7 downto 0);
		-- Burst size. This signal indicates the size of each transfer in the burst
		M_AXI_ARSIZE	: out std_logic_vector(2 downto 0);
		-- Burst type. The burst type and the size information, 
    -- determine how the address for each transfer within the burst is calculated.
		M_AXI_ARBURST	: out std_logic_vector(1 downto 0);
		-- Lock type. Provides additional information about the
    -- atomic characteristics of the transfer.
		M_AXI_ARLOCK	: out std_logic;
		-- Memory type. This signal indicates how transactions
    -- are required to progress through a system.
		M_AXI_ARCACHE	: out std_logic_vector(3 downto 0);
		-- Protection type. This signal indicates the privilege
    -- and security level of the transaction, and whether
    -- the transaction is a data access or an instruction access.
		M_AXI_ARPROT	: out std_logic_vector(2 downto 0);
		-- Quality of Service, QoS identifier sent for each read transaction
		M_AXI_ARQOS	: out std_logic_vector(3 downto 0);
		-- Write address valid. This signal indicates that
    -- the channel is signaling valid read address and control information
		M_AXI_ARVALID	: out std_logic;
		-- Read address ready. This signal indicates that
    -- the slave is ready to accept an address and associated control signals
		M_AXI_ARREADY	: in std_logic;
		-- Read ID tag. This signal is the identification tag
    -- for the read data group of signals generated by the slave.
		M_AXI_RID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		-- Master Read Data
		M_AXI_RDATA	: in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the read transfer
		M_AXI_RRESP	: in std_logic_vector(1 downto 0);
		-- Read last. This signal indicates the last transfer in a read burst
		M_AXI_RLAST	: in std_logic;
		-- Read valid. This signal indicates that the channel
    -- is signaling the required read data.
		M_AXI_RVALID	: in std_logic;
		-- Read ready. This signal indicates that the master can
    -- accept the read data and response information.
		M_AXI_RREADY	: out std_logic
	);
end odev_v1_0_M00_AXI;
 
architecture implementation of odev_v1_0_M00_AXI is
	-- C_TRANSACTIONS_NUM is the width of the index counter for
	-- number of beats in a burst write or burst read transaction.
	 constant  C_TRANSACTIONS_NUM : integer := 4;
	-- Example State machine to initialize counter, initialize write transactions, 
	 -- initialize read transactions and comparison of read data with the 
	 -- written data words.
	 type DMAStateType is ( IDLE, -- This state initiates AXI4Lite transaction  
	 							-- after the state machine changes state to INIT_WRITE
	 							-- when there is 0 to 1 transition on INIT_AXI_TXN
	 				ITAB_READ,			
	 				ITAB_READ_CHK,
	 				MEM_READ,    -- This state initializes read transaction
	 							-- once reads are done, the state machine 
	 							-- changes state to INIT_COMPARE 
	 				ITAB_EMPTY,
	 				RDBUFF_FULL_CHK,
	 				RDBUFF_FULL);

	 signal dma_state  : DMAStateType; 

	-- AXI4FULL signals
	--AXI4 internal temp signals
	signal axi_araddr	: std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arvalid	: std_logic;
	signal axi_rready	: std_logic;
	signal burst_size_bytes: integer := 64;
	signal start_single_burst_read	: std_logic;
	signal error_reg	: std_logic;
	signal burst_read_active	: std_logic;
	--Interface response error flags
	signal rnext	: std_logic;
	signal reg_src_addr : std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	-- inferred BRAM memory
	signal sig_itab_out_trans_req: std_logic; 
	signal sig_src_addr: std_logic_vector(31 downto 0);
	signal sig_src_len: std_logic_vector(15 downto 0); 
    signal sig_itab_empty: std_logic;
    signal sig_intr_trig: std_logic;
    signal sig_rdata: std_logic_vector(31 downto 0);
    signal sig_rdata_valid: std_logic;
    signal rdata_done_len: integer;
    
	attribute MARK_DEBUG : string;
	attribute MARK_DEBUG of dma_state: signal is "TRUE";
	attribute MARK_DEBUG of axi_araddr: signal is "TRUE";
--	attribute MARK_DEBUG of burst_read_active: signal is "TRUE";
	attribute MARK_DEBUG of sig_rdata: signal is "TRUE";
--	attribute MARK_DEBUG of axi_arvalid: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_ARREADY: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_RVALID: signal is "TRUE";
--	attribute MARK_DEBUG of axi_rready: signal is "TRUE";
--	attribute MARK_DEBUG of sig_rdata_valid: signal is "TRUE";
--	attribute MARK_DEBUG of M_AXI_RDATA: signal is "TRUE";
    attribute MARK_DEBUG of rnext: signal is "TRUE";
    attribute MARK_DEBUG of M_AXI_RLAST: signal is "TRUE";
--    attribute MARK_DEBUG of sig_src_len: signal is "TRUE";
--    attribute MARK_DEBUG of rdata_done_len: signal is "TRUE";
    
begin
	-- I/O Connections assignments
	--I/O Connections. Write Address (AW)
	M_AXI_AWID	<= (others => '0');
	--The AXI address is a concatenation of the target base address + active offset range
	M_AXI_AWADDR	<= (others => '0');
	--Burst LENgth is number of transaction beats, minus 1
	M_AXI_AWLEN	<= x"0f"; -- burst len = AWLEN + 1, 16 beats len of a burst
	M_AXI_AWSIZE	<= "010"; -- 4 bytes beat size
	--INCR burst type is usually used, except for keyhole bursts
	M_AXI_AWBURST	<= "01";
	M_AXI_AWLOCK	<= '0';
	--Update value to 4'b0011 if coherent accesses to be used via the Zynq ACP port. Not Allocated, Modifiable, not Bufferable. Not Bufferable since this example is meant to test memory, not intermediate cache. 
	M_AXI_AWCACHE	<= "0010";
	M_AXI_AWPROT	<= "000";
	M_AXI_AWQOS	<= x"0";
	M_AXI_AWVALID	<= '0';
	--Write Data(W)
	M_AXI_WDATA	<= (others => '0');
	--All bursts are complete and aligned in this example
	M_AXI_WSTRB	<= (others => '1');
	M_AXI_WLAST	<= '0';
	M_AXI_WVALID	<= '0';
	--Write Response (B)
	M_AXI_BREADY	<= '0';
	--Read Address (AR)
	M_AXI_ARID	<= (others => '0');
	M_AXI_ARADDR	<= std_logic_vector( unsigned(reg_src_addr) + unsigned( axi_araddr ) );
	M_AXI_ARLEN	<= x"0f"; -- burst len = ARLEN + 1, 16 beats len of a burst
	--The maximum number of bytes to transfer in each data transfer, or beat, in a burst,
	M_AXI_ARSIZE	<= "010"; -- 4 bytes beat size 
	--INCR burst type is usually used, except for keyhole bursts
	M_AXI_ARBURST	<= "01";
	M_AXI_ARLOCK	<= '0';
	--Update value to 4'b0011 if coherent accesses to be used via the Zynq ACP port. Not Allocated, Modifiable, not Bufferable. Not Bufferable since this example is meant to test memory, not intermediate cache. 
	M_AXI_ARCACHE	<= "0010";
	M_AXI_ARPROT	<= "000";
	M_AXI_ARQOS	<= x"0";
	M_AXI_ARVALID	<= axi_arvalid;
	--Read and Read Response (R)
	M_AXI_RREADY	<= axi_rready;

	------------------------------
	--Read Address Channel
	------------------------------

	--The Read Address Channel (AW) provides a similar function to the
	--Write Address channel- to provide the tranfer qualifiers for the burst.

	--In this example, the read address increments in the same
	--manner as the write address channel.

	  process(M_AXI_ACLK)
--	   variable rep: integer;										  
	  begin                                                              
	    if (rising_edge (M_AXI_ACLK)) then                               
			if (M_STREAM_START = '0') then                                 
	        axi_arvalid <= '0'; 
	        axi_araddr <= (others => '0');
--	        rep := 0;                                         
	     -- If previously not valid , start next transaction             
	      else                                                           
	        if (axi_arvalid = '0' and start_single_burst_read = '1') then
	          axi_arvalid <= '1';  
--	          axi_araddr <= std_logic_vector(unsigned(sig_src_addr) + to_unsigned(burst_size_bytes * rep, 32));
              axi_araddr <= std_logic_vector(unsigned(sig_src_addr) + to_unsigned(rdata_done_len, 32));  
--			  rep := rep + 1;                                      
	        elsif (M_AXI_ARREADY = '1' and axi_arvalid = '1') then       
	          axi_arvalid <= '0'; 
	          axi_araddr <= axi_araddr; 
--	          rep := 0;                                      
	        end if;                                                      
	      end if;                                                        
	    end if;                                                          
	  end process;                                                       

	----------------------------------
	--Read Data (and Response) Channel
	----------------------------------

	 -- Forward movement occurs when the channel is valid and ready   
	  rnext <= M_AXI_RVALID and axi_rready;                                 
 
 	-- Burst length counter. Uses extra counter register bit to indicate    
	-- terminal count to reduce decode logic                                
	process(M_AXI_ACLK)                                                   
	begin                                                                 
		if (rising_edge (M_AXI_ACLK)) then                                  
			if (M_STREAM_START = '0' OR start_single_burst_read = '1') then    
				sig_rdata <= (others => '0');
				sig_rdata_valid <= '0';
			else
				if (rnext = '1') then
					sig_rdata <= M_AXI_RDATA;
					sig_rdata_valid <= '1';  
				else 
					sig_rdata <= (others => '0');
					sig_rdata_valid <= '0';
				end if;                                                         
			end if;                                                           
		end if;                                                             
	end process;   
	  
	--/*                                                                    
	-- The Read Data channel returns the results of the read request        
	--                                                                      
	-- In this example the data checker is always able to accept            
	-- more data, so no need to throttle the RREADY signal                  
	-- */                                                                   
	process(M_AXI_ACLK)                                                   
	begin                                                                 
		if (rising_edge (M_AXI_ACLK)) then                                  
			if (M_STREAM_START = '0') then             
				axi_rready <= '0';                                              
			-- accept/acknowledge rdata/rresp with axi_rready by the master    
			-- when M_AXI_RVALID is asserted by slave                         
			else                                                   
				if (M_AXI_RVALID = '1') then                         
					if (M_AXI_RLAST = '1' and axi_rready = '1') then   
						axi_rready <= '0';                               
--					elsif (M_RDBUFF_AL_FULL = '1') then
--						axi_rready <= '0';
					else                                            
						axi_rready <= '1';                              
					end if;                                            
				end if;                                              
			end if;                                                
		end if;                                                  
	end process;  

	M_ITAB_OUT_TRANS_REQ <= sig_itab_out_trans_req;
	M_RDATA <= sig_rdata;
	M_RDATA_VALID <= sig_rdata_valid;
	                                                                               
	process(M_AXI_ACLK)                                                             
	begin                                                                                                      
		if (rising_edge (M_AXI_ACLK)) then                                                                       
			if (M_STREAM_START = '0') then                                                                         
			-- reset condition                                                                                   
			-- All the signals are ed default values under reset condition                                       
				dma_state     <= IDLE;                                                                   
				start_single_burst_read  <= '0';
				sig_itab_out_trans_req <= '0';
			else                                                                                                   
	        -- state transition                                                                                  
			case (dma_state) is
				when IDLE =>                                                                              
				-- This state is responsible to initiate                               
				-- AXI transaction when init_txn_pulse is asserted 
					if (M_STREAM_START = '1') then       
						dma_state  <= ITAB_READ;                                                        
					else                                                                                                                                                               
						start_single_burst_read  <= '0';
						sig_itab_out_trans_req <= '0';
						dma_state  <= IDLE;
					end if;     
	           
				when ITAB_READ =>  
					if (M_STREAM_START = '0') then
						dma_state <= IDLE;
--					elsif (M_ITAB_EMPTY = '1') then
--						sig_itab_out_trans_req <= '1';
--						dma_state <= ITAB_READ_CHK;
					else 
						sig_itab_out_trans_req <= '1';
						dma_state <= ITAB_READ_CHK;
					end if;
	               
				when ITAB_READ_CHK =>
					if (M_STREAM_START = '0') then
						dma_state <= IDLE;
					elsif (M_ITAB_OUT_VALID = '1') then
						sig_src_addr <= M_SRC_ADDR;
						sig_src_len <= M_SRC_LEN;
						sig_itab_out_trans_req <= '0';
						rdata_done_len <= 0;
						dma_state <= MEM_READ;
					else 
						dma_state <= ITAB_READ_CHK;
						if (M_ITAB_EMPTY = '1') then
							sig_itab_out_trans_req <= '1';
					    else 
					       sig_itab_out_trans_req <= '0';
					    end if;
					end if;  
	               
				when MEM_READ =>                                                                                
					if (M_STREAM_START = '0') then
						dma_state <= IDLE;
					-- This state is responsible to issue start_single_read pulse to                               
					-- initiate a memory read transaction. Read transactions will be                                      
					-- issued until burst_read_active signal is asserted.                                          
					-- read controller                                                                             
					elsif (M_AXI_RVALID = '1' and axi_rready = '1' and M_AXI_RLAST = '1') then
--						if (M_RDBUFF_FULL = '1') then
				            dma_state <= RDBUFF_FULL_CHK;
--				        elsif (rdata_done_len + 64 >= to_integer(unsigned(sig_src_len))) then 
--							dma_state <= ITAB_READ;
--						else 
--							rdata_done_len <= rdata_done_len + 64;
--							dma_state <= MEM_READ;
--						end if;
					else  
						-- start next burst read						
						if (axi_arvalid = '0' and burst_read_active = '0' and start_single_burst_read = '0') then    
							start_single_burst_read <= '1';
						else                                                                                            
							start_single_burst_read <= '0'; -- Negate to generate a pulse                               
						end if;
						dma_state <= MEM_READ;						
					end if;   
					                                                                            
	            when RDBUFF_FULL_CHK =>
	                if (M_STREAM_START = '0') then
					   dma_state <= IDLE;
					elsif(M_RDBUFF_FULL = '1')then
					   dma_state <= RDBUFF_FULL;
					elsif (rdata_done_len + 64 >= to_integer(unsigned(sig_src_len))) then 
						dma_state <= ITAB_READ;
					else 
						rdata_done_len <= rdata_done_len + 64;
						dma_state <= MEM_READ;
					end if;     
					                                                                                            
				when RDBUFF_FULL =>                                                                               
	            	if (M_STREAM_START = '0') then
					   dma_state <= IDLE;
					elsif(M_RDBUFF_FULL = '1')then
					   dma_state <= RDBUFF_FULL;
					elsif (rdata_done_len + 64 >= to_integer(unsigned(sig_src_len))) then 
							dma_state <= ITAB_READ;
					else 
						rdata_done_len <= rdata_done_len + 64;
						dma_state <= MEM_READ;
					end if;
					                                                                                                
				when others  =>                                                                                  
					dma_state  <= IDLE;                                                               
				end case  ;                                                                                        
			end if;                                                                                               
		end if;                                                                                                  
	end process;                                                                                               
	                                                                                                                                              
	process(M_AXI_ACLK)                                                                                        
	begin                                                                                                      
		if (rising_edge (M_AXI_ACLK)) then                                                                       
			if (M_STREAM_START = '0') then                                                                         
				burst_read_active <= '0';                                                                            
			--The burst_write_active is asserted when a write burst transaction is initiated                      
			else                                                                                                   
				if (start_single_burst_read = '1')then                                                               
					burst_read_active <= '1';
				elsif (M_AXI_RVALID = '1' and axi_rready = '1' and M_AXI_RLAST = '1') then                           
					burst_read_active <= '0';   
				end if;                                                                                              
			end if;                                                                                                
		end if;                                                                                                  
	end process;    
	-- User logic ends

end implementation;
