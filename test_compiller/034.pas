program fib;
var
i:integer;
arr: array [1..10] of integer;

function max(var arr : array [1..10] of integer) : integer;
var i : integer;
begin
	i := 1;
	result := 0;
	while i <= 10 do
	begin
		if result < arr[i] then
		begin
			result := arr[i];
		end;
		i := i + 1;
    end;
end;

begin
	for i := 1 to 10 do
		arr[i] := i;
    //write('n = ');
    //readln(n);
	write(max(arr));
end.
