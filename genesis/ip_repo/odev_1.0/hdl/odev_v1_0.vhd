library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity odev_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 5;

		-- Parameters of Axi Master Bus Interface M00_AXI
		C_M00_AXI_TARGET_SLAVE_BASE_ADDR	: std_logic_vector	:= x"40000000";
		C_M00_AXI_BURST_LEN	: integer	:= 16;
		C_M00_AXI_ID_WIDTH	: integer	:= 1;
		C_M00_AXI_ADDR_WIDTH	: integer	:= 32;
		C_M00_AXI_DATA_WIDTH	: integer	:= 32
	);
	port (
		-- Users to add ports here
        SW_DMA_IRQ : out std_logic;
		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in std_logic;
		s00_axi_aresetn	: in std_logic;
		s00_axi_awaddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in std_logic;
		s00_axi_araddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in std_logic;

		-- Ports of Axi Master Bus Interface M00_AXI
		m00_axi_aclk	: in std_logic;
		m00_axi_aresetn	: in std_logic;
		m00_axi_awid	: out std_logic_vector(C_M00_AXI_ID_WIDTH-1 downto 0);
		m00_axi_awaddr	: out std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
		m00_axi_awlen	: out std_logic_vector(7 downto 0);
		m00_axi_awsize	: out std_logic_vector(2 downto 0);
		m00_axi_awburst	: out std_logic_vector(1 downto 0);
		m00_axi_awlock	: out std_logic;
		m00_axi_awcache	: out std_logic_vector(3 downto 0);
		m00_axi_awprot	: out std_logic_vector(2 downto 0);
		m00_axi_awqos	: out std_logic_vector(3 downto 0);
		m00_axi_awvalid	: out std_logic;
		m00_axi_awready	: in std_logic;
		m00_axi_wdata	: out std_logic_vector(C_M00_AXI_DATA_WIDTH-1 downto 0);
		m00_axi_wstrb	: out std_logic_vector(C_M00_AXI_DATA_WIDTH/8-1 downto 0);
		m00_axi_wlast	: out std_logic;
		m00_axi_wvalid	: out std_logic;
		m00_axi_wready	: in std_logic;
		m00_axi_bid	: in std_logic_vector(C_M00_AXI_ID_WIDTH-1 downto 0);
		m00_axi_bresp	: in std_logic_vector(1 downto 0);
		m00_axi_bvalid	: in std_logic;
		m00_axi_bready	: out std_logic;
		m00_axi_arid	: out std_logic_vector(C_M00_AXI_ID_WIDTH-1 downto 0);
		m00_axi_araddr	: out std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
		m00_axi_arlen	: out std_logic_vector(7 downto 0);
		m00_axi_arsize	: out std_logic_vector(2 downto 0);
		m00_axi_arburst	: out std_logic_vector(1 downto 0);
		m00_axi_arlock	: out std_logic;
		m00_axi_arcache	: out std_logic_vector(3 downto 0);
		m00_axi_arprot	: out std_logic_vector(2 downto 0);
		m00_axi_arqos	: out std_logic_vector(3 downto 0);
		m00_axi_arvalid	: out std_logic;
		m00_axi_arready	: in std_logic;
		m00_axi_rid	: in std_logic_vector(C_M00_AXI_ID_WIDTH-1 downto 0);
		m00_axi_rdata	: in std_logic_vector(C_M00_AXI_DATA_WIDTH-1 downto 0);
		m00_axi_rresp	: in std_logic_vector(1 downto 0);
		m00_axi_rlast	: in std_logic;
		m00_axi_rvalid	: in std_logic;
		m00_axi_rready	: out std_logic
	);
end odev_v1_0;

architecture arch_imp of odev_v1_0 is

	signal sig_g_start : std_logic;
	signal sig_src_addr : std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
	signal sig_src_len : std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
	signal sig_in_trans_valid: std_logic;
	signal sig_itab_out_trans_req: std_logic;
	signal sig_itab_src_addr: std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
	signal sig_itab_src_len: std_logic_vector (15 downto 0);
	signal sig_Itab_full: std_logic;
	signal sig_itab_empty: std_logic;
