


x = linspace (-2, 4, 200);
y = 10 .** x;

#plot (x, y)

[s5, s2] = tick_step (y);
#plot (x, s)

plot(y./s5, "-o", y./s2, "-o")
