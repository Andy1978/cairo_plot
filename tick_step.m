function [ret_5, ret_2] = tick_step (range)

  f = 5;
  ret_5 = f .** ceil (log (range / 10) / log(f));

  f = 2;
  ret_2 = f .** ceil (log (range / 10) / log(f));

endfunction
