
if (0)
  x = linspace (-2, 4, 200);
  y = 10 .** x;

  #plot (x, y)

  [s5, s2] = tick_step (y);
  #plot (x, s)

  plot(y./s5, "-o", y./s2, "-o")
endif


## forward
f = @(n) (mod(n, 3).^2 + 1) .* 10.^floor(n/3);
#n = -3:20;
n = 0:20;
b = f(n);

##reverse
r = @(b) 3 * log10 (b);

figure (1)
semilogy (n, b, "-o", r(b), b, "-x")



###########
function ret = tick_step_new (range)

  n = round (3 * log10 (range / 7))
  ret = (mod(n, 3).^2 + 1) .* 10.^floor(n/3);

  printf ("p1=%f p2=%f\n", (mod(n, 3).^2 + 1), 10.^floor(n/3));
endfunction

figure (2)
x = linspace (-2, 4, 200);
y = 10 .** x;

s = tick_step_new (y);

plot (y./s)

