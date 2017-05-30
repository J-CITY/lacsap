program main;
Var
  a : record
    HouseNumber : Integer;
    PeopleName : char;
    r : record
      k,b,c:real;
      f,d:integer;
    end;
  End;

begin
	a.HouseNumber := 5;
  	a.r.k := a.HouseNumber;
	write(a.r.k);
end.
