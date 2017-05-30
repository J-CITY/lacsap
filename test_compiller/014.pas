program example_arr;

const
   n = 10;
var
   a: array[1..5, 1..10] of integer;
begin
	a[2, 2] := 3;
	write(a[2,2]);
end.