--	signal sig_intr: std_logic;
	signal sig_rdata: std_logic_vector(31 downto 0);
	signal sig_rdbuff_almost_full: std_logic;
	signal sig_rdbuff_empty: std_logic;
	signal sig_rdata_valid: std_logic;
    signal sig_outdata: std_logic_vector (31 downto 0);
    signal sig_outvalid: std_logic;
    signal sig_outreq: std_logic;
    signal sig_g_pulse: std_logic; -- global start / stop pulse
	signal sig_itab_out_valid: std_logic;
	signal sig_stream_start: std_logic;
	signal sig_itab_in_trans_done: std_logic;
	signal sig_consume_latency: std_logic_vector(C_M00_AXI_ADDR_WIDTH-1 downto 0);
	signal sig_intr_done: std_logic;
	signal sig_dma_irq: std_logic;
	signal sig_consumer_start: std_logic;
	
	attribute MARK_DEBUG : string;
	attribute MARK_DEBUG of sig_in_trans_valid : signal is "TRUE";
	attribute MARK_DEBUG of sig_dma_irq : signal is "TRUE";
	attribute MARK_DEBUG of sig_itab_empty: signal is "TRUE";
	attribute MARK_DEBUG of sig_rdbuff_empty: signal is "TRUE";
	
	-- component declaration
	component odev_v1_0_S00_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 5
		);
		port (
		TRIG_G_START : out std_logic;
		S_G_PULSE : out std_logic;
		S_STREAM_START: out std_logic;
		S_SRC_ADDR : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_SRC_LEN : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_ITAB_FULL : in std_logic;
		S_IN_TRANS_VALID: out std_logic;
		S_IN_TRANS_DONE: in std_logic;
		S_CONSUME_LATENCY: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_INTR_DONE: out std_logic;
		S_ITAB_EMPTY: in std_logic;
		S_RDBUFF_ALMOST_FULL: in std_logic;
        S_RDBUFF_EMPTY: in std_logic;
        S_CONSUMER_START: out std_logic;
		-------------------------------------------
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic
		);
	end component odev_v1_0_S00_AXI;

	component odev_v1_0_M00_AXI is
		generic (
		C_M_TARGET_SLAVE_BASE_ADDR	: std_logic_vector	:= x"40000000";
		C_M_AXI_BURST_LEN	: integer	:= 16;
		C_M_AXI_ID_WIDTH	: integer	:= 1;
		C_M_AXI_ADDR_WIDTH	: integer	:= 32;
		C_M_AXI_DATA_WIDTH	: integer	:= 32
		);
		port (
--		M_G_START	: in std_logic;
--		M_G_PULSE : in std_logic;
		M_STREAM_START: in std_logic;
		M_SRC_ADDR : in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		M_SRC_LEN : in std_logic_vector(15 downto 0);
		M_ITAB_OUT_VALID: in std_logic;
		M_ITAB_OUT_TRANS_REQ : out std_logic;
		M_ITAB_EMPTY : in std_logic;
		M_RDATA: out std_logic_vector(31 downto 0);
        M_RDATA_VALID: out std_logic;
        M_RDBUFF_AL_FULL: in std_logic;
		M_AXI_ACLK	: in std_logic;
		M_AXI_ARESETN	: in std_logic;
		M_AXI_AWID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_AWADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		M_AXI_AWLEN	: out std_logic_vector(7 downto 0);
		M_AXI_AWSIZE	: out std_logic_vector(2 downto 0);
		M_AXI_AWBURST	: out std_logic_vector(1 downto 0);
		M_AXI_AWLOCK	: out std_logic;
		M_AXI_AWCACHE	: out std_logic_vector(3 downto 0);
		M_AXI_AWPROT	: out std_logic_vector(2 downto 0);
		M_AXI_AWQOS	: out std_logic_vector(3 downto 0);
		M_AXI_AWVALID	: out std_logic;
		M_AXI_AWREADY	: in std_logic;
		M_AXI_WDATA	: out std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		M_AXI_WSTRB	: out std_logic_vector(C_M_AXI_DATA_WIDTH/8-1 downto 0);
		M_AXI_WLAST	: out std_logic;
		M_AXI_WVALID	: out std_logic;
		M_AXI_WREADY	: in std_logic;
		M_AXI_BID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_BRESP	: in std_logic_vector(1 downto 0);
		M_AXI_BVALID	: in std_logic;
		M_AXI_BREADY	: out std_logic;
		M_AXI_ARID	: out std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_ARADDR	: out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
		M_AXI_ARLEN	: out std_logic_vector(7 downto 0);
		M_AXI_ARSIZE	: out std_logic_vector(2 downto 0);
		M_AXI_ARBURST	: out std_logic_vector(1 downto 0);
		M_AXI_ARLOCK	: out std_logic;
		M_AXI_ARCACHE	: out std_logic_vector(3 downto 0);
		M_AXI_ARPROT	: out std_logic_vector(2 downto 0);
		M_AXI_ARQOS	: out std_logic_vector(3 downto 0);
		M_AXI_ARVALID	: out std_logic;
		M_AXI_ARREADY	: in std_logic;
		M_AXI_RID	: in std_logic_vector(C_M_AXI_ID_WIDTH-1 downto 0);
		M_AXI_RDATA	: in std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
		M_AXI_RRESP	: in std_logic_vector(1 downto 0);
		M_AXI_RLAST	: in std_logic;
		M_AXI_RVALID	: in std_logic;
		M_AXI_RREADY	: out std_logic
		);
	end component odev_v1_0_M00_AXI;

	component Itab is
        generic (
            Itab_entries: integer := 512
        );
        port (
            clk: in std_logic;
            ITAB_G_START: in std_logic;
            SRC_ADDR_IN: in std_logic_vector (31 downto 0);
            SRC_LEN_IN: in std_logic_vector (15 downto 0);
            ITAB_IN_TRANS_VALID: in std_logic;
            ITAB_IN_TRANS_DONE: out std_logic;
            SRC_ADDR_OUT: out std_logic_vector (31 downto 0);
            SRC_LEN_OUT: out std_logic_vector(15 downto 0);
            ITAB_OUT_TRANS_REQ: in std_logic;
            ITAB_FULL: out std_logic;
            ITAB_EMPTY: out std_logic;
			ITAB_OUT_VALID: out std_logic			
        );
    end component Itab;
    
    component RdBuff is
    port (
         CLK: in std_logic;
--         RDBUFF_G_START: in std_logic;
         RDBUFF_STREAM_START: in std_logic;
         RDATA: in std_logic_vector(31 downto 0);
         RDATA_VALID: in std_logic;
         RDBUFF_ALMOST_FULL: out std_logic;
         RDBUFF_EMPTY: out std_logic;
         OUTDATA: out std_logic_vector (31 downto 0);
         OUTVALID: out std_logic;
         OUTREQ: in std_logic         
    ); 
    end component RdBuff;
    
    component DataConsumer is
    Port (
        CLK: in std_logic;
--        RST: in std_logic;
        DATA_STREAM_START: in std_logic;
        DATA_IN: in std_logic_vector(31 downto 0);
        DATA_VALID: in std_logic;
        DATA_REQ: out std_logic;
        DATA_CONSUME_LATENCY: in std_logic_vector(31 downto 0);
        DATA_CONSUMER_START: in std_logic
    );
    end component DataConsumer;
