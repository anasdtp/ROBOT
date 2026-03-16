-- Motor Correction Module
-- Applique la correction PID aux commandes moteur
-- right_speed = BASE_SPEED + correction
-- left_speed = BASE_SPEED - correction

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY MUX IS
GENERIC (
	BASE_SPEED : integer := 1100  -- Vitesse de base des moteurs (à ajuster)
);
PORT (
	clk : IN STD_LOGIC;
	reset_n : IN STD_LOGIC;
	
	-- Motor commands from Qsys
	motor_right_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	
	-- Correction from PID controller
	correction_in : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
	ready : IN STD_LOGIC;
	
	-- Corrected motor outputs
	motor_right_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0);
	motor_left_out : OUT STD_LOGIC_VECTOR(13 DOWNTO 0)
);
END MUX;

ARCHITECTURE rtl OF MUX IS

signal correction_signed : signed(13 downto 0);
signal motor_right_signed : signed(13 downto 0);
signal motor_left_signed : signed(13 downto 0);
signal right_corrected : signed(13 downto 0);
signal left_corrected : signed(13 downto 0);

BEGIN

correction_signed <= signed(correction_in);
motor_right_signed <= signed(motor_right_in);
motor_left_signed <= signed(motor_left_in);

-- Apply correction: 
-- right = BASE_SPEED + correction
-- left = BASE_SPEED - correction
process(clk, reset_n)
begin
	if reset_n = '0' then
		motor_right_out <= (others => '0');
		motor_left_out <= (others => '0');
	elsif rising_edge(clk) then
		if ready = '1' then
			-- Right motor: add correction
			right_corrected <= motor_right_signed + correction_signed;
			-- Left motor: subtract correction
			left_corrected <= motor_left_signed - correction_signed;
			
			-- Output with saturation
			if right_corrected > to_signed(8191, 14) then
				motor_right_out <= std_logic_vector(to_signed(8191, 14));
			elsif right_corrected < to_signed(-8192, 14) then
				motor_right_out <= std_logic_vector(to_signed(-8192, 14));
			else
				motor_right_out <= std_logic_vector(right_corrected);
			end if;
			
			if left_corrected > to_signed(8191, 14) then
				motor_left_out <= std_logic_vector(to_signed(8191, 14));
			elsif left_corrected < to_signed(-8192, 14) then
				motor_left_out <= std_logic_vector(to_signed(-8192, 14));
			else
				motor_left_out <= std_logic_vector(left_corrected);
			end if;
		end if;
	end if;
end process;

END rtl;
