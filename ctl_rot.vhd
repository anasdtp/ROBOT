library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--==============================================================
-- MODULE: ctl_rot (Rotation)
-- Tourne sur place jusqu'a retrouver la ligne au centre.
--==============================================================

entity ctl_rot is
    port (
        clk         : in  std_logic;
        reset_n     : in  std_logic;
        start_rot   : in  std_logic;
        dir_rot     : in  std_logic; -- 0=gauche, 1=droite
        position_in : in  std_logic_vector(3 downto 0);
        line_lost   : in  std_logic;
        ready       : in  std_logic;
        fin_rot     : out std_logic
    );
end ctl_rot;

architecture rtl of ctl_rot is

    type state_t is (IDLE, ROTATING);
    signal state : state_t;

begin

    process(clk, reset_n)
    begin
        if reset_n = '0' then
            state <= IDLE;
            fin_rot <= '0';
        elsif rising_edge(clk) then
            fin_rot <= '0';
            case state is
                when IDLE =>
                    if start_rot = '1' then
                        state <= ROTATING;
                    end if;
                when ROTATING =>
                    if ready = '1' then
                        if (line_lost = '0') and (signed(position_in) = to_signed(0, 4)) then
                            fin_rot <= '1';
                            state <= IDLE;
                        end if;
                    end if;
            end case;
        end if;
    end process;

end rtl;
