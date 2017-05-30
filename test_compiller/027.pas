program p;
var
    i:integer;
	arr: array [1..10] of integer;


procedure proc(var a : array [1..10] of integer);
begin
	a[5] := 10;
end;

begin
	for i := 1 to 10 do
		arr[i] := i;
	proc(arr);
	for i := 1 to 10 do
		write(arr[i], ' ');
end.
