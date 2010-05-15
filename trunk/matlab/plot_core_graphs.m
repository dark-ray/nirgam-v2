function plot_core_graphs()

load ../log/matlab/core_latency_flit;
load ../log/matlab/core_latency_pkt;
load ../log/matlab/core_throughput;

%load core_latency_flit
figure(1);
h = bar(core_latency_flit, 1);
set(gca, 'fontweight', 'bold');
set(gca, 'xlim', [1 length(core_latency_flit)]);
title('Average router latency per flit','fontsize',12,'fontweight','bold');
xlabel('Routers ids (first one == 1)','fontsize',12,'fontweight','bold');

[zs, izs] = sortrows(core_latency_flit,1);
ch = get(h,'Children');
fvd = get(ch,'Faces');
fvcd = get(ch,'FaceVertexCData');

k = 128;                % Number of colors in color table
colormap(spring(k));    % Expand the previous colormap
shading interp          % Needed to graduate colors
n = length(core_latency_flit);  % Total bars
for i = 1:n
    color = floor(k*i/n);       % Interpolate a color index
    row = izs(i);               % Look up actual row # in data
    fvcd(fvd(row,1)) = 1;       % Color base vertices 1st index
    fvcd(fvd(row,4)) = 1;    
    fvcd(fvd(row,2)) = color;   % Assign top vertices color
    fvcd(fvd(row,3)) = color;
end
set(ch,'FaceVertexCData', fvcd);  % Apply the vertex coloring
set(ch,'EdgeColor','k')           % Give bars black borders
colorbar;

%load core_latency_pkt
figure(2);
h = bar(core_latency_pkt, 1);
set(gca, 'fontweight', 'bold');
set(gca, 'xlim', [1 length(core_latency_pkt)]);
title('Average router latency per packet','fontsize',12,'fontweight','bold');
xlabel('Routers ids (first one == 1)','fontsize',12,'fontweight','bold');

[zs, izs] = sortrows(core_latency_pkt,1);
ch = get(h,'Children');
fvd = get(ch,'Faces');
fvcd = get(ch,'FaceVertexCData');

k = 128;                % Number of colors in color table
colormap(spring(k));    % Expand the previous colormap
shading interp          % Needed to graduate colors
n = length(core_latency_pkt);  % Total bars
for i = 1:n
    color = floor(k*i/n);       % Interpolate a color index
    row = izs(i);               % Look up actual row # in data
    fvcd(fvd(row,1)) = 1;       % Color base vertices 1st index
    fvcd(fvd(row,4)) = 1;    
    fvcd(fvd(row,2)) = color;   % Assign top vertices color
    fvcd(fvd(row,3)) = color;
end
set(ch,'FaceVertexCData', fvcd);  % Apply the vertex coloring
set(ch,'EdgeColor','k')           % Give bars black borders
colorbar;

%load core_throughput
figure(3);
h = bar(core_throughput, 1);
set(gca, 'fontweight', 'bold');
set(gca, 'xlim', [1 length(core_throughput)]);
title('Router throughput (Gbps)','fontsize',12,'fontweight','bold');
xlabel('Routers ids (first one == 1)','fontsize',12,'fontweight','bold');

[zs, izs] = sortrows(core_throughput,1);
ch = get(h,'Children');
fvd = get(ch,'Faces');
fvcd = get(ch,'FaceVertexCData');

k = 128;                % Number of colors in color table
colormap(spring(k));    % Expand the previous colormap
shading interp          % Needed to graduate colors
n = length(core_throughput);  % Total bars
for i = 1:n
    color = floor(k*i/n);       % Interpolate a color index
    row = izs(i);               % Look up actual row # in data
    fvcd(fvd(row,1)) = 1;       % Color base vertices 1st index
    fvcd(fvd(row,4)) = 1;    
    fvcd(fvd(row,2)) = color;   % Assign top vertices color
    fvcd(fvd(row,3)) = color;
end
set(ch,'FaceVertexCData', fvcd);  % Apply the vertex coloring
set(ch,'EdgeColor','k')           % Give bars black borders
colorbar;

end

