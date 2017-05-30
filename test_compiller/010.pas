program example_for;

var
  i, j: integer;

begin
  for i := 1 to 10 do
  begin
	for j := 1 to 10 do
	begin
		write(i, ' ', j, #10);
	end;
  end;
end.
