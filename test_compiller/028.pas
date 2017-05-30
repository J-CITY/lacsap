program p;
var
    i:integer;
	ip: ^integer;


procedure proc(a : ^integer);
begin
	write(a^);
end;

begin
	i := 10;
	ip := @i;
	proc(ip);

end.
