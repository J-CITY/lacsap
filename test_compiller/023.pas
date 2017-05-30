program p;
var
    i:integer;

procedure print(a, b : integer);
var
	arr: array[1..10] of real;
begin
	arr[5] := 3.1415;
	write(arr[5], ' ');
	write(a, ' ', b);
end;

begin
	i := 10;
	print(i, i);
end.
