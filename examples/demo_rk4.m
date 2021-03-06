

xolotl.cleanup;
close all


% conversion from Prinz to phi
A = 0.0628;

channels = {'NaV','CaT','CaS','ACurrent','KCa','Kd','HCurrent'};
prefix = 'prinz/';
gbar(:,1) = [1000 25  60 500  50  1000 .1];
E =         [50   30  30 -80 -80 -80   -20];

x = xolotl;

x.add('compartment','AB','Cm',10,'A',A);

% add Calcium mechanism
x.AB.add('prinz/CalciumMech');

for i = 1:length(channels)
	x.AB.add([prefix channels{i}],'gbar',gbar(i),'E',E(i));
end

x.t_end = 1e4;
x.integrate;
x.t_end = 5e3;



x.closed_loop = false;
x.sim_dt = .1;
x.dt = .1;

figure('outerposition',[300 300 1200 600],'PaperUnits','points','PaperSize',[1200 600]); hold on
V = x.integrate;
plot(V,'k')

x.solver_order = 4;
[V, Ca] = x.integrate;
plot(V,'r')
