-- Motor Correction Module
-- Applique la correction PID aux commandes moteur
-- right_speed = BASE_SPEED + correction
-- left_speed = BASE_SPEED - correction

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

ENTITY MUX IS
GENERIC (
	BASE_SPEED : integer := 1856  -- Vitesse de base des moteurs (à ajuster)
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

constant MAX_SPEED : integer := 4095; -- 12-bit PWM
signal correction_signed : signed(13 downto 0);

BEGIN

correction_signed <= signed(correction_in);

-- Apply correction around a forward base speed.
-- If position > 0 (line on right), slow right and speed up left.
process(clk, reset_n)
    variable corr : integer;
    variable right_speed : integer;
    variable left_speed : integer;
begin
    if reset_n = '0' then
        motor_right_out <= (others => '0');
        motor_left_out <= (others => '0');
    elsif rising_edge(clk) then
        if ready = '1' then
            corr := to_integer(correction_signed);
            right_speed := BASE_SPEED - corr;
            left_speed := BASE_SPEED + corr;

            if right_speed < 0 then
                right_speed := 0;
            elsif right_speed > MAX_SPEED then
                right_speed := MAX_SPEED;
            end if;

            if left_speed < 0 then
                left_speed := 0;
            elsif left_speed > MAX_SPEED then
                left_speed := MAX_SPEED;
            end if;

            motor_right_out <= '1' & '0' & std_logic_vector(to_unsigned(right_speed, 12));
            motor_left_out  <= '1' & '0' & std_logic_vector(to_unsigned(left_speed, 12));
        end if;
    end if;
end process;

END rtl;
