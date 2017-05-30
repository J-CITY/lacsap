program p;
var
    i:integer;
	arr: array [1..10] of integer;

Function sum(arr: array[1..10] of integer): integer;
Var
  c, i: integer;
Begin
  for i := 1 to 10 do
  begin
	c := c + arr[i];
  end;
  result := c;
End;


begin

	for i := 1 to 10 do
		arr[i] := i;

	i := sum(arr);
	write(i);
end.
