program p;
var
    i:integer;
	arr: array [1..10] of integer;

function func(var i : integer) : integer;
begin
	i := i + 1;
	if i > 10 then
	begin
		result := i;
	end
	else
	begin
		result := func(i);
	end;
end;

begin
	i := 0;
	i := func(i);
	write(i);
end.
