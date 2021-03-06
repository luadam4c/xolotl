%{ 
           _       _   _
__  _____ | | ___ | |_| |
\ \/ / _ \| |/ _ \| __| |
>  < (_) | | (_) | |_| |
/_/\_\___/|_|\___/ \__|_|

# manipulate

method that allows you to manipulate some or all parameters in a model hile visualizing its behaviour. 

Usage:

```
x.manipulate();
x.manipulate('some*pattern')
x.manipulate({'parameter1','parameter2'})
```

The simplest way to use ``manipulate`` is to simply call 
it with no arguments. By default, all the parameters are 
linked to sliders that you can play with. In models with 
a large number of parameters, this can get messy. You can 
selectively only manipualte some parameters whose names 
match a pattern using ``x.manipulate('some*pattern')``

!!! info "See Also"
    ->xolotl.plot

%}



function manipulate(self, manipulate_these)


if isempty(self.linked_binary)
	self.transpile;
	self.compile;
end

% disable closed loop
self.closed_loop = false;

% make a snapshot of this now
self.snapshot('manipulate_zero')


t_end = self.t_end;

compartment_names = self.find('compartment');
n = length(compartment_names);




if nargin < 2

	[values, ~, ~, real_names] = self.serialize;

	% skip some dynamical values
	rm_this = [lineFind(real_names,'*dt'); lineFind(real_names,'*.m'); lineFind(real_names,'*.h'); lineFind(real_names,'synapses*.s')];

	% manually remove all the V, Ca for each neuron
	for i = 1:length(real_names)
		for j = 1:n
			if strcmp(real_names{i}, [compartment_names{j} '.Ca'])
				rm_this = [rm_this; i];
			end
			if strcmp(real_names{i}, [compartment_names{j} '.V'])
				rm_this = [rm_this; i];
			end
		end
	end

	values(rm_this) = [];
	real_names(rm_this) = [];
	manipulate_these = real_names;

else

	if ~iscell(manipulate_these) && any(strfind(manipulate_these,'*'))
		% first find objects, then get them
		manipulate_these = self.find(manipulate_these);
	end

	real_names = manipulate_these;
	values = NaN*ones(length(real_names),1);
	% check that these exist
	for i = 1:length(real_names)
		assert(self.exist(real_names{i}),'Unknown parameter to manipulate')
		temp = self.get(real_names{i});
		if isa(temp,'function_handle')
			temp = temp();
		end
		values(i) = temp;
	end
end

assert(~isempty(manipulate_these),'Manipulate was called with illegal or invalid parameters that did not resolve to anything.')

% semi-intelligently make the upper and lower bounds
lb = values/3;
ub = values*3;

% create a puppeteer instance and configure
warning('off','MATLAB:hg:uicontrol:ValueMustBeInRange')
warning('off','MATLAB:hg:uicontrol:MinMustBeLessThanMax')
p = puppeteer(real_names,values,lb,ub,[],true);
self.handles.puppeteer_object = p;



for i = length(self.manipulate_plot_func):-1:1
	self.reset('manipulate_zero')
	self.manipulate_plot_func{i}(self);
end

warning('on','MATLAB:hg:uicontrol:MinMustBeLessThanMax')
warning('on','MATLAB:hg:uicontrol:ValueMustBeInRange')

p.callback_function = @self.manipulateEvaluate;


