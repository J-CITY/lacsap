program main;
var
	arr : array [0..10] of record
		a:integer;
		rec : record
			x:integer;
			y:integer;
		end;
		arr:array [1..5] of integer;
		b:real;
		c:char;
		d:boolean;
	end;
begin
	arr[5].a := 5;
	write('rec.a = ', arr[5].a, #10);

	arr[5].b := arr[5].a;
	write('rec.b = ', arr[5].b, #10);

	arr[5].b := 3.1415;
	arr[5].c := 'c';
	arr[5].d := true;
	write('rec = ', arr[5].a, ' ', arr[5].b, ' ', arr[5].c, ' ', arr[5].d, #10);

	arr[5].rec.x := 10;
	arr[5].rec.y := arr[5].rec.x + 3;
	arr[5].arr[3] := 11;
	write('rec xy = ', arr[5].rec.x, ' ', arr[5].rec.y, ' ', arr[5].arr[1], ' ', arr[5].arr[3], #10);

end.
