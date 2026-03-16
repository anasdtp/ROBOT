-- Implements a simple Nios II system for the DE-series board.
-- Inputs: SW7-0 are parallel port inputs to the Nios II system.
-- CLOCK_50 is the system clock.
-- KEY0 is the active-low system reset.
-- Outputs: LED7-0 are parallel port outputs from the Nios II system.
-- SDRAM ports correspond to the SDRAM signals on the DE-series board.
LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY ROBOT IS
PORT (
	SW : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	KEY : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
	CLOCK_50 : IN STD_LOGIC;
	LED : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	DRAM_CLK, DRAM_CKE : OUT STD_LOGIC;
	DRAM_ADDR : OUT STD_LOGIC_VECTOR(12 DOWNTO 0);
	DRAM_BA : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
	DRAM_CS_N, DRAM_CAS_N, DRAM_RAS_N, DRAM_WE_N : OUT STD_LOGIC;
	DRAM_DQ : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	DRAM_DQM : OUT STD_LOGIC_VECTOR(1 DOWNTO 0); 
	-- Motor Driver DRV8848 interface
	MTRR_P, MTRR_N : OUT STD_LOGIC;
	MTRL_P, MTRL_N : OUT STD_LOGIC;
	MTR_Sleep_n : OUT STD_LOGIC;
	MTR_Fault_n : IN STD_LOGIC;
	-- Sensor power control (A-Cute Car)
	VCC3P3_PWRON_n : OUT STD_LOGIC;
	IR_LED_ON : OUT STD_LOGIC;
	-- ADC LTC2308 interface
	LTC_ADC_CONVST : OUT STD_LOGIC;
	LTC_ADC_SCK : OUT STD_LOGIC;
	LTC_ADC_SDI : OUT STD_LOGIC;
	LTC_ADC_SDO : IN STD_LOGIC );
END ROBOT;

ARCHITECTURE Structure OF ROBOT IS

COMPONENT nios_system_sdram
PORT (
	clk_clk : IN STD_LOGIC;
	reset_reset_n : IN STD_LOGIC;
	sdram_clk_clk : OUT STD_LOGIC;
	--leds_export : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	switches_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sdram_wire_addr : OUT STD_LOGIC_VECTOR(12 DOWNTO 0);
	sdram_wire_ba : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
	sdram_wire_cas_n : OUT STD_LOGIC;
	sdram_wire_cke : OUT STD_LOGIC;
	sdram_wire_cs_n : OUT STD_LOGIC;
	sdram_wire_dq : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
	sdram_wire_dqm : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
	sdram_wire_ras_n : OUT STD_LOGIC;
	sdram_wire_we_n : OUT STD_LOGIC;
	motor_right_export : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_export : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	-- Ground sensors PIOs
	sensor_control_export : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_status_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data0_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data1_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data2_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data3_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data4_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data5_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor_data6_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	kp_export : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
	kd_export : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
	start_sl_export : OUT STD_LOGIC;
	fin_sl_export : IN STD_LOGIC;
	start_rot_export : OUT STD_LOGIC;
	dir_rot_export : OUT STD_LOGIC;
	fin_rot_export : IN STD_LOGIC;
	base_speed_export : OUT STD_LOGIC_VECTOR(11 DOWNTO 0) );
END COMPONENT;

COMPONENT capteurs_sol
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	data_capture : IN STD_LOGIC;
	data_readyr : OUT STD_LOGIC;
	data0r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data1r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data2r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data3r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data4r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data5r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	data6r : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
	ADC_CONVSTr : OUT STD_LOGIC;
	ADC_SCK : OUT STD_LOGIC;
	ADC_SDIr : OUT STD_LOGIC;
	ADC_SDO : IN STD_LOGIC );
END COMPONENT;

COMPONENT PWM_generation
PORT (
	clk, reset_n : IN STD_LOGIC;
	s_writedataR, s_writedataL : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	dc_motor_p_R, dc_motor_n_R : OUT STD_LOGIC;
	dc_motor_p_L, dc_motor_n_L : OUT STD_LOGIC );
END COMPONENT;

COMPONENT position_ligne
PORT (
	clk         : IN STD_LOGIC;
	reset_n     : IN STD_LOGIC;
	sensor_ready: IN STD_LOGIC;
	sensor0     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor1     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor2     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor3     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor4     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor5     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	sensor6     : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
	position    : OUT STD_LOGIC_VECTOR(3 DOWNTO 0);
	line_lost   : OUT STD_LOGIC );
END COMPONENT;

COMPONENT ctl_sl
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	position_in : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
	ready : IN STD_LOGIC;
	start_sl : IN STD_LOGIC;
	line_lost : IN STD_LOGIC;
	correction : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	fin_sl : OUT STD_LOGIC;
	kp_in : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
	kd_in : IN STD_LOGIC_VECTOR(11 DOWNTO 0) );
END COMPONENT;

COMPONENT ctl_rot
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	start_rot : IN STD_LOGIC;
	dir_rot : IN STD_LOGIC;
	position_in : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
	line_lost : IN STD_LOGIC;
	ready : IN STD_LOGIC;
	fin_rot : OUT STD_LOGIC );
