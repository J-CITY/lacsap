program example_while;

var
  i, N: integer;

begin
  i := 0;
  while i <= 10 do
  begin
    write(i, ' ');
    i := i + 1;
  end;
end.
