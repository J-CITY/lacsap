program example_arr;

const
   n = 5;
type
   mas = array[1..n] of real;
var
   a: mas;
begin
	a[2] := 3.1415;
   write(a[2]);
end.
