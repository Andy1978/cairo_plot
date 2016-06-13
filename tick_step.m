function ret = tick_step (lim)

  f = 5;
  ret = f .** ceil (log (diff (lim) / 10) / log(f));

endfunction
