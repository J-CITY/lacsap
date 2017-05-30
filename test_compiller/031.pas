program p;
var
    i:integer;

function f1(var int :integer) : integer;
var i : ^integer;
	ii: integer;
begin
	i := @int;
	write(int, ' ', i^);
end;

begin
	i := 10;
	f1(i);
end.
