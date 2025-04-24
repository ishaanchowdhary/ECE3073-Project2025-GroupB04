	PROJECT_SYS_V2 u0 (
		.cam_ready_export     (<connected-to-cam_ready_export>),     //     cam_ready.export
		.clk_clk              (<connected-to-clk_clk>),              //           clk.clk
		.first_hex_export     (<connected-to-first_hex_export>),     //     first_hex.export
		.gyro_int_1_export    (<connected-to-gyro_int_1_export>),    //    gyro_int_1.export
		.gyro_int_2_export    (<connected-to-gyro_int_2_export>),    //    gyro_int_2.export
		.keys_export          (<connected-to-keys_export>),          //          keys.export
		.leds_export          (<connected-to-leds_export>),          //          leds.export
		.pixel_address_export (<connected-to-pixel_address_export>), // pixel_address.export
		.pixel_data_export    (<connected-to-pixel_data_export>),    //    pixel_data.export
		.sdram_addr           (<connected-to-sdram_addr>),           //         sdram.addr
		.sdram_ba             (<connected-to-sdram_ba>),             //              .ba
		.sdram_cas_n          (<connected-to-sdram_cas_n>),          //              .cas_n
		.sdram_cke            (<connected-to-sdram_cke>),            //              .cke
		.sdram_cs_n           (<connected-to-sdram_cs_n>),           //              .cs_n
		.sdram_dq             (<connected-to-sdram_dq>),             //              .dq
		.sdram_dqm            (<connected-to-sdram_dqm>),            //              .dqm
		.sdram_ras_n          (<connected-to-sdram_ras_n>),          //              .ras_n
		.sdram_we_n           (<connected-to-sdram_we_n>),           //              .we_n
		.second_hex_export    (<connected-to-second_hex_export>),    //    second_hex.export
		.spi_external_MISO    (<connected-to-spi_external_MISO>),    //  spi_external.MISO
		.spi_external_MOSI    (<connected-to-spi_external_MOSI>),    //              .MOSI
		.spi_external_SCLK    (<connected-to-spi_external_SCLK>),    //              .SCLK
		.spi_external_SS_n    (<connected-to-spi_external_SS_n>),    //              .SS_n
		.switches_export      (<connected-to-switches_export>),      //      switches.export
		.time_display_export  (<connected-to-time_display_export>)   //  time_display.export
	);

