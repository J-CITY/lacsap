program main;
var
	f:real;
	a : integer;
begin
f := (7 mod 4);
write(f, ' ');
f := (7 div 4);
write(f, ' ');

a := -2048;
write(a, ' ');
a := a shr 2;
write(a, ' ');
a := a or 15;
write(a, ' ');
a := a and 15;
write(a, ' ');

end.
