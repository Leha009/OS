﻿writingPages = [1 16 16 16 16 16 0 0 0 0 0 2 16 16 16 16 16 16 16 16 16 0 0 0 0 0 0 0 0 0 5 16 16 16 16 16 16 16 15 0 0 0 0 0 0 0 0 13 16 16 16 16 16 16 16 16 13 0 0 0 0 0 0 0 0 0 8 16 16 16 16 16 16 4 0 0 0 0 0 11 16 16 16 16 16 16 16 16 16 16 16 6 0 0 0 0 0 0 0 0 0 0 0 11 16 16 16 16 16 16 16 16 10 3 0 0 0 0 0 0 0 0 9 13 16 16 16 16 16 16 9 4 0 0 0 0 0 0 4 11 15 16 16 16 16 16 16 16 16 16 16 16 16 6 2 0 0 0 0 0 0 0 0 0 0 0 0 9 13 16 16 16 16 16 16 16 9 5 0 0 0 0 0 0 0 0 0 0 0 ];
readingPages = [0 0 0 0 0 0 16 16 16 16 16 14 0 0 0 0 0 0 0 0 0 16 16 16 16 16 16 16 16 16 11 0 0 0 0 0 0 0 1 16 16 16 16 16 16 16 16 0 0 0 0 0 0 0 0 0 3 16 16 16 16 16 16 16 16 16 7 0 0 0 0 0 0 11 16 16 16 16 16 5 0 0 0 0 0 0 0 0 0 0 0 10 16 16 16 16 16 16 16 16 16 16 16 5 0 0 0 0 0 0 0 0 6 11 16 16 16 16 16 16 16 16 7 3 0 0 0 0 0 0 7 12 16 16 16 16 16 16 11 5 0 0 0 0 0 0 0 0 0 0 0 0 0 9 13 16 16 16 16 16 16 16 16 16 16 16 16 7 3 0 0 0 0 0 0 0 7 11 15 16 16 16 16 16 16 14 6 3 0 ];
time = [0 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000 2100 2200 2300 2400 2500 2600 2700 2800 2900 3000 3100 3200 3300 3400 3500 3600 3700 3800 3900 4000 4100 4200 4300 4400 4500 4600 4700 4800 4900 5000 5100 5200 5300 5400 5500 5600 5700 5800 5900 6000 6100 6200 6300 6400 6500 6600 6700 6800 6900 7000 7100 7200 7300 7400 7500 7600 7700 7800 7900 8000 8100 8200 8300 8400 8500 8600 8700 8800 8900 9000 9100 9200 9300 9400 9500 9600 9700 9800 9900 10000 10100 10200 10300 10400 10500 10600 10700 10800 10900 11000 11100 11200 11300 11400 11500 11600 11700 11800 11900 12000 12100 12200 12300 12400 12500 12600 12700 12800 12900 13000 13100 13200 13300 13400 13500 13600 13700 13800 13900 14000 14100 14200 14300 14400 14500 14600 14700 14800 14900 15000 15100 15200 15300 15400 15500 15600 15700 15800 15900 16000 16100 16200 16300 16400 16500 16600 16700 16800 16900 17000 17100 17200 17300 17400 17500 17600 17700 17800 17900 18000 18100 18200 18300 18400 18500 18600 18700 18800 ];

figure;
hold on;
plot(time, writingPages, "r")
plot(time, readingPages, "g")
xlabel("Время, мс")
hold off;