begin

-- Instantiation of Axi Bus Interface S00_AXI
odev_v1_0_S00_AXI_inst : odev_v1_0_S00_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
	    TRIG_G_START => sig_g_start,
	    S_G_PULSE => sig_g_pulse,
	    S_STREAM_START => sig_stream_start,
	    S_SRC_ADDR => sig_src_addr,
	    S_SRC_LEN => sig_src_len,
	    S_ITAB_FULL => sig_itab_full,
	    S_IN_TRANS_VALID => sig_in_trans_valid,
	    S_IN_TRANS_DONE => sig_itab_in_trans_done,
	    S_CONSUME_LATENCY => sig_consume_latency,
	    S_INTR_DONE => sig_intr_done,
	    S_ITAB_EMPTY => sig_itab_empty,
	    S_RDBUFF_ALMOST_FULL => sig_rdbuff_almost_full,
        S_RDBUFF_EMPTY => sig_rdbuff_empty,
        S_CONSUMER_START => sig_consumer_start,
	    -----------------------------
		S_AXI_ACLK	=> s00_axi_aclk,
		S_AXI_ARESETN	=> s00_axi_aresetn,
		S_AXI_AWADDR	=> s00_axi_awaddr,
		S_AXI_AWPROT	=> s00_axi_awprot,
		S_AXI_AWVALID	=> s00_axi_awvalid,
		S_AXI_AWREADY	=> s00_axi_awready,
		S_AXI_WDATA	=> s00_axi_wdata,
		S_AXI_WSTRB	=> s00_axi_wstrb,
		S_AXI_WVALID	=> s00_axi_wvalid,
		S_AXI_WREADY	=> s00_axi_wready,
		S_AXI_BRESP	=> s00_axi_bresp,
		S_AXI_BVALID	=> s00_axi_bvalid,
		S_AXI_BREADY	=> s00_axi_bready,
		S_AXI_ARADDR	=> s00_axi_araddr,
		S_AXI_ARPROT	=> s00_axi_arprot,
		S_AXI_ARVALID	=> s00_axi_arvalid,
		S_AXI_ARREADY	=> s00_axi_arready,
		S_AXI_RDATA	=> s00_axi_rdata,
		S_AXI_RRESP	=> s00_axi_rresp,
		S_AXI_RVALID	=> s00_axi_rvalid,
		S_AXI_RREADY	=> s00_axi_rready
	);

