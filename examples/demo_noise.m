

vol = 0.0628; % this can be anything, doesn't matter

x = xolotl;
x.add('compartment','AB','A',0.0628);
x.AB.add('prinz/CalciumMech','f',1.496);

x.AB.add('liu/NaV','gbar',@() 115/x.AB.A,'E',30);
x.AB.add('liu/CaT','gbar',@() 1.44/x.AB.A,'E',30);
x.AB.add('liu/CaS','gbar',@() 1.7/x.AB.A,'E',30);
x.AB.add('liu/ACurrent','gbar',@() 15.45/x.AB.A);
x.AB.add('liu/KCa','gbar',@() 61.54/x.AB.A);
x.AB.add('liu/Kd','gbar',@() 38.31/x.AB.A);
x.AB.add('liu/HCurrent','gbar',@() .6343/x.AB.A);
x.AB.add('Leak','gbar',@() 0.0622/x.AB.A,'E',-50);

x.AB.add('goldwyn/CurrentNoise2','noise_amplitude',2);

x.sim_dt = .1;
x.dt = .1;

x.integrate;


x.t_end = 2e3;

x.snapshot('zero');

figure('outerposition',[300 300 1200 600],'PaperUnits','points','PaperSize',[1200 600]); hold on
V = x.integrate;
time = (1:length(V))*x.dt*1e-3;
plot(time,V,'k')
drawnow

x.reset('zero')
V = x.integrate;
plot(time,V,'r')
xlabel('Time (s)')
drawnow

x.reset('zero')
x.set('*noise_amplitude',0)
x.AB.HCurrent.add('SubunitNoise','noise_amplitude',.002);

V = x.integrate;
plot(time,V,'b')
drawnow