END COMPONENT;

COMPONENT MUX
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	motor_right_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	correction_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	base_speed_in : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
	start_sl : IN STD_LOGIC;
	fin_sl : IN STD_LOGIC;
	start_rot : IN STD_LOGIC;
	dir_rot : IN STD_LOGIC;
	fin_rot : IN STD_LOGIC;
	ready : IN STD_LOGIC;
	motor_right_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0) );
END COMPONENT;

-- Internal signals for motor control
signal motor_right_data : STD_LOGIC_VECTOR(13 DOWNTO 0);
signal motor_left_data : STD_LOGIC_VECTOR(13 DOWNTO 0);
signal motor_right_corrected : STD_LOGIC_VECTOR(13 DOWNTO 0);
signal motor_left_corrected : STD_LOGIC_VECTOR(13 DOWNTO 0);

-- Internal signals for ground sensors
-- sensor_control_sig is output from Nios II but not used in this version (reserved for future use)
signal sensor_status_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data_capture_sig : STD_LOGIC;
signal data_ready_sig : STD_LOGIC;
signal data0_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data1_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data2_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data3_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data4_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data5_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);
signal data6_sig : STD_LOGIC_VECTOR(7 DOWNTO 0);

-- Signals from position_ligne module
signal position_sig : STD_LOGIC_VECTOR(3 DOWNTO 0);
signal line_lost_sig : STD_LOGIC;

-- Signals from ctl_sl (PID controller)
signal correction_sig : STD_LOGIC_VECTOR(13 DOWNTO 0);
signal kp_sig : STD_LOGIC_VECTOR(11 DOWNTO 0);
signal kd_sig : STD_LOGIC_VECTOR(11 DOWNTO 0);
signal start_sl_sig : STD_LOGIC;
signal fin_sl_sig : STD_LOGIC;
signal start_rot_sig : STD_LOGIC;
signal dir_rot_sig : STD_LOGIC;
signal fin_rot_sig : STD_LOGIC;
signal base_speed_sig : STD_LOGIC_VECTOR(11 DOWNTO 0);

-- Clock divider for 2 KHz data_capture signal
-- 50 MHz / 2 KHz = 25000
signal clk_div_counter : INTEGER range 0 to 24999 := 0;
signal data_capture_2khz : STD_LOGIC := '0';

-- Sensor threshold constant
constant THRESHOLD : unsigned(7 downto 0) := to_unsigned(110, 8);

-- Instantiate the Nios II system entity generated by the Qsys tool.
BEGIN
NiosII: nios_system_sdram
PORT MAP (
	clk_clk => CLOCK_50,
	reset_reset_n => KEY(0),
	sdram_clk_clk => DRAM_CLK,
	--leds_export => LED,
	switches_export => SW,
	sdram_wire_addr => DRAM_ADDR,
	sdram_wire_ba => DRAM_BA,
	sdram_wire_cas_n => DRAM_CAS_N,
	sdram_wire_cke => DRAM_CKE,
	sdram_wire_cs_n => DRAM_CS_N,
	sdram_wire_dq => DRAM_DQ,
	sdram_wire_dqm => DRAM_DQM,
	sdram_wire_ras_n => DRAM_RAS_N,
	sdram_wire_we_n => DRAM_WE_N,
	motor_right_export => motor_right_data,
	motor_left_export => motor_left_data,
	-- Ground sensors - sensor_control_export not used (reserved for future expansion)
	sensor_status_export => sensor_status_sig,
	sensor_data0_export => data0_sig,
	sensor_data1_export => data1_sig,
	sensor_data2_export => data2_sig,
	sensor_data3_export => data3_sig,
	sensor_data4_export => data4_sig,
	sensor_data5_export => data5_sig,
	sensor_data6_export => data6_sig,
	kp_export => kp_sig,
	kd_export => kd_sig,
	start_sl_export => start_sl_sig,
	fin_sl_export => fin_sl_sig,
	start_rot_export => start_rot_sig,
	dir_rot_export => dir_rot_sig,
	fin_rot_export => fin_rot_sig,
	base_speed_export => base_speed_sig );

-- Extract control and status signals
sensor_status_sig <= "0000000" & data_ready_sig;

-- Generate 2 KHz pulse for data_capture
process(CLOCK_50, KEY(0))
begin
	if KEY(0) = '0' then
		clk_div_counter <= 0;
		data_capture_2khz <= '0';
	elsif rising_edge(CLOCK_50) then
		if clk_div_counter = 24999 then
			clk_div_counter <= 0;
			data_capture_2khz <= NOT data_capture_2khz; -- Toggle at 2 KHz
		else
			clk_div_counter <= clk_div_counter + 1;
		end if;
	end if;
end process;

-- Use 2 KHz signal for data capture
data_capture_sig <= data_capture_2khz;

