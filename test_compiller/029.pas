program p;
var
    i:integer;

function f1(int :integer) : integer;
var i : ^integer;
	ii: integer;
begin
	ii := 10;
	i := @ii;
	write(ii, ' ', i^);
end;

begin
	f1(0);
end.
