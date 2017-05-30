program p;
var
    i:integer;
	arr: array [1..10] of integer;

function f2(int :integer) : integer;
begin
	write(int);
	result := int;
end;

function f1(int :integer) : integer;
var i : integer;
begin
	i := int + 1;
	result := f2(i);
end;

begin
	write(f1(0));
end.
