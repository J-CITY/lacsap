program example_repeat;

var
  i, N: integer;

begin
  i := 0;
  repeat
  begin
    write(i, ' ');
    i := i + 1;
  end;
  until i = 10;
end.
