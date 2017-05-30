program p;
var
    i:integer;

procedure proc(var a : integer);
begin
	a := 10;
end;

begin
	i := 1;
	proc(i);
	write(i);
end.
