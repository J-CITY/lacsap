program main;
var
	i, j: ^integer;
  	r: ^real;
	rr:real;
	b:boolean;
	in, a, c:integer;
begin
	in := 13;
	i := @in;
	i^ := 2;
	write('1: ', i^,' ', i, ' ', in, #10);

	r := nil;
	write('4: ', ' ', r, #10);

end.