Capteurs: capteurs_sol
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	data_capture => data_capture_sig,
	data_readyr => data_ready_sig,
	data0r => data0_sig,
	data1r => data1_sig,
	data2r => data2_sig,
	data3r => data3_sig,
	data4r => data4_sig,
	data5r => data5_sig,
	data6r => data6_sig,
	ADC_CONVSTr => LTC_ADC_CONVST,
	ADC_SCK => LTC_ADC_SCK,
	ADC_SDIr => LTC_ADC_SDI,
	ADC_SDO => LTC_ADC_SDO );


-- Instantiate position_ligne module
Position: position_ligne
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	sensor_ready => data_ready_sig,
	sensor0 => data0_sig,
	sensor1 => data1_sig,
	sensor2 => data2_sig,
	sensor3 => data3_sig,
	sensor4 => data4_sig,
	sensor5 => data5_sig,
	sensor6 => data6_sig,
	position => position_sig,
	line_lost => line_lost_sig );


-- Instantiate PID controller for line following
PID_Controller: ctl_sl
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	position_in => position_sig,
	ready => data_ready_sig,
	start_sl => start_sl_sig,
	line_lost => line_lost_sig,
	correction => correction_sig,
	fin_sl => fin_sl_sig,
	kp_in => kp_sig,
	kd_in => kd_sig );

-- Instantiate rotation controller
Rotation_Controller: ctl_rot
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	start_rot => start_rot_sig,
	dir_rot => dir_rot_sig,
	position_in => position_sig,
	line_lost => line_lost_sig,
	ready => data_ready_sig,
	fin_rot => fin_rot_sig );

-- Instantiate MUX module (apply PID to motor speeds)
Mux_Inst: MUX
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	motor_right_in => motor_right_data,
	motor_left_in => motor_left_data,
	correction_in => correction_sig,
	base_speed_in => base_speed_sig,
	start_sl => start_sl_sig,
	fin_sl => fin_sl_sig,
	start_rot => start_rot_sig,
	dir_rot => dir_rot_sig,
	fin_rot => fin_rot_sig,
	ready => data_ready_sig,
	motor_right_out => motor_right_corrected,
	motor_left_out => motor_left_corrected );

-- Instantiate PWM generation for motor control
PWM_Motors: PWM_generation
PORT MAP (
	clk => CLOCK_50,
	reset_n => KEY(0),
	s_writedataR => motor_right_corrected,
	s_writedataL => motor_left_corrected,
	dc_motor_p_R => MTRR_P,
	dc_motor_n_R => MTRR_N,
	dc_motor_p_L => MTRL_P,
	dc_motor_n_L => MTRL_N );

-- Motor driver control signals
MTR_Sleep_n <= '1'; -- Enable motor driver (active high)
-- MTR_Fault_n is monitored but not used in current implementation

-- Sensor power control (A-Cute Car)
VCC3P3_PWRON_n <= '0'; -- Enable 3.3V power for sensors (active low)
IR_LED_ON <= '1'; -- Enable IR LEDs for line sensors

--==============================================================
-- DEBUG: Affichage direct des 7 capteurs sur les LEDs
-- LED[i] = '1' si capteur[i] > THRESHOLD (110)
--==============================================================
-- LED(0) <= '1' when unsigned(data0_sig) > THRESHOLD else '0';  -- LED0 = capteur0
-- LED(1) <= '1' when unsigned(data1_sig) > THRESHOLD else '0';  -- LED1 = capteur1
-- LED(2) <= '1' when unsigned(data2_sig) > THRESHOLD else '0';  -- LED2 = capteur2
-- LED(3) <= '1' when unsigned(data3_sig) > THRESHOLD else '0';  -- LED3 = capteur3
-- LED(4) <= '1' when unsigned(data4_sig) > THRESHOLD else '0';  -- LED4 = capteur4
-- LED(5) <= '1' when unsigned(data5_sig) > THRESHOLD else '0';  -- LED5 = capteur5
-- LED(6) <= '1' when unsigned(data6_sig) > THRESHOLD else '0';  -- LED6 = capteur6
-- LED(7) <= line_lost_sig;                                      -- LED7 = line_lost flag

process(position_sig, start_sl_sig, fin_sl_sig)
	variable pos : signed(3 downto 0);
	variable led_next : std_logic_vector(7 downto 0);
begin
	pos := signed(position_sig);
	led_next := (others => '0');

	case pos is
		when to_signed(-3, 4) =>
			led_next := "00000001";  -- LED0 - Extrême gauche
		when to_signed(-2, 4) =>
			led_next := "00000010";  -- LED1 - Gauche
		when to_signed(-1, 4) =>
			led_next := "00000100";  -- LED2 - Légèrement gauche
		when to_signed(0, 4) =>
			led_next := "00001000";  -- LED3 - Centre
		when to_signed(1, 4) =>
			led_next := "00010000";  -- LED4 - Légèrement droite
		when to_signed(2, 4) =>
			led_next := "00100000";  -- LED5 - Droite
		when to_signed(3, 4) =>
			led_next := "01000000";  -- LED6 - Extrême droite
		when others =>
			led_next := "00000000";  -- Aucune LED
	end case;

	led_next(7) := start_sl_sig; -- Start SL
	led_next(6) := fin_sl_sig;   -- Fin SL
	LED <= led_next;
end process;

END Structure;