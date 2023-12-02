clear all, close all;
format long
fsamp = 8000;
fcuts = [1500 2000];
mags = [1 0];
devs = [0.05 0.01];

[n,Wn,beta,ftype] = kaiserord(fcuts,mags,devs,fsamp);
hh = fir1(n,Wn,ftype,kaiser(n+1,beta),'noscale');

freqz(hh,1,1024,fsamp);
t_hh = table(hh');
writetable(t_hh,'kaiser_coef.txt')


in = (rand(1,1024)-0.5)*2;
p = mean(in.^2);
in = in./p^0.5;
p = mean(in.^2);
t_in = table(in');
writetable(t_in,'input.txt')
figure
freqz(in,1,1024,fsamp);
out = conv(in,hh);
figure
freqz(out,1,1024,fsamp);
%freqz(conv(in,hh),1,1024,fsamp);