program fib;

function fib(n :integer) : integer;
var a, b, i : integer;
begin
	a := 0;
    b := 1;
	
	for i:=3 to n do
	begin
        result := b;
        b := a + b;
        a := result;
    end;
end;

begin
    //write('n = ');
    //readln(n);
	write(fib(10));
end.
