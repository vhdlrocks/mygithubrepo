function []=curvefit()

%>>> Data from each of the Hall Sensors
% Example ...
data=[ 0 0;... %Sensor No.1 is at 0 deg orientation to the handle
 15 0;... % " No.2 " " 15 deg " " " "
 30 0;...
 45 1;...
 60 1;...
 75 1;...
 90 4;...
 105 8.8;...
 120 9;...
 135 7;...
 150 5;...
 165 1;...
 180 1;...
 195 1;...
 210 1;...
 225 1;...
 240 0;...
 255 0;...
 270 0;...
 285 0;...
 300 0;...
 315 0;...
 330 0;...
 345 0];
% | |---- amplitude
% |----------- location of sensor (24 sensor @ 15 deg spacing)
% Note:
% The next two steps could be done in the handle or the app
%
% Note: 
% The location will need to be adjusted by the IMU (the tilt of the
% handle). Like the MSAT our refernce is gravity. One needs to add the
% IMU angle to the phyical angles (as pictured). So if the handle is
% tilted 40 degrees counter clockwise
%
% data(:,1)=data(:,1) + (-40 deg)
%
% Sensor Key:
%
% 1
% 24 2
% 23 3
% 22 4
% 21 5
% 20 6
% 19 7
% 18 8
% 17 9
% 16 10
% 15 11
% 14 12
% 13
% |--------|
% | |
% | handle |
%
%>>> Step 0: Correct for handle tilt.
IMUangle=-40;
data(:,1)=data(:,1) + IMUangle;
% data =
%
% -40 0
% -25 0
% -10 0
% 5 1
% .

% .
% .
% 275 0
% 290 0
% 305 0
idf=find(data(:,1)<0); %Index of values less than zero.
%One could have used a for-loop as well if no
%built in function
data(idf,1)=data(idf,1)+360; %Adjust, now the data to oriented with respect to gravity
% data =
%
% 320 0
% 335 0
% 350 0
% 5 1
% 20 1
% .
% .
% .
% 275 0
% 290 0
% 305 0
%>>> Step 1: Find the maximum level. One could do this via a for-loop and
%compare to the next or use a max() command. I assume most languages have a
%maximum command.
[val,ind] = max(data(:,2)); %Maximum value and index
% val = 9
% ind = 9
%
% or
%
% data(ind,:)= [8, 9]
% | |---- amplitude
% |------- angle wrt gravity

%>>> Step 2: Need to use 5 datapoint from which to fit a parabola, the peak
%of which will be the reported angle in degrees.
%
%Note:
% Jeff could provide Josh with the 5 datapoint (corrected as was done up to this
% point) then Josh would plot the histogram, showing no value for the
% remaining angles. Or Jeff could pass raw data and the IMU angle from which
% Josh would correct and display.
dA=data(ind-2:1:ind+2,:); % 5 Datapoints
% dA =
%
% 50.0000 4.0000
% 65.0000 8.8000
% 80.0000 9.0000
% 95.0000 7.0000
% 110.0000 5.0000
%Note: I would now normalize the data to aid in plotting.
dA(:,2)=dA(:,2)/val;
%dA =
%
% 50.0000 0.4444
% 65.0000 0.9778
% 80.0000 1.0000
% 95.0000 0.7778
% 110.0000 0.5556
% Note: a parabola is a 2nd order polynomial
%
% Calculus ...
%
% A X = B
%

% -- -- -- -- -- --
% | n ?xi ?xi^2 | | a0 | | ?yi |
% | ?xi ?xi^2 ?xi^3 | | a1 | = | ?(xi yi) |
% | ?xi^2 ?xi^3 ?xi^4 | | a2 | | ?(xi^2 yi) |
% -- -- -- -- -- --
%
% ... variables ...
%
% -- -- -- -- -- --
% | A11 A12 A13 | | x0 | | B11 |
% | A21 A22 A23 | | x1 | = | B21 |
% | A31 A32 A33 | | x2 | | B31 |
% -- -- -- -- -- --
%>>> Calculate terms of above matricies
% Note: I assume programming language will have a "sum" command, if not one
% can use a for-loop.
n=5; %Number of data points
A11=n;
A12=sum(dA(:,1));
A13=sum(dA(:,1).^2);
A21=A12;
A22=A13;
A23=sum(dA(:,1).^3);
A31=A22;
A32=A23;
A33=sum(dA(:,1).^4);
B11=sum(dA(:,2));
B21=sum(dA(:,1).*dA(:,2));
B31=sum(dA(:,1).^2.*dA(:,2));
A=[A11 A12 A13;...
    
 A21 A22 A23;...
 A31 A32 A33];
B=[B11;...
 B21;...
 B31];
% Use matrix inversion to solve for coefficents
% Y = inv(X) computes the inverse of square matrix X.
% X^(-1) is equivalent to inv(X).
X=[0;0;0]; %Preallocate
% X =
%
% 0
% 0
% 0
X=inv(A)*B; %maxtix
% Note: This is matrix multiplication. C = A*B is the matrix product of A and B.
% If A is an m-by-p and B is a p-by-n matrix C(i,j) is the inner product
% of the ith row of A with the jth column of B.
% Results:
% X =
%
% -2.5768 -> X1
% 0.0893 -> X2
% -0.0006 -> X3
%
% Curve Fit is
% y(x)= X1 + X2*x + X3*x^2
% y(x)= -2.5768 + 0.0893*x - -0.0006*x^2
% Construct the fitted curve
xx=0:0.1:360; %X-axis data range (angle)

yx=X(1) + X(2).*xx + X(3).*xx.^2; %Y-axis data
% Find the peak
[vP,iP] = max(yx); %Same as shown above
aMax=xx(iP); %This is the angle of the magnet ... All done.
%Let's plot to verify.
%>>> Step 3: Plot Like the App Screen
%Data
 hB=bar(dA(:,1),dA(:,2)); hold on;
 set(hB','BarWidth',0.5);
 set(gca,'Xgrid','on',...
'Ygrid','on',...
'Xlim',[0,345],...
'XTick',linspace(0,345,24),...
'YLim',[0,1.25],...
'YTick',linspace(0,1.25,11),...
'FontSize',8);
%Curve Fit
 plot(xx,yx);
%Peak Angle
 plot(aMax,vP,'LineStyle','none','Marker','o','MarkerSize',6,'MarkerFaceColor',[1 0 0],'MarkerEdgeColor',[1 1 1]);
%Label
 title(sprintf('Magnet Location @ %0.2f°',aMax));
 xlabel('Angle (°)');
 ylabel('Amplitude (--)');
