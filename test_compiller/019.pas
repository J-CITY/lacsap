program main;
const  Max  = 10 * 2 - 16;
       NumChars = Ord('Z') - Ord('A') + 1;
var
	a, b:integer;
	_c, _v:char;
begin

write(Max);
write(NumChars);

_c := 'z';
_v := #67;
write(_c,_v);

a := ord(_v) + Max;
_v := chr(a);
write(a, _v);

end.
