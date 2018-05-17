% ConduinoMultichannel.m
%
% 1. With Matlab closed, plug in the Conduino;
% 2. Open Matlab;
% 3. Run this script.
%
% Marco Carminati, 2016
% 
% Includes edits from: Paolo Luzzatto-Fegiz, Pierre-Yves Passaggia.
% Last updated: May 17, 2018

clear
close all
delete(instrfindall); % Use this if the ports get stuck

%%%%%%%%%%%%%%%%%%%%%%%%%%%% Settings %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
macORpc = 'pc'; % 'mac' or 'pc' 
portNumber = 6; % COM port number on pc, port index on mac
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%assigns the object s to serial port
switch macORpc
    case 'pc'
        s1 = serial(['COM' num2str(portNumber)]); % for PC
    otherwise
        s = seriallist;
        disp(['Using port ' s(portNumber) ])
        s1 = serial(s(portNumber)); % for mac
end

set(s1, 'InputBufferSize', 128); %number of bytes in inout buffer
set(s1, 'BaudRate', 115200);
set(s1, 'Parity', 'none');
fopen(s1);
clc
prompt = 'How many samples? (dt ~40 ms typically) ';
nSamples=input(prompt,'s');
fprintf(s1,'%c',nSamples);
nSamples = str2double(nSamples);
ch1=zeros(0,nSamples);
ch2=zeros(0,nSamples);
ch3=zeros(0,nSamples);
ch4=zeros(0,nSamples);

tic
fwrite(s1, '%f'); 
for nSample=1:nSamples
    ch1(nSample)= fscanf(s1, '%f');
    ch2(nSample)= fscanf(s1, '%f');
    ch3(nSample)= fscanf(s1, '%f');
    ch4(nSample)= fscanf(s1, '%f');
end
timeElapsed = toc;
fclose(s1);

dt = timeElapsed/(nSamples-1);
disp(['Actual dt = ' num2str(round(dt*1e3)) ' ms'])

plot(ch1);
hold on;
plot(ch2);
plot(ch3);
plot(ch4);
grid on
xlabel('Sample')
ylabel('Reading')


figure
time = 0:dt:(nSamples-1)*dt;
plot(time,ch1);
hold on;
plot(time,ch2);
plot(time,ch3);
plot(time,ch4);
grid on
xlabel('Time, (s)')
ylabel('Reading')
legend('ch1', 'ch2', 'ch3', 'ch4')
pp=axis;
axis([pp(1:3) ceil(pp(4)+.1)])