-- Instantiation of Axi Bus Interface M00_AXI
odev_v1_0_M00_AXI_inst : odev_v1_0_M00_AXI
	generic map (
		C_M_TARGET_SLAVE_BASE_ADDR	=> C_M00_AXI_TARGET_SLAVE_BASE_ADDR,
		C_M_AXI_BURST_LEN	=> C_M00_AXI_BURST_LEN,
		C_M_AXI_ID_WIDTH	=> C_M00_AXI_ID_WIDTH,
		C_M_AXI_ADDR_WIDTH	=> C_M00_AXI_ADDR_WIDTH,
		C_M_AXI_DATA_WIDTH	=> C_M00_AXI_DATA_WIDTH
	)
	port map (
--		M_G_START => sig_g_start,
--		M_G_PULSE => sig_g_pulse,
		M_STREAM_START => sig_stream_start,
		M_SRC_ADDR => sig_itab_src_addr,
		M_SRC_LEN => sig_itab_src_len,
		M_ITAB_OUT_VALID => sig_itab_out_valid,
		M_ITAB_OUT_TRANS_REQ => sig_itab_out_trans_req,
		M_ITAB_EMPTY => sig_itab_empty,
		M_RDATA => sig_rdata,
        M_RDATA_VALID => sig_rdata_valid,
        M_RDBUFF_AL_FULL => sig_rdbuff_almost_full,
		M_AXI_ACLK	=> m00_axi_aclk,
		M_AXI_ARESETN	=> m00_axi_aresetn,
		M_AXI_AWID	=> m00_axi_awid,
		M_AXI_AWADDR	=> m00_axi_awaddr,
		M_AXI_AWLEN	=> m00_axi_awlen,
		M_AXI_AWSIZE	=> m00_axi_awsize,
		M_AXI_AWBURST	=> m00_axi_awburst,
		M_AXI_AWLOCK	=> m00_axi_awlock,
		M_AXI_AWCACHE	=> m00_axi_awcache,
		M_AXI_AWPROT	=> m00_axi_awprot,
		M_AXI_AWQOS	=> m00_axi_awqos,
		M_AXI_AWVALID	=> m00_axi_awvalid,
		M_AXI_AWREADY	=> m00_axi_awready,
		M_AXI_WDATA	=> m00_axi_wdata,
		M_AXI_WSTRB	=> m00_axi_wstrb,
		M_AXI_WLAST	=> m00_axi_wlast,
		M_AXI_WVALID	=> m00_axi_wvalid,
		M_AXI_WREADY	=> m00_axi_wready,
		M_AXI_BID	=> m00_axi_bid,
		M_AXI_BRESP	=> m00_axi_bresp,
		M_AXI_BVALID	=> m00_axi_bvalid,
		M_AXI_BREADY	=> m00_axi_bready,
		M_AXI_ARID	=> m00_axi_arid,
		M_AXI_ARADDR	=> m00_axi_araddr,
		M_AXI_ARLEN	=> m00_axi_arlen,
		M_AXI_ARSIZE	=> m00_axi_arsize,
		M_AXI_ARBURST	=> m00_axi_arburst,
		M_AXI_ARLOCK	=> m00_axi_arlock,
		M_AXI_ARCACHE	=> m00_axi_arcache,
		M_AXI_ARPROT	=> m00_axi_arprot,
		M_AXI_ARQOS	=> m00_axi_arqos,
		M_AXI_ARVALID	=> m00_axi_arvalid,
		M_AXI_ARREADY	=> m00_axi_arready,
		M_AXI_RID	=> m00_axi_rid,
		M_AXI_RDATA	=> m00_axi_rdata,
		M_AXI_RRESP	=> m00_axi_rresp,
		M_AXI_RLAST	=> m00_axi_rlast,
		M_AXI_RVALID	=> m00_axi_rvalid,
		M_AXI_RREADY	=> m00_axi_rready
	);

	-- Add user logic here

    Itab_inst: Itab
    generic map(
        Itab_entries => 512    
    )
    port map(
        clk => s00_axi_aclk,
        ITAB_G_START => sig_g_start,
        SRC_ADDR_IN => sig_src_addr,
        SRC_LEN_IN => sig_src_len (15 downto 0),
        ITAB_IN_TRANS_VALID => sig_in_trans_valid,
        ITAB_IN_TRANS_DONE => sig_itab_in_trans_done,
        SRC_ADDR_OUT => sig_itab_src_addr,
        SRC_LEN_OUT => sig_itab_src_len,
        ITAB_OUT_TRANS_REQ => sig_itab_out_trans_req,
        ITAB_FULL => sig_Itab_full,
        ITAB_EMPTY => sig_itab_empty,
		ITAB_OUT_VALID => sig_itab_out_valid
    );
    
    RdBuff_inst: RdBuff 
    port map (
         CLK => s00_axi_aclk,
--         RDBUFF_G_START => sig_g_start,
         RDBUFF_STREAM_START => sig_stream_start,
         RDATA => sig_rdata,
         RDATA_VALID => sig_rdata_valid,
         RDBUFF_ALMOST_FULL => sig_rdbuff_almost_full,
         RDBUFF_EMPTY => sig_rdbuff_empty,
         OUTDATA => sig_outdata,
         OUTVALID => sig_outvalid,
         OUTREQ => sig_outreq
    );
    
    DataConsumer_inst: DataConsumer
    Port map (
        CLK => s00_axi_aclk,
--        RST => sig_g_start,
        DATA_STREAM_START => sig_stream_start,
        DATA_IN => sig_outdata,
        DATA_VALID => sig_outvalid,
        DATA_REQ => sig_outreq,
        DATA_CONSUME_LATENCY => sig_consume_latency,
        DATA_CONSUMER_START => sig_consumer_start
    );
    
    SW_DMA_IRQ <= sig_dma_irq;
    process (s00_axi_aclk)
    begin
        if (rising_edge(s00_axi_aclk)) then
            if (sig_stream_start = '1' AND sig_consumer_start = '1') then
--                if (sig_itab_empty = '1' OR sig_rdbuff_empty = '1') then
                if (sig_rdbuff_empty = '1') then -- only rdbuff empty is a hazard
                    if (sig_dma_irq = '0') then 
                        sig_dma_irq <= '1';
                    elsif (sig_intr_done = '1') then
                        sig_dma_irq <= '0';
                    else
                        sig_dma_irq <= sig_dma_irq;
                    end if;
                else 
                    sig_dma_irq <= sig_dma_irq;
                end if;
            else
                sig_dma_irq <= '0';
            end if;
        end if;
    end process;
	-- User logic ends

end arch_imp;