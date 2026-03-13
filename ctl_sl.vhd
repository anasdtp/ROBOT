library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--==============================================================
-- MODULE: ctl_sl (SL = Suivi de Ligne / Line Following)
-- Contrôleur PID proportionnel (P-only)
-- Convertit position de ligne en correction de moteur
--
-- Équation: output = KP * position_sig
-- KP = 900
-- position_sig: -3 à +3
-- output: -2700 à +2700
--==============================================================

entity ctl_sl is
    port (
        clk         : in  std_logic;
        reset_n     : in  std_logic;
        
        -- Input from position_ligne module
        position_in : in  std_logic_vector(3 downto 0);  -- Signed -3 to +3
        ready       : in  std_logic;                     -- Data valid flag
        
        -- Output correction for motors
        -- Positive = turn right (reduce left motor speed / increase right motor speed)
        -- Negative = turn left  (reduce right motor speed / increase left motor speed)
        correction  : out std_logic_vector(13 downto 0)  -- Signed correction value
    );
end ctl_sl;

architecture rtl of ctl_sl is
    
    constant KP : integer := 900;
    
    signal position_signed : signed(3 downto 0);
    signal correction_temp : signed(13 downto 0);
    signal correction_out  : signed(13 downto 0);
    
begin
    
    -- Convert input to signed
    position_signed <= signed(position_in);
    
    -- P-only calculation: correction = KP * position
    -- Since KP = 900 and position is -3 to +3:
    -- max output = 900 * 3 = 2700 (fits in 14-bit signed: -8192 to +8191)
    correction_temp <= resize(position_signed * KP, 14);
    
    -- Register the output
    process(clk, reset_n)
    begin
        if reset_n = '0' then
            correction_out <= to_signed(0, 14);
        elsif rising_edge(clk) then
            if ready = '1' then
                correction_out <= correction_temp;
            end if;
        end if;
    end process;
    
    -- Convert back to std_logic_vector
    correction <= std_logic_vector(correction_out);
    
end rtl;
