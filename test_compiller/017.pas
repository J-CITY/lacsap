program main;
var
	rec : record
		a:integer;
		rec : record
			x:integer;
			y:integer;
		end;
	end;
begin
	rec.a := 5;
	write('rec.a = ', rec.a);

	rec.rec.x := 10;
	rec.rec.y := rec.rec.x + 3;
	write('rec xy = ', rec.rec.x, ' ', rec.rec.y);

end.
