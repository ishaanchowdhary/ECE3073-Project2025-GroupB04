
module PROJECT_SYS_V2 (
	cam_ready_export,
	clk_clk,
	first_hex_export,
	gyro_int_export,
	keys_export,
	leds_export,
	pixel_address_export,
	pixel_data_export,
	sdram_addr,
	sdram_ba,
	sdram_cas_n,
	sdram_cke,
	sdram_cs_n,
	sdram_dq,
	sdram_dqm,
	sdram_ras_n,
	sdram_we_n,
	second_hex_export,
	spi_external_MISO,
	spi_external_MOSI,
	spi_external_SCLK,
	spi_external_SS_n,
	switches_export,
	time_display_export);	

	input		cam_ready_export;
	input		clk_clk;
	output	[23:0]	first_hex_export;
	input		gyro_int_export;
	input	[1:0]	keys_export;
	output	[9:0]	leds_export;
	output	[16:0]	pixel_address_export;
	output	[3:0]	pixel_data_export;
	output	[12:0]	sdram_addr;
	output	[1:0]	sdram_ba;
	output		sdram_cas_n;
	output		sdram_cke;
	output		sdram_cs_n;
	inout	[15:0]	sdram_dq;
	output	[1:0]	sdram_dqm;
	output		sdram_ras_n;
	output		sdram_we_n;
	output	[23:0]	second_hex_export;
	input		spi_external_MISO;
	output		spi_external_MOSI;
	output		spi_external_SCLK;
	output	[1:0]	spi_external_SS_n;
	input	[9:0]	switches_export;
	input	[31:0]	time_display_export;
endmodule
