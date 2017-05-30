program p;
var
    i:integer;

Function sum(a, b: integer): integer;
Var
  c: integer;
Begin
  c := 0;
  c := a + b;
  result := c;
End;

begin
	i := sum(5, 6);
	write(i);
end.
